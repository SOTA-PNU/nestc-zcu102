# VTA 백엔드 Concat 지원 패치 (초안, 미검증)

작성 2026-10-08 · 대상 `gitlab.com/ones-ai/nest-compiler` 2024-07-09

---

## 문제

Concat 을 쓰는 모델(SqueezeNet, Inception, DenseNet)이 번들 생성에서 멈춘다.
Glow 는 Concat 을 세 단계로 로우어링한다. 출력 버퍼를 잡고(`AllocActivation`),
초기화되지 않았음을 표시하고(`TouchInst`), 입력들을 각자의 오프셋에 써 넣는다
(`InsertTensorInst`). VTA 백엔드의 **번들 경로**는 뒤의 두 명령을 모른다.

중요한 점은 **인터프리터 경로에는 이미 구현이 있다**는 것이다.

| 위치 | InsertTensor | Touch |
|---|---|---|
| `lib/Backends/VTA/VTA.cpp:124,247,314` | 등록됨 | — |
| `lib/Backends/VTA/VTAFunction.cpp:273` | 등록됨 | — |
| `lib/Backends/VTA/VTANodes.cpp:1406` | `fwdInsertTensorInst` 구현 | — |
| `lib/Backends/VTA/VTASave.cpp` | **없음** | **없음** |

그래서 보정 프로파일은 만들어지는데(VTAInterpreter 사용) 번들 생성에서만
깨진다. 게다가 `VTASave.cpp` 의 `default` 분기가 `llvm_unreachable` 이라
Release 빌드에서는 정의되지 않은 동작이 되어 원인이 가려졌다.

`TouchInst` 는 Glow 전체에서 아무 일도 하지 않는다. 인터프리터
(`lib/Interpreter/InterpreterNodes.cpp:2480`), VTAInterpreter
(`lib/Backends/VTAInterpreter/VTAInterpreterNodes.cpp:3233`), LLVM 코드젠
(`lib/LLVMIRCodeGen/LLVMIRGen.cpp:1212`) 모두 그냥 넘긴다.

---

## 패치 1 — vtalib 헤더에 선언 추가

`vta/vtalib/include/Bundle/VTABundle.h`, `transpose` 선언(72행) 근처에 추가한다.

```c
// Concat 로우어링이 만드는 InsertTensorInst 를 위한 런타임.
// 차원 수가 가변이라 transpose 처럼 개별 인자로 풀지 않고 배열로 받는다.
// offsetDim 은 Glow 시그니처와 맞추기 위한 것으로 사용하지 않는다.
int insert_tensor(int8_t *tensor, int8_t *slice, const dim_t *offset,
                  const dim_t *tensorDim, const dim_t *sliceDim,
                  dim_t numDimsTensor, dim_t numDimsSlice,
                  dim_t offsetDim, dim_t count, dim_t axis);
```

---

## 패치 2 — vtalib 구현

`vta/vtalib/lib/Bundle/CPUBundle.cpp` 와 `CPUBundle_aarch64.cpp` **양쪽에**
같은 내용을 넣는다. 둘은 `NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE` 로 갈리는
독립 구현이라, 한쪽만 고치면 다른 경로에서 링크가 깨진다.

Glow 원본(`lib/LLVMIRCodeGen/libjit/libjit.cpp:155`)은 차원 수마다 루프를
펼쳐 6차원부터 1차원까지 분기한다. 같은 일을 자리올림 카운터로 쓰면 짧아지고
차원 수에 무관해진다. 의미론은 동일하다.

```c
// 다차원 좌표를 일차원 인덱스로. 마지막 차원이 가장 빠르게 변한다.
static inline dim_t nestc_flat_index(const dim_t *dims, dim_t n,
                                     const dim_t *coord) {
  dim_t idx = 0;
  for (dim_t i = 0; i < n; i++) {
    idx = idx * dims[i] + coord[i];
  }
  return idx;
}

int insert_tensor(int8_t *tensor, int8_t *slice, const dim_t *offset,
                  const dim_t *tensorDim, const dim_t *sliceDim,
                  dim_t numDimsTensor, dim_t numDimsSlice,
                  dim_t offsetDim, dim_t count, dim_t axis) {
  (void)offsetDim;

  // Glow 는 좌표 버퍼를 6으로 잡아 두었다. 그 이상은 지원하지 않는다.
  if (numDimsSlice > 6 || numDimsTensor != numDimsSlice) {
    return -1;
  }
  if (axis >= numDimsSlice) {
    return -1;
  }

  dim_t total = 1;
  for (dim_t i = 0; i < numDimsSlice; i++) {
    total *= sliceDim[i];
  }
  if (total == 0 || count == 0) {
    return 0;
  }

  dim_t S[6];  // slice 좌표
  dim_t C[6];  // tensor 좌표

  for (dim_t c = 0; c < count; c++) {
    // count 는 같은 slice 를 axis 방향으로 여러 번 이어 붙일 때 쓰인다.
    const dim_t countAxisOffset = c * sliceDim[axis];

    for (dim_t i = 0; i < numDimsSlice; i++) {
      S[i] = 0;
    }

    for (dim_t e = 0; e < total; e++) {
      for (dim_t i = 0; i < numDimsSlice; i++) {
        C[i] = S[i] + offset[i] + ((i == axis) ? countAxisOffset : 0);
      }
      tensor[nestc_flat_index(tensorDim, numDimsTensor, C)] =
          slice[nestc_flat_index(sliceDim, numDimsSlice, S)];

      // 자리올림. 마지막 차원부터 증가시킨다.
      for (int i = (int)numDimsSlice - 1; i >= 0; i--) {
        if (++S[i] < sliceDim[i]) {
          break;
        }
        S[i] = 0;
      }
    }
  }
  return 0;
}
```

int8 만 다룬다. VTA 번들은 양자화된 모델만 내보내므로 지금은 이것으로 충분하다.
float 경로가 필요해지면 같은 본체를 템플릿이나 매크로로 복제한다.

---

## 패치 3 — VTASave.cpp 에 명령 처리 추가

### 3-1. 생성 함수

`saveReluInst`(4546행) 근처, 같은 파일의 다른 `save*Inst` 함수들 옆에 둔다.
심볼 처리 방식은 `saveReluInst` 를 그대로 따랐다 — Mutable 이면 심볼 테이블에
등록하고 그 이름을, 아니면 값의 이름을 쓴다.

```cpp
void saveInsertTensorInst(const glow::InsertTensorInst *Inst,
                          std::string *bundle, VTASaveContext *ctx) {
  auto src = Inst->getSrc();
  auto dest = Inst->getDest();

  auto srcWeight = static_cast<WeightVar *>(src);
  if (srcWeight->getMutability() == glow::WeightVar::MutabilityKind::Mutable) {
    addSymbolEntryGenBundle(srcWeight, bundle, ctx);
  }
  auto destWeight = static_cast<WeightVar *>(dest);
  if (destWeight->getMutability() == glow::WeightVar::MutabilityKind::Mutable) {
    addSymbolEntryGenBundle(destWeight, bundle, ctx);
  }

  auto emitName = [&](glow::Value *v, WeightVar *w) {
    if (w->getMutability() == glow::WeightVar::MutabilityKind::Mutable) {
      auto ste = addSymbolEntry(w, ctx);
      bundle->append(ste.name);
    } else {
      bundle->append(v->getName());
    }
  };

  auto destDims = dest->dims();
  auto srcDims = src->dims();
  auto offsets = Inst->getOffsets();

  // 차원과 오프셋은 컴파일 시점 상수다. 리터럴 배열로 박는다.
  // 같은 번들에 여러 개가 생기므로 이름이 겹치지 않게 블록으로 감싼다.
  static int insertTensorSeq = 0;
  std::string tag = "_it" + std::to_string(insertTensorSeq++);

  auto emitArray = [&](const char *suffix, llvm::ArrayRef<glow::dim_t> vals) {
    bundle->append("  const dim_t ");
    bundle->append(tag);
    bundle->append(suffix);
    bundle->append("[] = {");
    for (size_t i = 0; i < vals.size(); i++) {
      if (i) {
        bundle->append(", ");
      }
      bundle->append(std::to_string(vals[i]));
    }
    bundle->append("};\n");
  };

  emitArray("_dd", destDims);
  emitArray("_sd", srcDims);
  emitArray("_of", offsets);

  bundle->append("  insert_tensor(");
  emitName(dest, destWeight);
  bundle->append(", ");
  emitName(src, srcWeight);
  bundle->append(", ");
  bundle->append(tag + "_of, ");
  bundle->append(tag + "_dd, ");
  bundle->append(tag + "_sd, ");
  bundle->append(std::to_string(destDims.size()) + ", ");
  bundle->append(std::to_string(srcDims.size()) + ", ");
  bundle->append(std::to_string(offsets.size()) + ", ");
  bundle->append(std::to_string(Inst->getCount()) + ", ");
  bundle->append(std::to_string(Inst->getAxis()));
  bundle->append(");\n");
}
```

### 3-2. 디스패치에 등록

`VTASave.cpp` 의 명령 switch(5049행 근처, `TensorViewInstKind` 가 있는 곳)에
두 분기를 추가한다.

```cpp
    case Kinded::Kind::InsertTensorInstKind: {
      auto I2 = llvm::cast<InsertTensorInst>(&I);
      saveInsertTensorInst(I2, &bundle, &ctx);
      break;
    }
    // TouchInst 는 버퍼가 초기화되지 않았음을 알리는 표시일 뿐이다.
    // Glow 의 다른 백엔드도 전부 그냥 넘긴다.
    case Kinded::Kind::TouchInstKind:
      break;
```

---

## 패치 4 (권장) — default 분기를 안전하게

`VTASave.cpp:5093` 의 `default` 가 `llvm_unreachable` 이다. Release 빌드에서
`NDEBUG` 가 켜지면 이 매크로는 "여기 도달하지 않는다"는 최적화 힌트로만 남아
정의되지 않은 동작이 된다. 미지원 명령을 만나도 조용히 이상하게 동작한다.
SqueezeNet 이 왜 깨지는지 알아내는 데 이 때문에 시간을 썼다.

```cpp
    default: {
      llvm::errs() << "VTA backend: unhandled instruction "
                   << I.getKindName() << "\n";
      std::exit(1);
    }
```

상류에 별도로 보고할 사항이다.

---

## 검증 절차

1. PC 에서 컴파일러를 다시 빌드한다(컨테이너 안).
2. SqueezeNet 으로 번들 생성이 통과하는지 본다.
   ```
   bash gen-bundle.sh --conf models/squeezenet.conf --tools ... --upstream ... --out ...
   ```
3. 통과하면 보드에서 실행해 결과를 본다. 기준값은 PC 에서 VTAInterpreter 로
   먼저 떠 둔다 — 인터프리터 경로에는 이미 구현이 있으므로 그쪽은 지금도 돈다.
4. ResNet-18 을 다시 돌려 회귀가 없는지 확인한다. Concat 을 쓰지 않으므로
   결과가 변하면 안 된다.

**아직 또 다른 미지원 명령이 나올 가능성이 있다.** 패치 4를 먼저 넣어 두면
그때 무엇이 빠졌는지 바로 알 수 있다.

---

## 미검증 사항

이 패치는 작성만 했고 빌드도 실행도 하지 않았다. 다음을 특히 확인해야 한다.

`addSymbolEntry` 와 `addSymbolEntryGenBundle` 의 시그니처를 `saveReluInst`
에서 유추했다. 실제와 다르면 컴파일이 깨진다.

`Inst->getOffsets()` 가 반환하는 타입이 `llvm::ArrayRef<dim_t>` 인지 확인해야
한다. `std::vector` 라면 `emitArray` 의 인자 타입을 맞춰야 한다.

생성되는 C 코드에서 리터럴 배열이 함수 중간에 선언된다. 번들이 C99 이상으로
컴파일되면 문제없으나, C89 라면 블록 맨 앞으로 옮겨야 한다.

`count` 와 `axis` 의 의미를 Glow 구현에서 그대로 가져왔다. Concat 로우어링이
실제로 `count > 1` 을 쓰는지는 확인하지 않았다.
