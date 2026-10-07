# CI 구성 (SOTA-PNU/nestc-etri)

GitLab `ones-ai/nest-compiler` 의 CI 를 GitHub Actions 로 옮긴 것.
원본 CI 는 ETRI 자체 러너(`etri-compute`, `etri-board` 태그)에서만 돌기 때문에
외부에서는 재현할 수 없어, 같은 파이프라인을 우리 인프라에 다시 세운다.

## 파일 배치

```
.github/workflows/build.yml        # 컴파일러 빌드 (GitHub 호스티드 러너)
.github/workflows/board-test.yml   # ZCU102 보드 테스트 (self-hosted 러너)
ci/patch_transpose.py              # 상류 vtalib 버그 패치
```

## 원본 CI 와의 대응

| GitLab | GitHub Actions | 비고 |
|---|---|---|
| `.builds` + `build` | `build.yml` | 이미지 `onesai1/nest-compiler-sdk:1.0.0` 그대로 |
| `test-board` | `board-test.yml` | 원본은 실행이 주석 처리되어 있었다 |
| `publish` (S3 업로드) | `upload-artifact` | S3 버킷이 삭제되어 대체 |

## 원본에서 바뀐 점과 이유

**S3 버킷이 사라졌다.** `nestc-data-pub` 이 `NoSuchBucket` 을 반환한다.
다행히 같은 내용이 GitLab `yongin.kwon/nestc-data` 에 그대로 있다(198개 파일 —
모델 ONNX, 보정 프로파일 YAML, 번들 소스, Input/Golden 바이너리, 비트스트림).
경로가 1:1 로 대응하므로 `aws` 명령을 흉내 내는 shim 을 보드의 `~/bin/aws` 에 두어
CMakeLists 를 수정하지 않고 통과시킨다.

```sh
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
```

`s3://nestc-pub/vta/bundles/ResConv*Test/` 는 GitLab 에 미러가 없어
ResConv 1~10 테스트는 이 경로로 복구되지 않는다.

**보드 테스트를 실제로 실행한다.** 원본 `test-board` 는 cmake 설정만 하고
`sudo make check_zcu102` 가 주석 처리되어 있다. 그래서 상류의 aarch64 경로는
CI 에서 한 번도 밟히지 않았고, `transpose()` 미구현 버그가 3년 넘게 남아 있었다.

**transpose 패치를 적용한다.** `ci/patch_transpose.py` 참조. 멱등하다.

## 러너 등록 — 보드가 아니라 PC 에

러너를 보드에 직접 올리는 구성은 두 가지 이유로 쓰지 않는다.

하나, 보드는 Ubuntu 18.04 / glibc 2.27 이다. GitHub Actions 의 자바스크립트 액션
(`actions/checkout@v4`, `actions/upload-artifact@v4` 등)은 Node 20 바이너리를
러너가 직접 실행하는데, Node 20 은 glibc 2.28 이상을 요구한다. 보드에서는
러너가 등록은 되어도 액션 단계에서 바로 죽는다.

둘, 보드의 리눅스 가용 메모리는 1.5GB 뿐이다(DDR 4GB 중 나머지는 VTA 용 CMA 예약).
실측 가용은 615MB 였다. 러너 프로세스가 수백 MB 를 차지하면 `-j1` 빌드조차
OOM 으로 떨어진다.

그래서 러너는 PC(WSL2, x64)에 두고, 보드에는 SSH 로 명령만 보낸다.
보드는 지금까지와 똑같이 빌드와 실행만 담당하고, 자바스크립트는 한 줄도 돌지 않는다.

```sh
# PC 의 WSL 안에서
mkdir -p ~/actions-runner && cd ~/actions-runner
# GitHub 저장소 Settings > Actions > Runners > New self-hosted runner
# Architecture 는 Linux x64 를 선택하고 안내대로 진행
./config.sh --url https://github.com/SOTA-PNU/nestc-etri --token <토큰> \
            --labels self-hosted,linux,x64
sudo ./svc.sh install && sudo ./svc.sh start   # 부팅 시 자동 시작
```

저장소 시크릿(Settings > Secrets and variables > Actions):

| 이름 | 값 |
|---|---|
| `BOARD_HOST` | 보드 주소 |
| `BOARD_PORT` | SSH 포트 |
| `BOARD_USER` | `xilinx` |
| `BOARD_SSH_KEY` | 보드 `~/.ssh/authorized_keys` 에 등록한 개인키 전문 |
| `BOARD_SUDO_PW` | (선택) sudo 비밀번호. NOPASSWD 라면 불필요 |

키는 반드시 터미널에서 만들고 시크릿 입력란에만 붙여넣는다.
`ssh-keygen -t ed25519 -f ~/.ssh/board_ci -N ""` 로 만든 뒤
`ssh-copy-id -i ~/.ssh/board_ci.pub -p <포트> xilinx@<주소>` 로 등록한다.

## 주의

**VTA 프로그램 실행 중 중단 금지.** FPGA 와 커널 메모리 할당기가 정리되지 않아
재부팅 전까지 보드를 쓸 수 없다. 워크플로를 취소할 때도 같은 문제가 생길 수 있으므로
`timeout-minutes` 를 넉넉히 둔다.

**GitHub 호스티드 러너 자원.** 4코어 / 16GB RAM / 디스크 14GB 다.
`ninja check_nestc` 전체는 디스크가 모자랄 수 있어 `build.yml` 은
`model-compiler` 와 `image-classifier` 만 만든다. 전체 테스트가 필요하면
PC 에 self-hosted 러너를 하나 더 두는 편이 낫다.
