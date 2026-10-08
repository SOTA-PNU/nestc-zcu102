# ZCU102 + NEST-C ResNet-18 실행 매뉴얼

작성 2026-10-08 · donghun (PNU-SOTA) · 대상 ZCU102 + PYNQ(Ubuntu 18.04) + ETRI NEST-C

---

## 0. 이 문서의 범위

보드를 켜는 것부터 ResNet-18 추론 결과를 확인하는 것까지를 다룬다. 처음 받은 사람이 이 문서만 보고 끝까지 갈 수 있도록 썼다.

세 가지 경로가 있다. 목적에 따라 하나만 하면 된다.

**경로 A — 이미 만들어진 번들로 실행한다.** 보드에 올라가 있는 실행 파일을 그대로 돌린다. 5분이면 된다. 보드가 살아 있는지 확인하거나 결과를 재현할 때 쓴다. 3장.

**경로 B — 상류 소스에서 빌드해 실행한다.** GitLab 상류를 클론해 번들 실행 파일을 직접 만든다. 한 시간 가까이 걸린다. 환경을 새로 세우거나 상류 변경을 반영할 때 쓴다. 4장.

**경로 C — ONNX 모델에서 번들을 직접 생성한다.** PC에서 양자화 보정과 컴파일을 수행해 번들을 만들고 보드로 보낸다. 새 모델을 올릴 때의 본래 경로다. 5장.

연구를 이어받는 입장이라면 A로 동작을 확인하고, B로 환경을 복원한 뒤, C를 익히는 순서를 권한다.

### 0.1 번들이 세 종류라는 점에 주의한다

"ResNet-18 번들"이라고 말할 때 서로 다른 세 가지를 가리킬 수 있다. 혼동하면 어느 것을 고치고 있는지 알 수 없게 된다.

| 출처 | 어디에 있나 | 무엇인가 |
|---|---|---|
| ETRI 배포본 | `/home/xilinx/nest-compiler/origin/vta/bundles/Resnet18Test/` | 보드를 받았을 때부터 있던 실행 파일. 경로 A가 돌리는 것 |
| 자체 생성본 | PC에서 만들어 보드로 전송 | ONNX에서 직접 컴파일한 것. 파이프라인 검증용. 경로 C의 산출물 |
| 상류 테스트용 | `~/nest-compiler-2024/build_board/vta/bundles/*/` | `aws` shim이 GitLab 미러에서 받아 온 상류 번들 소스를 보드에서 빌드한 것. 경로 B가 만드는 것 |

셋은 같은 모델이지만 만들어진 경로가 다르다. 자체 생성본을 만든 목적은 ResNet-18을 돌리는 것이 아니라 **정답을 아는 모델로 파이프라인을 채점하는 것**이었다. 그 결과가 5.6절이다.

---

## 1. 전체 구조

NEST-C는 Glow 기반 AOT(ahead-of-time) 컴파일러다. 모델을 미리 컴파일해 **번들**이라는 C++ 소스와 가중치 파일을 만들고, 그것을 보드에서 컴파일해 단일 실행 파일로 돌린다. 보드에는 인터프리터도 런타임 그래프도 없다.

```
  PC                                        보드
  ───────────────────────────────           ──────────────────────
  ONNX 모델
     │
     │ ① 보정 프로파일 생성
     │   image-classifier -dump-profile
     ▼
  calib.yaml
     │
     │ ② 번들 생성
     │   model-compiler -backend=VTA
     ▼
  model.cpp / model.h / model.weights.bin  ──전송──▶  ③ g++ 로 컴파일
                                                        │
                                                        ▼
                                                     실행 파일
                                                        │
                                                        │ ④ 실행
                                                        ▼
                                                   VTA(FPGA) + ARM CPU
```

VTA는 16×16 int8 GEMM 시스톨릭 어레이다. 곱셈기는 정수뿐이고 ALU는 덧셈·최대·최소·시프트만 가진다. 부동소수점 곱셈기가 없으므로 양자화 스케일이 2의 거듭제곱이어야 하고, 그래서 `symmetric_with_power2_scale` 스키마를 쓴다. 모델 전체가 VTA에서 도는 것은 아니다. 컨볼루션 등 일부만 VTA로 가고 나머지는 ARM CPU가 처리한다.

---

## 2. 보드 준비

### 2.1 전원과 부트 모드

SD 카드 부팅이다. 부트 모드 스위치와 UART 콘솔 설정은 `01-boot-setup.md` 를 따른다. 정상 부팅하면 FPGA 비트스트림이 자동으로 올라간다.

**SD 카드 주의.** 원본 SD 카드는 교수님 소유이며 보존 중이다. 모든 작업은 md5 검증을 거친 복제본에서 한다. 원본을 보드에 꽂지 않는다.

### 2.2 접속

보드는 사설망(`192.168.1.40`) 뒤에 있고 외부에서는 포워딩으로 들어간다.

```sh
ssh xilinx@sota.pusan.ac.kr -p 25022
```

접속되면 프롬프트가 `xilinx@pynq:~$` 다. 이 문서에서 보드 명령이라고 하면 이 프롬프트에서 치는 것을 말한다.

**긴 작업은 tmux 안에서 한다.** SSH가 자주 끊기는데, tmux 세션은 살아남는다.

```sh
tmux new -s work          # 새로 만들기
tmux attach -t work       # 다시 붙기
tmux ls                   # 목록
```

세션에서 빠져나올 때는 `Ctrl+B` 를 누른 뒤 `D` 를 누른다.

### 2.3 상태 확인

```sh
uname -a                  # aarch64, Ubuntu 18.04
free -h                   # 가용 1.5GB 내외
ls -l /dev/xlnk           # crw------- root root
df -h /                   # 여유 공간
```

`/dev/xlnk` 가 없으면 VTA 드라이버가 올라오지 않은 것이다. 재부팅한다.

---

## 3. 경로 A — 이미 만들어진 번들 실행

가장 빠른 길이다. 보드에 번들 실행 파일이 이미 있다.

```sh
cd /home/xilinx/nest-compiler/origin/vta/bundles/Resnet18Test
sudo ./vtaMxnetResnet18Bundle \
  /home/xilinx/nest-compiler/glow/tests/images/imagenet/dog_207.png
```

출력은 이렇게 나온다.

```
Loaded weights of size: 12003168 from the file mxnet_exported_resnet18.weights.bin
Loaded image: .../imagenet/dog_207.png
Loaded images size in bytes is: 602112
Result: 207
Confidence: 0.977876
Inference time: 110.817ms
```

### 3.1 반드시 sudo 로 실행한다

VTA 런타임은 FPGA와 DMA로 데이터를 주고받기 위해 `/dev/xlnk` 를 연다. 이 디바이스는 root 전용이다. 일반 사용자로 실행하면 `Failed to open /dev/xlnk` 로 즉시 끝난다.

### 3.2 입력 이미지는 필수 인자다

인자 없이 실행하면 가중치는 읽지만 이미지가 0개라 세그폴트가 난다. `Loaded images size in bytes is: 602112` 가 정상값이다. 224 × 224 × 3 × 4바이트(float32 RGB)가 602112다. 이 숫자가 0이면 경로를 잘못 준 것이다.

### 3.3 테스트 이미지와 기대 결과

| 파일 | ImageNet 클래스 | 실제 출력 | 확신도 |
|---|---|---|---|
| `cat_285.png` | 285 (Egyptian cat) | 281 | 0.478113 |
| `dog_207.png` | 207 (golden retriever) | 207 | 0.977876 |
| `zebra_340.png` | 340 (zebra) | 340 | 0.969942 |

**고양이가 281로 나오는 것은 오동작이 아니다.** 281은 tabby cat, 285는 Egyptian cat으로 같은 품종군이고 확신도도 0.478로 낮다. 모델이 헷갈린 경우이며 int8 양자화 모델에서 흔하다. 개와 얼룩말은 정확히 맞고 확신도도 0.97 이상이다.

이 세 줄이 재현되면 보드와 VTA가 정상이라고 판단해도 된다.

### 3.4 성능

첫 실행 약 180ms, 이후 약 110ms다. 첫 회에는 VTA 드라이버 초기화와 캐시 워밍업이 포함되므로 벤치마크할 때는 버린다.

### 3.5 실행 인자를 모를 때

각 번들 디렉터리의 `CTestTestfile.cmake` 에 원래 등록된 명령이 남아 있다.

```sh
cat CTestTestfile.cmake
```

`LABELS "NESTC;ZCU102"` 가 붙은 테스트가 보드 실기용이다. 다른 번들도 같은 방법으로 실행법을 알 수 있다.

### 3.6 절대로 하지 말 것

**VTA 프로그램 실행 중 Ctrl+C 금지.** FPGA와 커널 메모리 할당기(xlnk)가 정리되지 않아 재부팅 전까지 보드를 쓸 수 없다. 추론은 길어야 2초이므로 기다린다. 컴파일 중단은 안전하다.

---

## 4. 경로 B — 상류 소스에서 빌드

환경을 새로 세우거나 상류 변경을 반영할 때의 절차다. 보드에서 한 시간 가까이 걸린다.

### 4.1 aws shim 설치

ETRI가 데이터 배포에 쓰던 S3 버킷 `nestc-data-pub` 이 삭제되어 `NoSuchBucket` 을 반환한다. 빌드 중 CMakeLists가 `aws s3 cp` 로 번들 소스와 Input/Golden 데이터를 받으려 하므로 그대로 두면 실패한다.

같은 내용 198개 파일이 GitLab `yongin.kwon/nestc-data` 에 남아 있고 경로가 1:1로 대응한다. `aws` 명령을 흉내 내는 shim을 두어 CMakeLists를 수정하지 않고 통과시킨다.

```sh
mkdir -p ~/bin
cat > ~/bin/aws <<'EOF'
#!/bin/sh
BASE=https://gitlab.com/yongin.kwon/nestc-data/-/raw/master
if [ "$1" = "s3" ] && [ "$2" = "cp" ]; then
  SRC=$3; DST=$4
  case "$SRC" in
    s3://nestc-data-pub/*) P=${SRC#s3://nestc-data-pub/} ;;
    s3://nestc-pub/*)      P=${SRC#s3://nestc-pub/} ;;
    *) echo "aws-shim: 지원하지 않는 경로 $SRC" >&2; exit 1 ;;
  esac
  mkdir -p "$(dirname "$DST")" 2>/dev/null
  curl -fL "$BASE/$P" -o "$DST" || { echo "aws-shim: 실패 $P" >&2; exit 1; }
  exit 0
fi
exit 0
EOF
chmod +x ~/bin/aws
echo 'export PATH=$HOME/bin:$PATH' >> ~/.bashrc
export PATH=$HOME/bin:$PATH
```

`PATH` 설정은 빌드하는 모든 셸에서 필요하다. `~/.bashrc` 에 넣어두면 잊지 않는다.

### 4.2 상류 클론

```sh
git clone --recursive https://gitlab.com/ones-ai/nest-compiler.git ~/nest-compiler-2024
```

1.1GB 정도이며 시간이 걸린다. `--recursive` 를 빼면 `vta_lib` 서브모듈이 없어 빌드가 되지 않는다.

### 4.3 transpose 패치 적용

상류 `vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp` 의 `transpose()` 는 `//TODO re-implement` 주석과 함께 `-1` 만 반환하는 미구현 스텁이다. 출력 버퍼에 아무것도 쓰지 않으므로 초기화되지 않은 메모리가 후속 연산으로 흘러간다.

ResNet-18은 VTA 타일 레이아웃 전용 변환만 쓰므로 **영향을 받지 않는다.** 하지만 ResNet-50은 avgpool과 FC 사이에서 이 함수를 호출하기 때문에, aarch64 경로로 돌리면 입력 영상과 무관하게 항상 같은 클래스를 낸다. 지금 적용해 두는 편이 낫다.

저장소의 `ci/patch_transpose.py` 를 보드로 복사해 실행한다. 멱등하므로 여러 번 돌려도 안전하다.

```sh
python3 patch_transpose.py ~/nest-compiler-2024/vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp
grep -c at8dim_nestc_patch ~/nest-compiler-2024/vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp
```

마지막 줄이 `3` 이면 적용된 것이다(함수 정의 1 + 호출 2).

### 4.4 설정과 빌드

```sh
cat > ~/run-build.sh <<'EOF'
set -e
export PATH=$HOME/bin:$PATH
cd ~/nest-compiler-2024
mkdir -p build_board && cd build_board
cmake .. \
  -DNESTC_WITH_EVTA=ON \
  -DLLVM_DIR=/usr/lib/llvm-8.0/lib/cmake/llvm \
  -DCMAKE_BUILD_TYPE=Release \
  -DNESTC_USE_VTASIM=OFF \
  -DVTA_RESNET18_WITH_SKIPQUANT0=ON \
  -DNESTC_EVTA_RUN_ON_ZCU102=ON \
  -DNESTC_USE_PRECOMPILED_BUNDLE=ON \
  -DNESTC_EVTA_RUN_WITH_GENERIC_BUNDLE=ON
make -j1 zcu102 || true
echo DONE
EOF
chmod +x ~/run-build.sh
tmux new -d -s bld '~/run-build.sh > ~/bld.log 2>&1'
```

이 플래그 조합은 상류의 `nestc_tests/vta_bundle_test.sh` 와 동일하다. 각각의 의미는 `03-build-notes.md` 에 있다.

진행 확인은 별도 창에서 한다.

```sh
tail -3 ~/bld.log
grep -cE 'error:' ~/bld.log      # 실제 컴파일 에러 수
grep -c 'aws-shim' ~/bld.log     # shim 호출 수 (15 내외)
```

`DONE` 이 보이고 `error:` 가 0이면 성공이다.

### 4.5 테스트 실행

```sh
tmux new -s chk
cd ~/nest-compiler-2024/build_board
sudo -E env PATH=$HOME/bin:$PATH ctest -L ZCU102 --output-on-failure 2>&1 | tee ~/check.log
```

기대 결과는 다음과 같다.

```
100% tests passed, 0 tests failed out of 51
Total Test time (real) =  12.40 sec
```

ResNet-18만 돌리려면 이렇게 한다.

```sh
sudo -E env PATH=$HOME/bin:$PATH ctest -L ZCU102 -R vtaMxnetResnet18Bundle --output-on-failure
```

### 4.6 빌드에서 흔히 빠지는 함정

**`make` 를 인자 없이 돌리지 않는다.** 아무것도 지정하지 않으면 TVM 전체와 Glow 컴파일러 본체를 만들기 시작한다. 보드는 `NESTC_USE_PRECOMPILED_BUNDLE=ON` 이라 `model-compiler` 가 필요 없고, 필요한 것은 번들 실행 파일뿐이다. 전체 빌드는 수 시간을 쓰고 LLVM 8.0의 RTTI 불일치에 부딪혀 실패한다. 반드시 `make zcu102` 로 타깃을 지정한다.

**`zcu102` 는 빌드 타깃이 아니라 실행 타깃이다.** 빌드가 끝나면 이어서 ctest를 실행한다. 비root로 돌면 전부 실패하고 make는 `Error 8`(ctest의 종료 코드)로 끝난다. 이것을 빌드 실패로 오인하기 쉽다. **make의 `Error N` 은 자식 프로세스의 종료 코드일 뿐 컴파일 에러가 아니다.** 실제 컴파일 에러는 `grep -cE 'error:'` 로만 판단한다. 위 스크립트에 `|| true` 를 넣은 이유가 이것이다.

**`| tail -N` 으로 빌드를 지켜보지 않는다.** 파이프 버퍼링 때문에 출력이 멈춘 것처럼 보여 멀쩡한 빌드를 중단하게 된다. `tee` 로 파일에 남기고 별도 창에서 `tail` 한다.

**동시에 두 개의 `make` 를 돌리지 않는다.** 가용 메모리 1.5GB에서 `cc1plus` 두 개가 뜨면 바로 한계에 닿는다. 중단했던 빌드의 유령 프로세스가 남아 있을 수 있으므로 `pgrep -a cc1plus` 로 확인하는 습관을 들인다.

**`NESTC_USE_PRECOMPILED_EVTA_LIBRARY=ON` 을 쓰지 않는다.** 공식 install.md에 있으나 S3가 죽어 404로 실패한다. 기본값 OFF면 서브모듈 소스에서 빌드하여 정상 동작한다. 공식 문서의 다른 부분도 같은 이유로 신뢰할 수 없으니 이 매뉴얼을 우선한다.

**`/home/xilinx/nest-compiler/exec1` 에서 빌드하지 않는다.** cmake 캐시에 `-fno-rtti` 가 오염되어 있다.

**병렬 빌드를 시도하지 않는다.** `-j2` 이상은 메모리 부족으로 떨어진다. `-j1` 이 유일한 선택지다.

---

## 5. 경로 C — ONNX에서 번들 직접 생성

새 모델을 올릴 때의 본래 경로다. 컴파일은 PC에서, 실행은 보드에서 한다.

### 5.1 왜 PC에서 하는가

보드는 DDR 4GB 중 대부분을 VTA용 연속 메모리(CMA)로 예약하여 리눅스 가용 메모리가 1.5GB뿐이다. 컴파일러 전체 빌드는 4시간이 걸리고 그마저 실패했다. 반면 `NESTC_USE_PRECOMPILED_BUNDLE=ON` 이 말해 주듯 **보드가 실제로 필요로 하는 것은 컴파일러가 아니라 런타임뿐이다.** PC에서 컴파일하면 5분이면 끝난다.

### 5.2 PC 환경

도커 이미지가 두 개 있다. **번들 생성에 실제로 사용하고 검증한 것은 `leejaymin/nestc-ssh:latest` 이다.** 5.6절의 검증 결과가 이 환경에서 나왔다.

```sh
docker pull leejaymin/nestc-ssh:latest
```

Ubuntu 20.04 + clang 8.0.1 + LLVM 8 구성이다. 이 안에서 상류를 빌드해 `model-compiler` 와 `image-classifier` 를 만든다. 절차는 `03-build-notes.md` 를 따른다.

다른 하나는 ETRI 공식 SDK `onesai1/nest-compiler-sdk` 다. 상류 `.gitlab-ci.yml` 이 빌드 이미지로 `1.0.0` 태그를 지정하고 있어 우리 CI(`.github/workflows/build.yml`)도 그대로 쓴다. aarch64 크로스 컴파일러와 onnxruntime 1.12.1을 포함하며 정의는 `gitlab.com/ones-ai/nest-compiler-sdk` 에 있다. 다만 **이 이미지로 번들을 생성해 본 적은 아직 없다.** CI 빌드용으로만 검증되었다. 번들 생성에 쓰려면 5.6절의 기준값으로 먼저 대조해 보아야 한다.

(로컬에 받아 둔 태그는 `latest` 이고 CI가 지정하는 태그는 `1.0.0` 이다. 두 태그가 동일한지는 확인하지 않았다.)

### 5.3 1단계 — 보정 프로파일 생성

**보정 프로파일은 어디서 받는 것이 아니라 직접 만드는 것이다.** 이 사실을 몰라 이틀을 소모했으므로 특히 강조해 둔다.

양자화는 각 레이어의 활성값 분포를 알아야 스케일을 정할 수 있다. 그 분포를 실제 이미지 몇 장을 흘려보내 수집하는 과정이 보정(calibration)이고, 결과가 `calib.yaml` 이다. 레이어별 Min/Max/Histogram이 들어 있다.

```sh
image-classifier <이미지들> \
  -m=<model>.onnx \
  -model-input-name=<입력이름> \
  -backend=VTAInterpreter \
  -image-layout=NCHW \
  -image-mode=0to1 \
  -use-imagenet-normalization \
  -dump-profile=calib.yaml \
  -quantization-schema=symmetric_with_power2_scale
```

`-backend=VTAInterpreter` 는 FPGA 없이 소프트웨어로 VTA를 흉내 내는 백엔드다. 보정은 하드웨어가 필요 없다.

`symmetric_with_power2_scale` 은 영점(zero point)이 0이고 스케일이 2의 거듭제곱인 스키마다. VTA에 부동소수점 곱셈기가 없고 시프트만 있어서 강제되는 제약이다.

### 5.4 2단계 — 번들 생성

```sh
model-compiler -g \
  -model=<model>.onnx \
  -backend=VTA \
  -emit-bundle=<절대경로> \
  -bundle-api=dynamic \
  -model-input=<입력이름>,float,<shape> \
  -load-profile=calib.yaml \
  -quantization-schema=symmetric_with_power2_scale
```

`-emit-bundle` 에는 **절대 경로**를 준다. 상대 경로로 주면 내부에서 경로 조립이 꼬인다.

산출물은 네 개다. `<model>.cpp`(생성된 연산 순서), `<model>.h`, `<model>.weights.bin`(int8 가중치), `VTARuntime.h`.

양자화 스케일은 이 시점에 코드에 박힌다. 실행 시점에 바꿀 수 없으므로, 양자화를 바꾸려면 번들을 다시 만들어야 한다.

### 5.5 3단계 — 보드에서 컴파일

번들을 보드로 전송하고 상류 VTA 런타임과 링크한다. CMake를 거치지 않고 기존 빌드의 컴파일 인자를 추출해 직접 컴파일하는 방식을 쓴다. 기존 작업 트리를 건드리지 않기 위함이다. 구체적인 링크 명령은 `03-build-notes.md` 에 있다.

**주의 — 상류 번들은 상류 런타임을 요구한다.** 2024년 번들이 호출하는 `VTAUopBufferReset()` 이 보드에 설치된 2021년 런타임에는 없다. 상류 `vta_lib` 을 보드에서 별도로 빌드해야 하고, 링크할 때 **상류 헤더 경로를 기존 트리보다 앞에 두어야** 한다. 이걸 놓치면 `VTAUopBufferReset was not declared` 가 난다. 라이브러리 경로만 고쳐서는 해결되지 않는다.

### 5.6 검증

자체 생성한 ResNet-18 번들이 ETRI 배포 번들과 동일한 결과를 내는지 확인했다.

| 입력 | 결과 | 확신도 | 추론 시간 |
|---|---|---|---|
| cat_285.png | 281 | 0.478113 | 122.1ms |
| dog_207.png | 207 | 0.977876 | 109.0ms |
| zebra_340.png | 340 | 0.969942 | 112.6ms |

확신도가 소수점 여섯 자리까지 일치한다. 파이프라인이 올바르다는 근거다. 새 모델을 올릴 때도 같은 방식으로 기준값을 먼저 확보해 두면 디버깅이 쉬워진다.

---

## 6. 문제 해결

| 증상 | 원인과 조치 |
|---|---|
| `Failed to open /dev/xlnk` | sudo 없이 실행했다. sudo를 붙인다 |
| `Loaded images size in bytes is: 0` 뒤 세그폴트 | 이미지 인자를 주지 않았거나 경로가 틀렸다 |
| 입력을 바꿔도 결과가 같다 | aarch64 경로에서 `transpose()` 스텁을 밟고 있다. 4.3절의 패치를 적용한다 |
| `[ERROR] invalid layout` | aarch64 FC의 전제 조건(입력 16의 배수, 출력 4의 배수)을 못 맞췄다. 반환값을 검사하지 않는 상류 결함이라 결과가 조용히 틀린다 |
| 빌드가 `Error 8` 로 끝난다 | ctest가 비root로 실행된 것이다. 컴파일 에러가 아니다. `grep -cE 'error:'` 로 확인한다 |
| `VTAUopBufferReset was not declared` | 상류 헤더 경로가 기존 트리보다 뒤에 있다. 5.5절 참조 |
| `NoSuchBucket` / 404 | S3가 삭제되었다. 4.1절의 shim을 설치한다 |
| 빌드 중 OOM | `make` 가 두 개 돌고 있거나 `-j2` 이상을 썼다. `pgrep -a cc1plus` 로 확인한다 |
| 보드가 응답하지 않는다 | VTA 실행 중 Ctrl+C를 눌렀을 가능성이 높다. 재부팅 외에 방법이 없다 |
| SSH가 자꾸 끊긴다 | tmux 안에서 작업한다 |

---

## 7. 용어

**번들(bundle)** — AOT 컴파일 산출물. 생성된 C++ 소스, 헤더, int8 가중치 바이너리의 묶음. 보드에서 컴파일해 단일 실행 파일로 만든다.

**보정 프로파일(calibration profile)** — 레이어별 활성값의 Min/Max/Histogram을 담은 YAML. 양자화 스케일을 정하는 근거다. 실제 이미지를 흘려 수집한다.

**`symmetric_with_power2_scale`** — 영점 0, 스케일 2ⁿ인 양자화 스키마. VTA에 부동소수점 곱셈기가 없고 시프트만 있어 강제된다.

**generic / aarch64 번들** — VTA가 처리하지 못하는 연산을 넘겨받는 CPU 폴백 구현. `NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE` 로 전환한다. ON이면 이식성 우선의 참조 C++, OFF면 NEON + ARM Compute Library. 후자가 ResNet-50 기준 4.08배 빠르다.

**VTAInterpreter** — FPGA 없이 소프트웨어로 VTA 동작을 흉내 내는 백엔드. 보정과 디버깅에 쓴다.

**xlnk** — Xilinx 커널 메모리 할당기. VTA가 DMA로 쓸 연속 물리 메모리를 잡아 준다. `/dev/xlnk` 가 root 전용이라 sudo가 필요하다.

---

## 8. 관련 문서

부팅과 콘솔 설정은 `01-boot-setup.md`, 실행 세부와 번들 구성은 `02-run-resnet18.md`, 빌드 옵션과 LLVM 제약은 `03-build-notes.md`, SD 카드 백업은 `04-backup.md`, ResNet-50 디버깅 전 과정은 `05-model-debugging.md`, NEST-C 구조 학습은 `06-nestc-study-guide.md`, 전체 경과와 로드맵은 `07-progress-report.md` 를 참조한다.
