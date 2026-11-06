# 이 저장소에서 일하는 법

ETRI 의 NEST-C 컴파일러로 ZCU102 보드의 VTA/EVTA NPU 에 모델을 올리고,
그 과정을 GitHub Actions 로 자동화한 것이다. 답은 한국어로 한다.

**2026-11-06 부터 개인 공부·연구용 저장소다.** 상류는 ETRI 원본이 아니라
SOTA-PNU/nest-compiler 의 `ci/upstream.ref` 커밋이다. 연구실 공식 보드 CI 는
nest-compiler 의 `build.yml` → `test-board` 다. 같은 서버의 두 러너가 보드를
`scripts/board-lock.sh` 로 번갈아 쓴다 — 보드를 쓰는 새 workflow 에는 반드시
잠금 단계를 넣는다. 자세한 것은 `docs/15-nest-compiler-ci.md`.

**컴파일러를 고치는 순서 (2026-11-06~):** 포크 `udonghun/nest-compiler` 에 브랜치로
올린다 → 이 저장소의 model-test / multievta 를 입력 `ref=<브랜치>` 로 수동 실행해
보드에서 시험한다 → **성과가 있는 것만** SOTA-PNU/nest-compiler 에 PR 을 연다.
통과만으로는 올리지 않는다. 정확도(Top-1 확신도)가 그대로이면서 장당 시간·처리량·CMA
사용량·정확도 중 하나가 `ci/upstream.ref` 기준값보다 나아져야 한다. PR 본문에 전후 수치를 적는다. 시험 중인 것을
nest-compiler 에 바로 올리지 않는다(그쪽 Actions 와 compute-nestc 를 쓰지 않게).

**먼저 `NEXT.md` 를 읽는다.** 남은 일이 거기 있다.

---

## 1. 기계가 넷이다 — 창을 반드시 구분한다

| 프롬프트 | 기계 | 역할 |
|---|---|---|
| `xilinx@pynq:~$` | ZCU102 보드 (aarch64, Ubuntu 18.04) | 추론을 실행한다 |
| `ehdgns@sota-AIP:~$` | **연구실 서버** 192.168.1.150 (Ubuntu 24.04, 24코어) | **CI 러너 `sota-aip`** (2026-11-04~) |
| `ehdgns@ehdgns:~$` | 개인 PC 의 WSL (Ubuntu 22.04) | 예전 러너(해제함). 서버 접속 키가 여기 있다 |
| `PS C:\...>` | 개인 PC 의 윈도우 | git 푸시, 파일 관리 |
| `sota@vcu118-dev:~$` | 연구실 VirtualBox 게스트 | **CI 서버 후보였으나 중단** |

명령을 안내할 때는 **어느 창인지 반드시 쓴다.** 사용자가 자주 헷갈린다.
실제로 보드에서 `/dev/ttyUSB*` 를 찾거나 서버용 명령을 WSL 에서 돌린 적이
여러 번 있다.

여러 줄을 한 번에 붙여넣게 하지 않는다. 비밀번호를 묻는 명령이 섞이면
다음 줄이 비밀번호로 먹힌다.

보드 접속은 포워딩 주소를 쓴다. 사설 주소 `192.168.1.40` 은 공유기 안에서만
유효하다.

```sh
ssh -p 25022 xilinx@sota.pusan.ac.kr
```

## 2. 하면 안 되는 것

**VTA 추론 중에 Ctrl+C 를 누르거나 워크플로를 취소하지 않는다.** FPGA 와
xlnk 가 정리되지 않아 재부팅 전까지 보드를 못 쓴다. 빌드 단계는 VTA 가
안 열려 있으므로 취소해도 안전하다 — 구분해서 판단한다.

**보드의 CMA 부팅 인자를 원격으로 바꾸지 않는다.** 잘못 넣으면 부팅이 안
되고 누가 가서 SD 카드를 뽑아야 한다. UART 콘솔이 상시로 붙은 뒤에만 한다
(`docs/10` 4-b 절).

**토큰·개인키를 채팅에 쓰지 않는다.** 과거에 PAT 와 SSH 개인키가 대화에
노출되어 폐기한 적이 있다. 대화에 나타난 자격 증명은 전부 폐기 대상으로
다룬다. 터미널 프롬프트에만 입력하게 안내한다.

**`/home/xilinx/nest-compiler/exec1` 로 빌드하지 않는다.** cmake 캐시가
오염되어 있다(`-fno-rtti`).

교수님의 원본 SD 카드는 보존한다. 작업은 md5 검증된 복제본으로만 한다.

## 3. 지금까지의 결론

**ResNet-18 은 보드에서 정상이다.** `cat 281 / dog 207 / zebra 340`,
장당 **91.6ms**, 확신도 **0.478113**. 이 확신도가 ETRI 배포본의 공개
기준값과 소수점까지 일치한다.

**CPU 폴백은 aarch64 가 기준이다.** generic(순수 C) 1475ms 대
aarch64(NEON+ACL) 91.6ms 로 16배다. CI 의 기본 보드 빌드는
`build_aarch64` 다. generic 수치를 인용하지 않는다.

**(낡은 결론) "멀티 EVTA 는 코어를 늘려도 처리량이 안 는다."** 2코어로
100장을 9074ms(초당 11.02장)라 1.01배로 봤다. 단일 비트스트림 위에서 잰
것이라 실제로는 코어 0 혼자 처리한 수치였다 — 아래 정정을 본다.

설정 문제가 아님을 확인했다. 보드 빌드는 `-DVTA_DEVICE_NUM=4` 와 네 코어의
레지스터 주소를 제대로 넘기고, 생성된 번들도 코어 0 과 1 에 각각
`idEVTA=0, 1` 을 넣으며 명령 핸들도 `vtaCmdH` / `vtaCmdH1` 로 따로 갖는다.
런타임에 뮤텍스도 없다. 한때 "네 코어가 DDR 경로를 공유한다" 로 설명했으나
**틀렸을 가능성이 크다** — 아래 SError 항목을 본다.

**4코어가 못 돌던 이유는 찾았다.** 코어당 실측 77,616 kB 인데 번들 요구량은
12.6MB 뿐이었다. 나머지 64MB 는 VTA 컨텍스트마다 잡히는 명령 큐 둘이다.

```c
vta/vtalib/include/zcu102/vta/driver.h:45   #define VTA_MAX_XFER (1<<25)   // 32MB
vta/vtalib/include/zcu102/cmd_queue.h:579   UopQueue <VTA_MAX_XFER, ...>
vta/vtalib/include/zcu102/cmd_queue.h:581   InsnQueue<VTA_MAX_XFER, ...>
```

`ci/patch_vta_max_xfer.py` 가 `1<<24` 로 낮춘다. 코어당 44.6MB 가 되어 네
코어가 178MB 로 CMA 256MB 에 들어간다. **부팅 인자를 건드릴 필요가 없다.**
메모리는 2026-11-03 실측으로 확인했다(코어당 44,816 kB).

**부팅 기본 비트스트림은 단일 VTA 다.** 그 위에서 코어 1~3 스레드가
`SError`(커널 oops)로 죽고, 공유 카운터 덕에 코어 0 하나가 100장을 다 처리해
"통과" 로 찍혔다. 지금까지의 "2코어"·"4코어" 수치가 단일 코어와 같았던 이유다.

**진짜 4코어는 ETRI 4코어 비트스트림 + 멀티 구성 런타임으로 돈다.**
`scripts/board-bitstream.sh multi` 로 `zcu102_4_1x16_i8w8a32_16_15_18_17.bit`
를 올리고 `build_aarch64_multi`(NESTC_EVTA_MULTI=ON, 버퍼 15/18/17)와
링크한다. 2026-11-03 실측 4코어 23.58 img/s(2.16배), 2코어 18.23(1.67배),
oops 0, 오답 0. 끝나면 `single` 로 되돌린다 — 단일 코어 기준값은 단일
비트스트림의 것이다.

**멀티 EVTA 결과를 볼 때는 반드시 보드 `dmesg` 에 oops 가 없는지 본다.**
`board-multi-run.sh` 가 이제 그것을 판정한다. 자세한 것은 `NEXT.md` 0절.

**SqueezeNet 오답(905/904/533)은 종결했다.** 원인을 못 찾았다. `models/`
에서 빼 `models-off/` 로 옮겼다. 배제한 것과 근거는
`docs/13-status-2026-11-02.md` 4절에 있다. 다시 손대려면 그것부터 읽는다.

## 4. CMA — 이 보드의 가장 흔한 고장 원인

FPGA 는 연속 물리 메모리(CMA, 총 256MB)를 쓴다. **비정상 종료하면 반납되지
않는다.** 정상 종료하는 모델은 거의 안 샌다(ResNet-18 0.6MB, SqueezeNet 0).
중간에 죽는 모델만 정확히 64MB 를 들고 간다 — 명령 큐 둘이 그대로 남는
것이다.

모자란 상태로 실행하면 원인이 안 보이는 모습으로 죽는다.

```
dat_queue.h:39: Check failed: fpga_buff_ != nullptr
```

**이 메시지를 정확도 문제로 오해하지 않는다.** 2026-11-01 에 실제로
그럴 뻔했다. `board-build-run.sh` 가 실행 전에 관문으로 막고, 실행 뒤에도
`CmaFree` 를 찍어 모델마다 얼마씩 새는지 남긴다.

고치는 방법은 재부팅뿐이다. 프로세스를 죽여도 안 돌아온다.

```sh
ssh -p 25022 xilinx@sota.pusan.ac.kr 'sudo reboot'
```

**전원 차단이나 커널 oops 뒤에는 파일시스템을 의심한다.** 2026-11-03 에 블록
비트맵이 깨져 있었다. 루트가 `rw` 로 올라와 부팅 fsck 가 돌지 않으므로 `swapoff`
→ `sysrq u` → `e2fsck -fn`/`-fy` → `sysrq b` 로 그 카드에서 고쳤다(`NEXT.md` 0절).

**재부팅 명령을 두 번 보내지 않는다.** 부팅 도중에 겹치면 보드가 네트워크에서
사라져 전원을 껐다 켜야 한다(2026-11-03 에 겪었다). 90초 기다렸다가
`grep CmaFree /proc/meminfo` 로 확인한다. 25만 kB 근처면 정상이다.

## 5. 자동화 구조

모델 하나는 `models/<이름>.conf` 하나로 기술한다. 푸시하면 보정 프로파일
생성 → 번들 컴파일 → 보드 전송 → 추론 → 정답 대조까지 돌고 결과가
`results/latest.md` 로 저장소에 다시 커밋된다.

| 워크플로 | 하는 일 |
|---|---|
| `model-test` | `models/*.conf` 전부 (푸시 자동) |
| `multievta` | `models-multi/*.conf`, 멀티 코어 처리량 |
| `debug-dump` | 층별 중간 텐서 덤프 (수동) |
| `board-recover` | 보드 점검·강제 종료·재부팅 (수동) |
| `board-health` | 2시간마다 보드 점검 |

**보드는 한 대뿐이라 전부 `concurrency: zcu102-board` 로 묶여 순서대로
돈다.** 이 그룹은 저장소 안에서만 통한다. nest-compiler 의 `test-board` 와는
서버의 `~/.zcu102-board.lock`(`scripts/board-lock.sh`)으로 순서를 맞춘다. GitHub 클라우드 러너는 교내 방화벽 때문에 보드에 못 닿는다
(종료 코드 255). 전부 self-hosted 여야 한다.

상류 패치는 **손으로 고치지 않고 `ci/patch_*.py` 로 유지한다.** 매 실행마다
다시 적용되므로 상류 트리를 통째로 옮기지 않아도 된다. 기계를 바꿀 때
이 선택이 값을 했다.

## 6. 알려진 함정

러너는 **연구실 서버 `sota-AIP`(192.168.1.150) 의 `ehdgns` 계정**에 있다
(2026-11-04 이전, 이름 `sota-aip`, systemd 서비스라 재부팅해도 뜬다). CI 가
큐에만 쌓이면 Actions → Settings → Runners 에서 Offline 인지 먼저 본다.
접속은 개인 PC WSL 의 `ssh lab`(키 `~/.ssh/id_ed25519`, 외부 경로
`sota.pusan.ac.kr:25622`). 개인 PC 가 유선(10.126.x)이면 `192.168.1.x` 내부망에
안 닿으므로 외부 경로가 기본이다. 같은 공유기 Wi-Fi 일 때만 `ssh lab-lan`.

```sh
ssh lab 'cd ~/actions-runner && sudo ./svc.sh status'
```

**공용 서버다.** 다른 사용자(sota, jiho, furio …)가 있으니 docker 와 내 계정
밖은 건드리지 않는다. 컴파일러 트리 `~/nestc-upstream` 은 개인 PC 에서
rsync 로 옮긴 것이다(1.4GB, 빌드 생략).

PC 가 Wi-Fi 라 **보드로 가는 SSH 가 중간에 끊긴다.** 2026-11-03 에 보드에서는
빌드가 끝났는데 세션이 안 돌아와 단계가 매달렸다. keepalive 를 60초로
줄이고 재시도를 넣었다.

`make ... | tail -N` 에 `set -o pipefail` 이 없으면 **빌드 실패가 조용히
통과한다.** 종료 코드가 `tail` 의 것이 된다.

보드는 Ubuntu 18.04(glibc 2.27)라 **GitHub Actions 러너가 설치되지 않는다.**
도커 이미지도 amd64 전용이다. 보드를 CI 서버로 쓸 수 없다. 시험 대상 위에
시험 장비를 올리면 보드가 멎을 때 같이 죽는다는 문제도 있다.

`-instrument-debug` 는 VTA 백엔드에서 아무 일도 하지 않는다(LLVM 백엔드
기능). 층별 덤프는 `ci/inject_layer_dump.py` 가 생성된 번들 소스에 직접
넣는다.

## 7. 어디에 무엇이 있나

| 경로 | 내용 |
|---|---|
| `NEXT.md` | 남은 일. **먼저 읽는다** |
| `docs/14-wrapup-2026-11-05.md` | **마무리 정리** — 성능, 한계, 사용법 |
| `docs/13-status-2026-11-02.md` | 11/02 현황. 12번은 성능 수치가 낡았다 |
| `docs/08-resnet18-manual.md` | 보드에서 손으로 돌리는 전 과정 |
| `docs/10-lab-server-setup.md` | CI 서버 구축. 5-a 결정, 5-b 함정, 4-b 콘솔 |
| `docs/11-automation.md` | 자동화 구조와 고장났을 때 볼 곳 |
| `patches/` | 상류 결함 다섯 건의 분석 |
| `ci/patch_*.py` | 상류에 적용하는 패치. 전부 멱등 |
| `backups/` | SD 카드 복제본 14GB. git 에 없다. 지우지 않는다 |
| `.upstream-ref/` | 상류 소스 사본. ETRI 저작권이라 공개 금지 |

`backups/` 와 `.upstream-ref/` 는 `.gitignore` 에 있다. 새 기계에서는
저장소를 클론한 뒤 `backups/` 를 따로 복사해 넣어야 한다
(`checksums.md5` 로 검증).
