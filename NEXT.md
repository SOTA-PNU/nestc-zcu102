# 다음에 할 일 (2026-11-03 기준)

## 0. 지금 어디까지 왔나 (2026-11-03 저녁)

`VTA_MAX_XFER` 패치를 CI 에 연결하고 푸시했다(`cb594fb`, `c79a458`).
**패치는 보드에 적용되는 것까지 확인했다** — 로그에
`VTA_MAX_XFER 1<<25 -> 1<<24` 와 `45:#define VTA_MAX_XFER (1<<24)` 가
찍혔고, `libVTABundle.a` 와 `libvta_runtime.a` 가 1~2분 만에 다시
지어졌다.

**4코어가 진짜로 돈다** (2026-11-03 18:12 KST, 보드에서 손으로 실측).

| 코어 | 100장 | 장당 | 처리량 | 단일 대비 | oops |
|---|---|---|---|---|---|
| 1 | — | 91.6ms | 10.9 img/s | 1.0 | — |
| 2 | 5485ms | 54.9ms | 18.23 img/s | 1.67 | 0 |
| 4 | 4241ms | 42.4ms | 23.58 img/s | 2.16 | 0 |

CI(multievta, 18:19 KST, `e515d87`)에서도 같다: 2코어 18.26 img/s, 4코어
23.86 img/s, oops 0. 오답 0건, 코어당 CMA 44.8MB, 끝난 뒤 전부 반납.

**ResNet-50 이 보드에서 돈다** (2026-11-04, model-test 41813475989, 서버 러너).
281 / 207 / 340 전부 정답, 장당 약 419ms(ResNet-18 의 4.6배). `models/resnet50.conf`
는 상류 `vta/bundles/Resnet50Test` 레시피(Caffe2 판, GitLab 미러에서 받음)다.
막힌 것은 셋이었고 전부 **main 틀(ResNet-18 용) 쪽**이었다 — 입력 심볼이
`"data"` 로 박혀 있었고(죽으면서 CMA 98MB 누수, 보드 재부팅으로 회복), NCHW
입력인데 NHWC 로 전치했고, 채널을 RGB 로 넣었다(Caffe2 는 BGR). 마지막 것은
ETRI 사전 빌드 번들과 맞대어 갈랐다: ETRI 번들+상류 main 은 정답, ETRI
번들+우리 main 은 개 209, 우리 번들+우리 main 은 170, 둘 다 BGR 로 바꾸면 정답.
**번들 코드는 결백했다.** 그 과정에서 `gen-bundle` 의 `CHANNEL_ORDER` 뜻이
거꾸로 적혀 있던 것을 바로잡았다(기존 conf 동작은 그대로). SqueezeNet 도 같은
틀을 쓰므로 채널 순서·NHWC 전치가 오답의 후보로 다시 열린다(종결 상태, 손대지 않음).
확신도가 0.002 로 찍히는 것은 모델 출력이 이미 softmax 인데 main 이 한 번 더
거는 탓이라 순위에는 영향이 없다.

**ResNet-50 멀티 EVTA** (2026-11-04, 보드 실측, 같은 4코어 비트스트림).

| 코어 | 처리량 | 1코어 대비 | 실행 중 최저 CMA 여유 |
|---|---|---|---|
| 1 | 2.43 img/s | 1.00 | 194 MB |
| 2 | 4.29 img/s | 1.77 | 135 MB |
| 3 | 5.59 img/s | 2.30 | 33 MB |
| 4 | 6.47 img/s | 2.66 | **2.8 MB** |

전부 정답, oops 0. ResNet-18(2.04배)보다 잘 늘어나는 것은 한 장에서 VTA 가
차지하는 몫이 더 커서 CPU 경합의 영향이 작기 때문으로 보인다(연산별 계측은
안 함). **4코어는 겨우 들어간다** — 코어당 명령 큐 32MB + 가중치 25.6MB.
`models-multi/` 에 3코어(안정)와 4코어를 둘 다 넣었고, `board-multi-run.sh`
의 CMA 관문을 고정 50MB/코어에서 `코어 x (32MB + 가중치) + 4MB` 로 바꿔
누수가 조금이라도 있으면 4코어를 시작하지 않게 했다.

**4배가 아닌 이유를 쟀다** (2026-11-03 23:00 KST, `results/scaling-20261103.md`).
같은 멀티 비트스트림에서 1/2/3/4 스레드 = 11.66/18.20/21.88/23.79 img/s
(2.04배). 연산별로 재 보니 1스레드 한 장 86ms 중 **VTA 합성곱은 26ms(30%)
뿐**이고 나머지는 CPU 일이다 — 레이아웃 transpose 29ms, 첫 7x7 합성곱
14.5ms(`num_threads(4)` 고정 OpenMP 로 CPU 4개를 다 빌려 씀), 매 장 VTA 버퍼
할당 4ms 등. 4스레드면 CPU 합성곱 +35ms, transpose·버퍼 할당·VTA 합성곱
각 +12ms. 간섭 실험(CPU 부하 vs 메모리 부하)으로 앞의 것은 CPU 경합, 나머지는
메모리 대역폭임을 갈랐다. 결함이 아니라 구조적 특성이다. 개선안(VTA 층 사이
transpose 제거, 버퍼 재사용)은 상류 컴파일러 변경이라 하지 않았다.

필요했던 것은 셋이다.

1. **4코어 비트스트림.** 부팅 기본 `/root/bitstream/vta.bit`(rc.local 이 올림,
   `vta_wrapper` 2020/08/04, ETRI 공개본 어느 것과도 해시가 다름)은 **단일
   VTA** 였다. 디바이스 트리에도 코어 0 만 있다. ETRI `nest-data` 의
   `bitstreams/zcu102_4_1x16_i8w8a32_16_15_18_17.bit`(Git LFS, 26,510,896 B,
   sha256 `7d550bef…dc24056f`)를 올리면 코어 0~3 의 fetch/load/compute/store
   16개가 모두 `ap_idle`(0x4)로 응답한다. 보드의 `~/bitstreams/` 에 있다.
2. **멀티 구성 보드 런타임.** 상류 `vta/vtalib/CMakeLists.txt` 는
   `NESTC_EVTA_MULTI=ON` 일 때 버퍼를 15/18/17 로 잡는다(단일은 16/19/18).
   비트스트림 이름의 뒤 숫자와 같다. 보드에 `build_aarch64_multi` 를 만들었다
   (`scripts/board-setup-multi.sh`, vtalib 만, 1분).
3. **컴파일러는 그대로다.** VTA 백엔드와 타일링 표는 버퍼 구성을 참조하지
   않는다. 같은 번들을 멀티 런타임과 링크하면 된다 — 오답 0건으로 확인.

CI: `scripts/board-bitstream.sh single|multi` 가 비트스트림을 갈아 끼우고(해시
검증, 코어 응답 확인, VTA 사용 중이면 거절) model-test 와 multievta 가 4코어
전에 multi, 끝나면 항상 single 로 되돌린다. 단일 코어 기준값 0.478113 은
단일 비트스트림에서 잰 것이라 섞지 않는다. 재부팅하면 rc.local 이 single 을
올린다.

### 그 전에 "4코어가 돌았다" 고 했던 것은 틀렸다

multievta 41605026990(17:42 KST)은 "전부 통과" 로 찍혔지만 보드 dmesg 에
그 실행 중의 커널 oops 가 남아 있었다.

| 시각 (KST) | 실행 | dmesg |
|---|---|---|
| 17:41:06 | 2코어 | 스레드 1개 `SError` (Oops #1) |
| 17:41:55 | 4코어 | 스레드 3개 `SError` (Oops #3~#5) |
| 17:49 | model-test 에 붙인 4코어 | 스레드 3개 이상 `SError`, 프로세스가 매달림 |

네 스레드가 공유 카운터로 100회를 나눠 가지므로, 코어 1~3 스레드가 죽어도
코어 0 이 100장을 다 처리하고 시간 줄과 오답 0건이 찍힌다. 수치도 정확히
그렇다 — 1코어 91.6ms, "2코어" 88.8ms, "4코어" 93.9ms 가 모두 단일 코어
속도다. **CLAUDE.md 의 "DDR 경로 공유" 결론은 이 착시를 읽은 것일
가능성이 크다.** 11/02 실행은 재부팅으로 dmesg 가 사라져 확인할 수 없다.

살아 있는 사실: `VTA_MAX_XFER` 패치로 네 컨텍스트가 CMA 에 들어간다(코어당
44,816 kB, 합계 179MB). 메모리는 더 이상 막는 원인이 아니다.

SError 는 비동기 버스 오류다. 단일 비트스트림에는 코어 1~3 주소에 응답하는
것이 없어서 났다(위 1번).

조치: `board-multi-run.sh` 가 실행 전후 oops 수를 비교해 늘면 실패로 판정하고,
실행을 5분에서 끊는다. model-test 에 4코어를 붙였던 커밋(`02b10ec`)은
되돌렸다(`f069258`).

**SD 카드 파일시스템을 고쳤다** (2026-11-03 19:40 KST, 복제본 교체 없이 그 카드에서).

증상은 둘이었다. `EXT4-fs error ... bad block bitmap checksum`(파일시스템 장부
손상, `tune2fs` 상태 `clean with errors`, 오류 12건, 첫 오류 11/03 08:08 = 전원
차단 직후 부팅)과 `mmcblk0: error -84`(전송 CRC 오류, 10/30~11/03 사이 간헐적,
수리 전 부팅부터 0건, 문제 섹터도 다시 읽힌다). 앞의 것은 비정상 종료(전원 차단,
SError oops, 추론 중 강제 종료)에서 왔고 9/22 복제 이후 fsck 를 한 번도 안 했다.
뒤의 것은 카드 접촉·신호 쪽으로 보이나 카드 상태를 단정하지는 못했다.

커널이 루트를 `rw` 로 올려 부팅 fsck 가 돌지 않으므로(부팅 인자는 안 건드린다)
이렇게 했다: 중요 파일 백업(`backups/board-files-20261103/`, 원본 vta.bit 해시
확인) → `swapoff` → `sysrq u`(루트 읽기 전용) → `e2fsck -fn` 으로 범위 확인 →
`e2fsck -fy` → 재점검 exit 0 → `sysrq b` 한 번. 고친 것은 블록 공유 1건
(logrotate 상태 파일과 그 옛 판), 떨어진 inode 2개(Jupyter 런타임 JSON, 옛
logrotate 상태 → lost+found), 링크 수 1건(syslog.7.gz), 비트맵·여유 블록 수.
지워진 데이터는 없다. 로그는 `backups/board-files-20261103/fsck-20261103.log`.

수리 후: 상태 `clean`, 부팅·4코어 작업 뒤에도 EXT4/mmc 오류와 oops 0건.

**재발 방지.** 전원을 그냥 끊지 않는다. 커널 oops 나 전원 차단을 겪은 뒤에는 같은
방법으로 `e2fsck -fn` 부터 본다. `mmcblk0: error -84` 가 다시 늘면 카드를
의심한다.

오늘 세 번 막혔는데 전부 보드가 아니라 러너 쪽이었다.

러너가 두 번 Offline 이 되었다. WSL 가상머신이 통째로 자동 종료된
것이다(다시 띄웠을 때 `Main PID: 199`). 시작 폴더의 `wsl-runner.cmd` 는
부팅 때 한 번만 깨우므로 그 뒤에 내려가면 못 일으킨다. 고장 진단은 항상
**Settings → Actions → Runners 에서 Offline 인지 보는 것부터** 한다.

17:32 에 세 번째로 Offline 이 되어(`wsl -l -v` 가 Stopped) 고쳤다. 시작
폴더의 `wsl-runner.cmd` 를 `/bin/true` 대신 숨김 창의
`wsl -e sleep infinity` 로 바꿔 WSL 을 상시 붙잡아 둔다. 사본은
`scripts/windows/wsl-runner.cmd`. **아직 PC 재부팅 뒤에 실제로 붙는지는 못 봤다** —
다음 로그인 때 `wsl -l -v` 가 Running 이고
`pgrep -a sleep` 에 `sleep infinity` 가 있는지 본다.

PC 가 Wi-Fi 라 보드로 가는 SSH 가 끊겼다. 보드에서는 빌드가 끝났는데
세션이 안 돌아와 단계가 매달렸다. keepalive 를 60초로 줄이고 재시도
세 번과 30분 제한을 넣었다(`c79a458`).

보드 재부팅 명령을 두 번 보내 보드가 네트워크에서 사라졌다. 전원을
껐다 켜서 살렸다. **재부팅은 한 번만 보내고 90초 기다린다.**

위 세 가지(CMA 관문, 끝까지 도는지, 코어당 약 45,000 kB)는 17:42 실행에서
겉으로는 다 맞았다. 그러나 그 세 기준은 코어 1~3 이 죽는 것을 못 잡는다 —
위 표를 본다.

---


남은 목표는 셋이다. **4코어 멀티 EVTA**, **CI 를 연구실 서버로**, 그리고
**마무리 보고**. SqueezeNet 오답 추적은 종결했다(4절).

---

## 1. 4코어 — 빌드 플래그부터 확인한다

지금까지 "멀티코어가 안 빨라진다" 고 본 것이 하드웨어 한계가 아니라
**빌드 플래그 하나**일 가능성이 크다.

측정값부터 보면, 2코어로 100장을 9074ms 에 처리했다. 스레드당 50장이니
장당 181ms 인데 단일 코어는 91.6ms 다. **정확히 2배 느려졌고 총 처리량은
그대로다.** 겹쳐 돌렸더니 각자 딱 그만큼 느려졌다는 뜻이다.

원인 후보를 좁히다 드라이버에서 이것을 찾았다.

```c
// vta/vtalib/include/zcu102/pynq_driver.h:63
#ifndef VTA_DEVICE_NUM
#define VTA_DEVICE_NUM 1
#endif
```

컴파일할 때 `-DVTA_DEVICE_NUM=4` 를 주지 않으면 드라이버는 코어가 하나인
줄 안다. 그러면 번들이 `idEVTA` 로 1·2·3번 코어를 가리켜도 전부 0번으로
간다. 두 스레드가 같은 코어를 두드리니 각자 2배 느려진다 — 측정값과
정확히 맞는다.

**확인할 것.** 보드의 `build_aarch64` 가 그 플래그를 넘기는가.

```sh
alias b='ssh -i ~/.ssh/board_key -p 25022 xilinx@sota.pusan.ac.kr'
b 'grep -rh VTA_DEVICE_NUM nest-compiler-2024/build_aarch64/vta/vtalib/CMakeFiles/*/flags.make'
```

아무것도 안 나오면 안 넘기고 있는 것이다. 그러면 `-DVTA_DEVICE_NUM=4` 로
vtalib 를 다시 빌드하고 2코어를 재측정한다. 처리량이 두 배가 되면 거기서
끝이고, 그대로면 하드웨어가 경로를 공유하는 것이니 그렇게 보고한다.

## 2. 4코어 — 메모리 64MB 의 출처

코어당 실측 77,616 kB 인데 번들이 요구하는 것은 가중치 12,003,168 바이트
와 활성 606,112 바이트, 합쳐 12.6MB 뿐이다. **65MB 가 설명되지 않는다.**
중간에 죽는 모델이 들고 가던 양(정확히 64MB)과 같은 숫자다.

`dat_queue.h` 의 `kMaxBytes` 가 그 자리로 보인다. 상수가 아니라 템플릿
인자다(`template<int kMaxBytes, bool kCoherent, bool kAlwaysCache>`).
큐를 만드는 쪽에서 넘기는 값을 찾으면 줄일 수 있는지 보인다.

```sh
b 'grep -n "Queue<" nest-compiler-2024/vta/vtalib/include/zcu102/api.h'
```

줄일 수 있으면 4×(작은 풀 + 13MB) 가 되어 256MB 안에 들어간다. **그러면
부팅 인자를 건드리지 않아도 된다.** 지금 계산으로는 76×4=304MB 라 안
들어간다.

부팅 인자로 CMA 를 늘리는 길은 **마지막 수단**이다. 잘못 넣으면 보드가
부팅되지 않고 누군가 현장에서 SD 카드를 꺼내야 한다. 연구실에서 UART
콘솔을 볼 수 있을 때만 한다.

## 3. CI 를 연구실 상시 PC 로 — **끝 (2026-11-04)**

연구실 서버 `sota-AIP`(192.168.1.150, Ubuntu 24.04, 24코어, 62GB, 공용)로
옮겼다. 계정 `ehdgns`(sudo, docker), 러너 `sota-aip`(systemd). 도커 설치,
이미지 pull(30.6GB, 개인 PC 와 같은 ID), 상류 트리는 WSL 에서 rsync(빌드
생략). 첫 model-test(41809028759)가 서버에서 통과 — resnet18 3/3,
4코어 24.20 img/s. 개인 PC 러너 `ehdgns` 는 서비스를 끄고 GitHub 에서
지웠다. 이제 개인 PC 를 꺼도 CI 가 돈다. 아래는 당시 기록이다.

### (기록) 옮기기 전 계획

**2026-11-02 변경: vcu118-dev 를 쓰지 않는다.** 연구실에 상시 켜 두는
새 PC 가 들어오므로 거기에 올린다. 근거는 `docs/10-lab-server-setup.md`
5-a 절. 요약하면 vcu118-dev 는 VirtualBox NAT 뒤라 밖에서 못 들어오고,
DNS 가 다섯 번에 한 번 죽고, 15GB 메모리를 다른 프로젝트의 8시간짜리
Vivado 합성과 다툰다. 새 PC 는 셋 다 해당하지 않는다.

새 PC 가 올 때까지는 **지금 개인 PC 의 러너를 그대로 둔다.** 잘 돌고
있으므로 공백은 없다.

vcu118-dev 에서 하던 것(도커 이미지 받기, 상류 클론, cmake)은 멈췄다.
남긴 것은 `/etc/hosts` 의 보드 주소 한 줄과 보드 접속 키뿐이다 —
손으로 보드를 확인할 때 쓸모가 있다.

하루 꼬박 들였지만 버린 시간은 아니다. 서버 구축에서 걸리는 것들을 전부
밟아 `docs/10` 5-b 절에 적어 두었고, 새 기계에서 그대로 쓴다.

새 PC 에서 할 일 (순서대로, 자세한 것은 `docs/10`):

도커 이미지 받기와 상류 클론을 마친다. 둘 다 한 번씩 네트워크로 끊겼다
(`connection reset by peer`, `Could not resolve host: github.com`).
이어받기가 되므로 같은 명령을 다시 걸면 된다. 두 번 이상 DNS 로 실패하면
가상머신의 resolver 를 고정한다.

```sh
nohup docker pull leejaymin/nestc-ssh:latest > ~/pull2.log 2>&1 &
cd ~/nestc-upstream && nohup git submodule update --init --recursive > ~/sub.log 2>&1 &
```

그 다음 컨테이너 안에서 컴파일러를 빌드한다. **Vivado 가 끝나 메모리가
14Gi 비었으므로 지금 해도 된다.** 절차는 `docs/03-build-notes.md`,
S3 버킷 우회(aws 대체 스크립트)는 `docs/08-resnet18-manual.md` 4.1절.

마지막으로 러너를 등록한다(`docs/10` 8절). 등록이 끝나고 한 번 돌려 본
뒤에 개인 PC 의 러너를 해제한다. 둘 다 켜 두면 작업이 아무 데나 떨어지는데,
서버 쪽 빌드 트리가 준비되기 전에 떨어지면 실패한다.

## 4. SqueezeNet — 종결

보드에서 905/904/533 으로 틀리는 문제는 **원인을 찾지 못한 채 종결한다.**
`models/` 에서 빼 `models-off/` 로 옮겼다. 돌아가는 CI 에는 없다.
(2026-11-03 정정: 이 문장과 달리 파일은 `models/` 에 남아 model-test 가 계속
"알려진 실패" 로 돌리고 있었다. 이날 실제로 옮겼다.)

남긴 이유는 그 과정에서 찾은 상류 결함들이 보고할 값이 있기 때문이다.
배제 목록과 근거는 `docs/13-status-2026-11-02.md` 4절과 `patches/` 의
문서들에 있다. 다시 손대려는 사람은 그것부터 읽으면 같은 곳을 두 번 파지
않는다.

## 5. 마무리 보고

상류(ETRI, yongin.kwon@etri.re.kr)에 보낼 것이 다섯 건이다.

Concat 이 AOT 번들 경로에 없고 미지원 명령이 `llvm_unreachable` 로 빠지는
것, 튜닝 표에 없는 레이어의 타일링 기본값, 독립 ReLU 재양자화의 int8 포화
누락, 재양자화 버림(정확도 개선 제안), 그리고 번들이 소리 없이 죽으면서
CMA 를 반납하지 않는 것.

보낼지와 범위는 교수님 판단이 필요하다. 비트스트림이 multi-EVTA 4코어가
맞는지도 함께 여쭌다(`docs/12-status-2026-10-30.md` 참조).
