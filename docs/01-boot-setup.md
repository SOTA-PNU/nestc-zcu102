# 부팅 설정

## 부트 모드 스위치

SD 부트를 쓰려면 **SW6** 를 다음과 같이 맞춘다.

```
SW6
┌─────────────────┐
│ 1    2   3   4  │
│ ON  OFF OFF OFF │
└─────────────────┘
```

U-Boot 로그에 `Bootmode: LVL_SHFT_SD_MODE1` 이 찍히면 SD 부트로 잡힌 것이다.

## UART 콘솔

ZCU102는 Silicon Labs **CP2108** 을 통해 USB 포트 하나로 시리얼 4개를 노출한다.
Windows 장치 관리자에서 COM 포트 4개가 한꺼번에 잡힌다.

| Interface | 역할 |
|-----------|------|
| **0** | **Linux 콘솔** ← 여기로 접속 |
| 1 | PS UART1 |
| 2 | — |
| 3 | System Controller (BMC). `Press ESC to enter System Controller mode` 출력 |

번호가 순서대로 배정되지 않으니 Interface 번호로 판단해야 한다.
한 예로 Interface 0 = COM9, Interface 3 = COM8 로 잡힌 적이 있다.

설정은 **115200 8N1**. PuTTY에서 Connection type을 Serial로 두고
Serial line에 COM 번호, Speed에 115200을 넣는다.

### WSL에서는 시리얼이 안 보인다

WSL은 USB 장치를 그대로 넘겨받지 못해 `/dev/ttyUSB*` 가 생기지 않는다.
UART는 Windows 쪽 PuTTY나 Tera Term으로 붙어야 한다.
`usbipd-win` 으로 넘길 수도 있지만 번거롭고, 부팅 후에는 SSH가 훨씬 편하다.

## 부팅 흐름

전원을 넣으면 이 순서로 진행된다.

```
FSBL (Xilinx Zynq MP First Stage Boot Loader, Release 2018.3)
  → ATF / BL31
  → PMUFW
  → U-Boot 2018.01
  → BOOTP broadcast 1..17  ← 네트워크 부팅 시도, 전부 실패함
  → Retry time exceeded
  → SD에서 자동 부팅
  → PYNQ Linux
```

**BOOTP broadcast가 17회 반복되는 것은 정상이다.** U-Boot 환경변수가
네트워크 부팅을 먼저 시도하도록 되어 있어서 그렇고, 타임아웃 후 SD로 넘어간다.
1~2분 기다리면 된다. 중간에 키를 누르면 U-Boot 프롬프트(`ZynqMP>`)로 빠지는데,
이때는 `boot` 를 입력하거나 재부팅하면 된다.

`sdboot` 환경변수는 정의되어 있지 않으므로 `run sdboot` 는 동작하지 않는다.

로그인은 자동이다 (`xilinx` 계정).

## FPGA 비트스트림

**VTA 비트스트림은 `BOOT.BIN` 안에 포함되어 부팅 시 FPGA에 자동 로드된다.**
BOOT.BIN이 27 MB로 큰 이유가 이것이다. 별도로 `.bit` 파일을 올릴 필요가 없다.

SD 부트 파티션에서 `.bit` 파일을 찾아도 나오지 않는 것이 정상이다.

## SSH 접속

부팅 후 UART에서 IP를 확인한다.

```bash
ip addr show eth0
```

이후로는 SSH가 편하다.

```bash
ssh xilinx@<IP>
```

### 호스트 키 경고가 뜰 때

DHCP로 IP가 돌려 쓰이면서 예전 장비의 키가 `known_hosts` 에 남아 있으면
`REMOTE HOST IDENTIFICATION HAS CHANGED!` 경고가 뜬다. 중간자 공격이 아니라
IP 재할당 때문이다. 접속 시 표시되는 지문이 보드의 것과 같은지 확인한 뒤
낡은 항목을 지운다.

```bash
ssh-keygen -f ~/.ssh/known_hosts -R "<IP>"
```

이 파일은 PC 안의 기록일 뿐이라 지워도 보드에는 아무 영향이 없다.
