# ZCU102 + NEST-C ResNet-18 실행 매뉴얼

작성 2026-10-08 · donghun (PNU-SOTA) · 대상 ZCU102 + PYNQ(Ubuntu 18.04) + ETRI NEST-C

---

## 0. 이 문서의 범위

보드를 켜는 것부터 ResNet-18 추론 결과를 확인하는 것까지를 다룬다. 처음 받은 사람이 이 문서만 보고 끝까지 갈 수 있도록 썼다.

세 가지 경로가 있다. 목적에 따라 하나만 하면 된다.

**경로 A — 이미 만들어진 번들로 실행한다.** 보드에 올라가 있는 실행 파일을 그대로 돌린다. 5분이면 된다. 보드가 살아 있는지 확인하거나 결과를 재현할 때 쓴다. 3장.

**경로 B — 상류 소스에서 빌드해 실행한다.** GitLab 상류를 클론해 번들 실행 파일을 직접 만든다. 한 시간 가까이 걸린다. 환경을 새로 세우거나 상류 변경을 반영할 때 쓴다. 4장.

**경로 C — ONNX 모델에서 번들을 직접 생성한다.** PC에서 양자화 보정과 컴파일을 수행해 번들을 만들고 보드로 보낸다. 새 모델을 올릴 때의 본래 경로다. 5장.

**경로 D — GitHub 푸시로 자동 추론.** 경로 C를 CI로 자동화한 것이다. 모델 기술서를 푸시하면 번들 생성부터 보드 추론, 판정까지 사람 손 없이 이루어진다. 6장.

연구를 이어받는 입장이라면 A로 동작을 확인하고, B로 환경을 복원하고, C로 원리를 익힌 뒤, D로 자동화에 올리는 순서를 권한다.

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

**도커 이미지는 빌드 환경만 제공한다. 컴파일러는 그 안에 들어 있지 않다.** 어느 이미지에도 `model-compiler` 와 `image-classifier` 가 없으며, 상류 소스를 직접 빌드해야 한다. 이 점을 오해하면 "이미지를 받았는데 명령이 없다"에서 막힌다.

구조는 이렇다. 호스트의 작업 디렉터리를 컨테이너에 마운트하고, 컨테이너 안에서 빌드한다. 결과물은 컨테이너가 아니라 **호스트에 남는다.** 컨테이너를 지워도 안전하고, 반대로 말하면 호스트 디렉터리를 지우면 몇 시간짜리 빌드를 다시 해야 한다.

현재 구성은 다음과 같다.

| 항목 | 값 |
|---|---|
| 호스트 작업 디렉터리 | `~/nestc-upstream` (WSL) |
| 컨테이너 안 경로 | `/root/nestc` |
| 빌드 결과물 | `~/nestc-upstream/build/glow/bin/model-compiler`<br>`~/nestc-upstream/build/glow/bin/image-classifier` |
| 컨테이너 이름 | `nestc` |

```sh
docker start nestc
docker exec -it nestc bash
# 컨테이너 안에서 /root/nestc 가 호스트의 ~/nestc-upstream 이다
```

새로 세운다면 이렇게 한다.

```sh
docker pull leejaymin/nestc-ssh:latest
docker run -dit --name nestc -v ~/nestc-upstream:/root/nestc leejaymin/nestc-ssh:latest bash
```

빌드 절차는 `03-build-notes.md` 를 따른다.

#### 이미지 선택

세 가지가 있고 전부 Ubuntu 20.04 + clang 8.0.1 기반이다.

| 이미지 | 크기 | LLVM | 비고 |
|---|---|---|---|
| `leejaymin/nestc-ssh:latest` | 6.26GB | `llvm-config` → 8.0.1 | **번들 생성에 사용·검증됨.** 5.6절이 이 환경 |
| `onesai1/nest-compiler-sdk:latest` | 6.26GB | `llvm-config` → 8.0.1 | llvm-18도 포함 |
| `onesai1/nest-compiler-sdk:1.0.0` | 4.68GB | `llvm-config` 없음(`llvm-config-8` 만) | 상류 `.gitlab-ci.yml` 이 지정하는 공식 CI 이미지 |

`onesai1` 의 두 태그는 **서로 다른 이미지다.** 다이제스트가 다르고 크기와 생성 시점도 다르다(`1.0.0` 이 2년 전, `latest` 가 20개월 전). 태그 이름만 보고 같은 것이라 가정하면 안 된다.

`1.0.0` 에는 `llvm-config` 가 PATH에 없고 `/usr/lib/llvm-8` 디렉터리만 있다. cmake가 LLVM을 자동으로 찾지 못할 수 있으므로 `-DLLVM_DIR=/usr/lib/llvm-8/lib/cmake/llvm` 을 명시하는 편이 안전하다.

번들 생성에는 검증된 `leejaymin/nestc-ssh:latest` 를 쓴다. 공식 SDK로 통일하려면 5.6절의 기준값과 먼저 대조해야 한다.

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

### 5.7 실전 예시 — 내 PC의 ONNX를 푸시해서 원격으로 추론하기

여기까지가 원리라면, 이 절은 실제 순서다. 새 모델 파일이 로컬 PC에 있고
그것을 보드에서 돌려 보려는 상황을 가정한다. 모델 이름을 `mymodel` 이라 하고,
파일은 윈도우의 `C:\Users\ehdgn\Downloads\mymodel.onnx` 에 있다고 하자.

#### 1단계 — 모델을 들여다본다

설정 파일에 적을 입력 이름과 형상을 **추측하지 않고 모델에서 읽는다.** 여기서
틀리면 보정 단계에서 `Mismatch between input image and ONNX input shape` 로
멈춘다.

```sh
cp /mnt/c/Users/ehdgn/Downloads/mymodel.onnx ~/nestc-upstream/
docker start nestc
docker exec -it nestc bash
```

컨테이너 안에서:

```sh
cd /root/nestc
python3 - <<'EOF'
import onnx
m = onnx.load('mymodel.onnx')
for i in m.graph.input:
    d = [x.dim_value or x.dim_param for x in i.type.tensor_type.shape.dim]
    print('입력', i.name, d)
for o in m.graph.output:
    print('출력', o.name)
print('연산자', sorted({n.op_type for n in m.graph.node}))
EOF
```

세 줄을 본다. 입력 이름은 `INPUT_NAME` 에, 형상은 `INPUT_SHAPE` 에 들어간다.
형상이 `[1,3,224,224]` 처럼 채널이 앞에 있어도 Glow에는 NHWC로 적는다
(`[1,224,224,3]`). 연산자 목록에 `Concat` 이 있으면 **지금은 올릴 수 없다.**
VTA 백엔드가 처리하지 못한다(5.3절).

#### 2단계 — PC에서 먼저 돌려 기준값을 만든다

새 모델은 정답을 모르므로 판정 기준이 없다. 보드에 올리기 전에 PC에서
VTAInterpreter로 돌려 그 결과를 골든으로 삼는다. 하드웨어 없이 도는 백엔드다.

```sh
/root/nestc/build/glow/bin/image-classifier \
  /root/nestc/glow/tests/images/imagenet/cat_285.png \
  /root/nestc/glow/tests/images/imagenet/dog_207.png \
  /root/nestc/glow/tests/images/imagenet/zebra_340.png \
  -m=mymodel.onnx -model-input-name=<1단계의 입력 이름> \
  -backend=VTAInterpreter \
  -image-layout=NHWC -image-mode=0to255 \
  -compute-softmax -topk=1
```

여기서 나오는 클래스 번호가 `EXPECT` 에 들어갈 값이다. **보드에서 나오기를
바라는 값이 아니라, 실제로 나온 값을 적는다.** 모델이 틀려도 그대로 적는다
(6.3절). 이 단계를 건너뛰면 CI가 무엇을 기준으로 통과를 판단할지 알 수 없다.

#### 3단계 — 저장소에 파일을 넣는다

두 가지면 된다. ONNX와 설정 파일이다.

```powershell
cd C:\Users\ehdgn\SOTA\nestc
mkdir models\onnx -Force
copy C:\Users\ehdgn\Downloads\mymodel.onnx models\onnx\
```

**Main.cpp는 만들지 않아도 된다.** `gen-bundle.sh` 가 ResNet-18의 main을 틀로
삼아 자동 생성한다. 분류 모델이 아니거나 출력 해석이 다르면 그때만 전용 main을
`MAIN_CPP` 로 지정한다.

ONNX가 100MB를 넘으면 GitHub가 푸시를 거부한다. 그때는 저장소에 넣지 말고
어딘가에 올린 뒤 설정에 URL을 적는다. `MODEL_ONNX=https://.../mymodel.onnx`
형태면 스크립트가 내려받는다.

#### 4단계 — 설정 파일을 쓴다

`models/mymodel.conf` 를 만든다. `models/resnet18.conf` 를 복사해 고치는 편이
빠르다.

```sh
MODEL_NAME=mymodel
MODEL_ONNX=repo:models/onnx/mymodel.onnx
INPUT_NAME=data                      # 1단계에서 읽은 값
INPUT_SHAPE="[1,224,224,3]"          # 1단계에서 읽은 값, NHWC로
IMAGE_LAYOUT=NHWC
IMAGE_MODE=0to255
USE_IMAGENET_NORMALIZATION=0
CALIB_EXTRA="-compute-softmax -topk=5"
CALIB_IMAGES=upstream:glow/tests/images/imagenet/cat_285.png
OUTPUT_NAME=<번들 생성 로그가 알려 준다, 아래 참고>
EXPECT="cat_285:281 dog_207:207 zebra_340:340"   # 2단계의 실제 출력
```

전처리 세 줄(`IMAGE_LAYOUT`, `IMAGE_MODE`, `USE_IMAGENET_NORMALIZATION`)은
2단계에서 쓴 것과 **반드시 같아야 한다.** 보정과 실행의 전처리가 다르면
양자화 스케일이 어긋나 엉뚱한 결과가 나온다.

`OUTPUT_NAME` 은 번들이 내보내는 출력 텐서 이름이다. 미리 알기 어려우므로
일단 비워 두고 한 번 돌린다. `gen-bundle.sh` 가 번들 생성을 마치면
`번들이 내보내는 심볼 (OUTPUT_NAME 후보)` 목록을 찍는다. 보통 ONNX 출력 이름에
`__1` 이 붙은 형태다(ResNet-18이면 `resnetv10_dense0_fwd__1`). 거기서 골라
설정에 적고 다시 푸시한다.

#### 5단계 — 푸시한다

```powershell
git add models
git commit -m "models: mymodel 추가"
git push sota main
```

`models/**` 가 바뀌었으므로 `model-test` 워크플로가 자동으로 기동한다.

#### 6단계 — 결과를 본다

https://github.com/SOTA-PNU/nestc-zcu102/actions 에서 방금 실행을 연다.
단계는 순서대로 모델 기술서 확인, SSH 준비, 컴파일러 확인, 번들 생성(컨테이너),
보드로 전송, 스크립트 전송, 보드에서 빌드하고 실행이다.

마지막 단계의 출력이 결론이다.

```
   OK   cat_285    281  (0.533373, 1476.76ms)
   OK   dog_207    207  (0.980803, 1477.54ms)
   OK   zebra_340  340  (0.963957, 1478.47ms)
   == 전부 통과
```

실패하면 Artifacts에 `model-mymodel` 이 올라와 있다. 보드의 빌드·실행 로그와
`calib.yaml` 이 들어 있어 원인을 찾을 수 있다.

#### 어디서 막히는가

| 단계 | 증상 | 원인 |
|---|---|---|
| 번들 생성 | `Mismatch between input image and ONNX input shape` | `INPUT_NAME` 또는 `INPUT_SHAPE` 가 모델과 다르다. 1단계를 다시 한다 |
| 번들 생성 | `is an unhandled instruction` 또는 조용히 중단 | VTA 백엔드가 그 연산을 모른다. Concat이 가장 흔하다 |
| 번들 생성 | `weights.bin 이 없다` | `model-compiler` 가 실패했다. 그 위 출력을 본다 |
| 보드 빌드 | `undefined reference to ...` | 자동 생성된 main의 접두사가 번들과 다르다. 번들 소스 파일 이름을 확인한다 |
| 보드 실행 | 입력을 바꿔도 결과가 같다 | `transpose()` 스텁을 밟고 있다. 패치를 확인한다 |
| 보드 실행 | `Result` 는 나오는데 기대값과 다르다 | 2단계의 전처리와 설정의 전처리가 어긋났을 가능성이 크다 |
| 보드 실행 | 결과가 늘 0이거나 의미 없는 값 | `OUTPUT_NAME` 이 틀렸다. 번들 생성 로그의 후보 목록을 본다 |

---

## 6. 경로 D — GitHub 푸시로 추론하기

경로 C를 자동화한 것이다. 모델 기술서 하나를 저장소에 푸시하면 CI가 번들을
만들어 보드로 보내고, 추론을 돌려 기대값과 대조한다. 사람이 손댈 일이 없다.

### 6.1 구성

```
  GitHub (SOTA-PNU/nestc-zcu102)
     │ push: models/**
     ▼
  PC 의 self-hosted 러너 (WSL x64)
     │ ① 컨테이너에서 번들 생성   scripts/gen-bundle.sh
     │ ② scp 로 보드에 전송
     ▼
  ZCU102 보드
       ③ 컴파일·링크·실행·판정    scripts/board-build-run.sh
```

러너를 보드가 아니라 PC에 두는 이유는 10.5절과 같다. 보드의 glibc 2.27로는
GitHub Actions의 자바스크립트 액션이 돌지 않고, 가용 메모리도 615MB뿐이다.

번들 생성은 반드시 **컨테이너 안에서** 한다. `model-compiler` 는 Ubuntu 20.04
컨테이너에서 빌드되어 `libprotobuf.so.17` 을 요구하는데, 러너가 사는 WSL은
22.04라 그 라이브러리가 없다. 워크플로가 `docker run` 으로 같은 이미지를
띄워 그 안에서 스크립트를 돌린다. 이때 상류 트리를 `/src` 에 마운트한다.
`/root` 아래에 마운트하면 `--user` 로 권한을 낮춘 컨테이너가 접근하지 못한다.

### 6.2 새 모델 올리기

`models/<이름>.conf` 를 만들어 푸시하는 것이 전부다. `models/resnet18.conf` 를
복사해 값을 바꾸면 된다.

```sh
MODEL_NAME=내모델
MODEL_ONNX=repo:models/onnx/내모델.onnx
INPUT_NAME=data
INPUT_SHAPE="[1,224,224,3]"
IMAGE_LAYOUT=NHWC
IMAGE_MODE=0to255
USE_IMAGENET_NORMALIZATION=0
CALIB_EXTRA="-compute-softmax -topk=5"
CALIB_IMAGES=upstream:glow/tests/images/imagenet/cat_285.png
OUTPUT_NAME=내모델출력__1
EXPECT="cat_285:281 dog_207:207 zebra_340:340"
```

경로 접두사는 `upstream:`(상류 트리), `repo:`(이 저장소), `http(s)://`(내려받기)
세 가지다.

채우기 전에 정해야 할 것이 셋 있다.

**입력 이름과 형상.** 추측하지 말고 모델에서 읽는다. `python3 -c "import onnx;
m=onnx.load('모델.onnx'); print(m.graph.input)"` 로 확인한다. Glow는 내부적으로
NHWC로 다루므로 채널이 마지막에 온다.

**출력 텐서 이름.** `OUTPUT_NAME` 에 적는다. Main.cpp는 자동 생성되지만
이 이름만은 접두사에서 파생되지 않아 따로 알려 주어야 한다. 모르면 비워 두고
한 번 돌리면 `gen-bundle.sh` 가 후보 목록을 찍는다.

**기대값.** 새 모델은 정답을 모르므로 판정 기준을 먼저 만들어야 한다. float
또는 VTAInterpreter로 PC에서 돌린 결과를 골든으로 삼는다. 이것이 없으면 CI가
무엇을 가지고 통과를 판단할지 알 수 없다.

### 6.3 기대값은 "정답"이 아니라 "어제와 같은 값"이다

`cat_285.png` 의 기대값이 285가 아니라 281인 것이 좋은 예다. 285는 Egyptian
cat, 281은 tabby cat이므로 모델은 틀렸다. 확신도도 0.53으로 낮아 모델이
자신 없어 하는 것이 드러난다.

그래도 281을 기대값으로 둔다. 이 테스트가 보는 것은 모델의 정확도가 아니라
**파이프라인이 어제와 같은 답을 내는가**이기 때문이다. ETRI 배포 번들도 똑같이
281을 낸다. 281로 고정해 두면 누군가 transpose 패치를 깨뜨리거나 양자화 설정을
바꿨을 때 값이 달라지면서 즉시 잡힌다. 285로 두면 영원히 빨간불이라 아무 신호도
주지 못한다.

모델이 실제로 얼마나 맞히는지는 별개의 작업이다. ImageNet 검증셋 수백 장으로
top-1 정확도를 재야 하며, 양자화 품질을 논할 때 하면 된다.

### 6.4 번들은 매번 다시 만들어야 하는가

아니다. 번들은 모델과 양자화 설정의 함수이므로, 둘 중 어느 것도 바뀌지
않았다면 다시 만들 필요가 없다. 다시 만들어야 하는 경우는 셋이다. 모델 파일이
바뀌었을 때, 설정의 전처리나 보정 이미지가 바뀌었을 때, 그리고 컴파일러나
상류 소스가 바뀌었을 때다. 같은 입력이면 같은 번들이 나온다.

푸시로 기동할 때는 안전하게 매번 생성한다. `models/**` 가 바뀌었다는 것은
대개 위 둘 중 하나가 바뀌었다는 뜻이기 때문이다. 손으로 돌릴 때는
`Run workflow` 에서 `rebuild_bundle` 을 꺼서 보드에 이미 있는 번들을 재사용할
수 있다. 보드 쪽 스크립트만 고쳤거나 실행을 한 번 더 해 보고 싶을 때 쓴다.

비용도 생각만큼 크지 않다. ResNet-18 기준으로 보정이 몇 초, 번들 생성이 수십 초
수준이다. 오래 걸리는 쪽은 보드에서의 컴파일이며, 그쪽은 번들이 바뀌지 않으면
`make` 가 건너뛴다.

### 6.5 보드가 한 대뿐이라는 제약

워크플로에 `concurrency: zcu102-board` 를 걸어 두 실행이 겹치지 않게 했다.
앞선 실행을 취소하지 않고 **기다리게** 한다. VTA 프로그램 실행 중 취소는
FPGA와 xlnk를 정리하지 못한 채 끝나므로, 재부팅 전까지 보드를 쓸 수 없다.

같은 이유로 저장소를 둘 이상에 CI를 걸면 안 된다. `concurrency` 는 저장소
단위라 서로를 막아 주지 못한다. 개인 저장소 `udonghun/nestc` 는 미러로 두고
Actions를 꺼 두는 편이 안전하다.

### 6.6 사전 준비

러너가 있는 PC에서 한 번만 해 두면 된다.

| 항목 | 내용 |
|---|---|
| self-hosted 러너 | 라벨 `self-hosted,linux,x64`, 서비스로 등록 |
| docker 권한 | 러너 계정이 docker 그룹에 있어야 한다 |
| 상류 클론 | `~/nestc-upstream`, 컨테이너에서 빌드한 컴파일러 포함 |
| 보드 SSH 키 | `~/.ssh/board_ci`, 공개키를 보드 `authorized_keys` 에 등록 |

저장소 시크릿 네 개도 필요하다. `BOARD_HOST`(`sota.pusan.ac.kr`),
`BOARD_PORT`(`25022`), `BOARD_USER`(`xilinx`), `BOARD_SSH_KEY`(개인키 전문).

**보드의 사설 주소를 쓰면 안 된다.** WSL2는 자체 NAT 안에 있어 `192.168.1.40`
에 닿지 못한다. 반드시 포워딩 주소를 쓴다. 이것 때문에 CI 첫 실행이
`Connection timed out` 으로 실패했다.

보드에서는 sudo 비밀번호를 생략해 두어야 한다. 워크플로가 `sudo ctest` 를
비대화식으로 돌리기 때문이다.

```sh
echo 'xilinx ALL=(ALL) NOPASSWD: ALL' | sudo tee /etc/sudoers.d/xilinx-ci
sudo chmod 440 /etc/sudoers.d/xilinx-ci
```

### 6.7 현재 검증된 범위와 남은 일

ResNet-18로 전 구간이 통과했다. 푸시 한 번으로 번들 생성, 전송, 보드 빌드,
추론, 판정까지 자동으로 이루어지며 세 장 모두 기대값과 일치했다.

남은 것이 셋 있다.

**추론이 느리다.** 1477ms가 나오는데 ETRI 배포 번들은 110ms다. 13배 차이는
generic과 aarch64 CPU 폴백의 차이와 일치한다(10.2절). 보드 빌드 디렉터리를
`build_aarch64` 로 바꾸면 해결될 것으로 보인다.

**확신도가 배포본과 다르다.** 0.533373 대 0.478113이다. 상류는
`VTA_RESNET18_WITH_SKIPQUANT0=ON` 일 때 `VTASkipQuantizeNodes.txt` 를 번들
디렉터리에 복사해 특정 노드의 양자화를 건너뛴다. `gen-bundle.sh` 에 그 단계가
없다. 클래스는 맞으므로 치명적이지 않으나, 배포본과 비트 단위로 맞추려면
넣어야 한다.

(2026-10-08 해결) **Main.cpp 일반화.** 모델마다 전용 main을 두던 것을
접두사 치환으로 자동 생성하게 바꾸었다. ResNet-18로 검증했다.

---

## 7. 문제 해결

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

## 8. 용어

**번들(bundle)** — AOT 컴파일 산출물. 생성된 C++ 소스, 헤더, int8 가중치 바이너리의 묶음. 보드에서 컴파일해 단일 실행 파일로 만든다.

**보정 프로파일(calibration profile)** — 레이어별 활성값의 Min/Max/Histogram을 담은 YAML. 양자화 스케일을 정하는 근거다. 실제 이미지를 흘려 수집한다.

**`symmetric_with_power2_scale`** — 영점 0, 스케일 2ⁿ인 양자화 스키마. VTA에 부동소수점 곱셈기가 없고 시프트만 있어 강제된다.

**generic / aarch64 번들** — VTA가 처리하지 못하는 연산을 넘겨받는 CPU 폴백 구현. `NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE` 로 전환한다. ON이면 이식성 우선의 참조 C++, OFF면 NEON + ARM Compute Library. 후자가 ResNet-50 기준 4.08배 빠르다.

**VTAInterpreter** — FPGA 없이 소프트웨어로 VTA 동작을 흉내 내는 백엔드. 보정과 디버깅에 쓴다.

**xlnk** — Xilinx 커널 메모리 할당기. VTA가 DMA로 쓸 연속 물리 메모리를 잡아 준다. `/dev/xlnk` 가 root 전용이라 sudo가 필요하다.

---

## 9. 관련 문서

부팅과 콘솔 설정은 `01-boot-setup.md`, 실행 세부와 번들 구성은 `02-run-resnet18.md`, 빌드 옵션과 LLVM 제약은 `03-build-notes.md`, SD 카드 백업은 `04-backup.md`, ResNet-50 디버깅 전 과정은 `05-model-debugging.md`, NEST-C 구조 학습은 `06-nestc-study-guide.md`, 전체 경과와 로드맵은 `07-progress-report.md` 를 참조한다.
