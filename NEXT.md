# 다음에 할 일 (2026-11-02 기준)

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

## 3. CI 를 연구실 서버로 — vcu118-dev

진행 상황과 걸린 것들은 `docs/10-lab-server-setup.md` 5-b 절에 있다.

끝난 것: 교내망 확인(보드 25022 열림), 도커 설치, 서버→보드 SSH 키 등록
(비밀번호 없이 들어감).

남은 것:

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
