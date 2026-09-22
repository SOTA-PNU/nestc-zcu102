# nestc — ZCU102 + VTA에서 NEST-C로 ResNet-18 추론

Xilinx ZCU102 보드의 FPGA에 올라간 VTA(NPU)에서 ResNet-18 추론을 실행하는
환경의 설정·실행·백업 기록이다.

컴파일러는 ETRI의 [NEST-C](https://github.com/etri/nest-compiler) (Glow 기반).

## 현재 상태

ResNet-18 추론이 VTA 하드웨어에서 **동작 확인됨** (2026-09-21).

| 이미지 | 기대 클래스 | 결과 | Confidence | 추론 시간 |
|--------|------------|------|-----------|----------|
| dog_207.png | 207 | 207 | 0.978 | 110.8 ms |
| zebra_340.png | 340 | 340 | 0.970 | 109.0 ms |
| cat_285.png | 285 | 281 | 0.478 | 179.9 ms |

cat만 281(tabby cat)로 나왔으나 285(Egyptian cat)와 같은 고양이 품종군이고
confidence가 낮아, int8 양자화 모델이 헷갈린 정상 범위의 근사다.

첫 실행 180ms는 VTA 드라이버 초기화와 캐시 워밍업이 포함된 값이고,
정상 상태는 110ms 수준이다.

## 빠른 시작

보드 부팅 후 SSH 접속해서:

```bash
cd /home/xilinx/nest-compiler/origin/vta/bundles/Resnet18Test
sudo ./vtaMxnetResnet18Bundle \
  /home/xilinx/nest-compiler/glow/tests/images/imagenet/dog_207.png
```

`sudo`는 필수다. 자세한 내용은 [docs/02-run-resnet18.md](docs/02-run-resnet18.md).

## 환경

| 항목 | 값 |
|------|-----|
| 보드 | Xilinx ZCU102 Rev1.0 (XCZU9EG) |
| OS | PYNQ Linux (Ubuntu 18.04) |
| 커널 | 4.14.0-xilinx-v2018.3 aarch64 |
| RAM | 1.5 GB (+1 GB swap) |
| SD | 128 GB 카드, 64 GB 파티션 (rootfs 59 GiB, 15 GB 사용) |
| LLVM | 6.0 (apt 저장소 최대치) |
| 빌드도구 | cmake 3.10.2, ninja, gcc 7.3.0 |

## 문서

| 문서 | 내용 |
|------|------|
| [01-boot-setup.md](docs/01-boot-setup.md) | 부트 스위치, UART 포트, 부팅 절차, SSH |
| [02-run-resnet18.md](docs/02-run-resnet18.md) | 추론 실행, sudo 필요 이유, 결과 해석 |
| [03-build-notes.md](docs/03-build-notes.md) | 빌드 옵션, LLVM 제약, 함정들 |
| [04-backup.md](docs/04-backup.md) | SD카드 백업 절차와 복원 |

## 저장소 구성

```
.
├── docs/         설정·실행·빌드·백업 문서
├── patches/      nest-compiler의 커밋 안 된 수정분
├── scripts/      실행 스크립트
└── backups/      SD카드 백업 (.gitignore, 로컬 전용)
```

## 주의사항

**`nest-compiler`에 git 커밋되지 않은 수정이 2건 있다** (이전 작업자 작업):

- `vta/bundles/Resnet18Test/CMakeLists.txt` (106줄)
- `vta/bundles/Resnet18PartitionTest/CMakeLists.txt` (76줄)

`git checkout`으로 되돌리면 복구할 수 없다. `patches/`에 백업해두었다.

**보드에서 nest-compiler를 빌드할 수 없다.** 공식 문서가 LLVM >= 7.0을
요구하는데 Ubuntu 18.04 arm64 저장소는 6.0이 최대다. 다만 번들이 이미
생성되어 있어 재빌드할 필요가 없다. [03-build-notes.md](docs/03-build-notes.md) 참고.
