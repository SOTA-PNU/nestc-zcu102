# 연구실 서버 구축 — 보드 연결부터 팀 공용까지

> **2026-10-30 추가 — 서버는 교내망 안에 있어야 한다.**
> 보드 감시와 복구를 GitHub 클라우드 러너에서 돌려 보았으나 첫 SSH 에서
> 종료 코드 255(연결 실패)로 죽었다. 보드가 `sota.pusan.ac.kr:25022` 로
> 공개되어 있는데도 그렇다 — 교내 방화벽이 외부 IP 를 막는 것으로 보인다.
> 따라서 CI 서버 후보는 **교내망 안**에 있어야 하고, 2절의 연결 확인이
> 더더욱 첫 단계다.

작성 2026-10-30 · donghun (PNU-SOTA)

---

## 0. 무엇을 옮기는가

지금은 donghun 개인 PC 가 세 가지 역할을 하고 있다. 그것을 연구실 전용 PC 로
옮겨, 팀원 누구나 푸시만 하면 보드에서 추론이 돌게 만드는 것이 목표다.

| 역할 | 현재 | 이후 |
|---|---|---|
| 번들 생성 (도커 + 컴파일러) | 개인 PC 의 WSL | 연구실 서버 |
| GitHub Actions 러너 | 개인 PC 의 WSL | 연구실 서버 |
| 보드 SSH 통로 | 개인 PC | 연구실 서버 |
| 추론 실행 | ZCU102 보드 | 그대로 |

보드는 바뀌지 않는다. 서버는 보드에게 명령을 보내는 쪽일 뿐이다.

**왜 옮기는가.** 개인 PC 가 꺼져 있으면 팀원이 푸시해도 아무 일도 일어나지
않는다. 상태 점검 워크플로도 돌지 않아 보드가 멎어도 모른다. 서버는 계속
켜져 있으므로 둘 다 해결된다.

---

## 1. 서버가 갖춰야 할 것

| 항목 | 최소 | 이유 |
|---|---|---|
| CPU 아키텍처 | x86_64 | 도커 이미지가 amd64 전용. ARM 서버는 불가 |
| OS | Ubuntu 22.04 권장 | 윈도우면 WSL2 를 거쳐야 해 단계가 늘어난다 |
| 디스크 | 100GB 이상 | 도커 이미지 37GB + 상류 1.1GB + 빌드 30GB + 러너 작업 공간 |
| 메모리 | 16GB 이상 | 컴파일러 빌드 |
| 코어 | 8개 이상 | 적으면 빌드가 몇 시간이 된다 |
| 네트워크 | 보드와 GitHub 양쪽 | 아래 2장에서 먼저 확인한다 |

윈도우 PC 밖에 없다면 WSL2 를 쓰되, 네트워크 모드를 미러로 바꿔야 한다
(7장 참조). 개인 PC 에서 이것 때문에 CI 첫 실행이 실패했다.

---

## 2. 가장 먼저 — 서버가 보드에 닿는지 확인한다

다른 무엇보다 이것을 먼저 한다. 여기서 막히면 나머지가 전부 헛수고다.

서버에서:

```sh
ping -c 2 sota.pusan.ac.kr
nc -zv -w 5 sota.pusan.ac.kr 25022
```

포트가 열려 있다고 나오면 통과다. 닿지 않으면 세 가지를 의심한다. 서버가
연구실 망 밖에 있거나, 방화벽이 막고 있거나, WSL2 의 NAT 안에 갇혀 있거나.

**보드의 사설 주소(`192.168.1.40`)를 쓰면 안 된다.** 그 주소는 보드가 물린
공유기 안에서만 유효하다. 바깥에서는 포워딩 주소 `sota.pusan.ac.kr:25022` 를
쓴다. 개인 PC 에서 사설 주소로 시도하다 `Connection timed out` 으로 실패한
적이 있다.

---

## 3. 서버 기본 설정

```sh
sudo apt update && sudo apt install -y docker.io git curl tmux
sudo usermod -aG docker $USER
```

그룹 변경은 다시 로그인해야 적용된다. 확인:

```sh
docker ps >/dev/null && echo "docker OK" || echo "권한 없음 — 재로그인 필요"
```

---

## 4. 보드 접속용 SSH 키

서버에서 키를 만들고 공개키를 보드에 등록한다. 개인 PC 의 키를 복사해 쓰지
않는다 — 기계마다 따로 두어야 나중에 하나만 폐기할 수 있다.

```sh
ssh-keygen -t ed25519 -f ~/.ssh/board_ci -N ""
cat ~/.ssh/board_ci.pub
```

출력된 한 줄을 보드의 `~/.ssh/authorized_keys` 에 추가한다. 보드에서:

```sh
mkdir -p ~/.ssh && chmod 700 ~/.ssh
echo '<복사한 공개키 한 줄>' >> ~/.ssh/authorized_keys
chmod 600 ~/.ssh/authorized_keys
```

`ssh-copy-id` 가 편하지만 깨지는 경우가 있어 직접 넣는 쪽이 확실하다.
공개키는 노출되어도 안전하다. 개인키(`board_ci`, 확장자 없는 쪽)는 절대
공유하지 않는다.

서버에서 확인:

```sh
ssh -i ~/.ssh/board_ci -p 25022 -o PasswordAuthentication=no xilinx@sota.pusan.ac.kr 'echo OK; uname -m'
```

`OK` 와 `aarch64` 가 나와야 한다.

---

## 5. 보드 쪽 준비

한 번만 하면 된다. 이미 되어 있다면 건너뛴다.

**sudo 비밀번호 생략.** 워크플로가 `sudo ctest` 를 비대화식으로 돌린다.

```sh
echo 'xilinx ALL=(ALL) NOPASSWD: ALL' | sudo tee /etc/sudoers.d/xilinx-ci
sudo chmod 440 /etc/sudoers.d/xilinx-ci
sudo -n true && echo NOPASSWD-OK
```

**aws shim.** 삭제된 S3 버킷을 GitLab 미러로 우회한다. 내용은
`docs/08-resnet18-manual.md` 4.1절에 있다. 이미 `~/bin/aws` 가 있으면 된다.

**상류 클론과 패치.** `~/nest-compiler-2024` 가 있고 `insert_tensor` 가
vtalib 에 들어가 있어야 한다.

```sh
nm -C ~/nest-compiler-2024/build_board/vta/vtalib/libVTABundle.a | grep -c insert_tensor
```

---

## 6. 서버에 컴파일러 빌드

도커 이미지는 **환경만 제공한다.** `model-compiler` 는 들어 있지 않으므로
직접 빌드해야 한다. 호스트 디렉터리를 마운트하고 그 안에서 빌드하면 결과물이
호스트에 남는다.

```sh
docker pull leejaymin/nestc-ssh:latest
git clone --recursive https://gitlab.com/ones-ai/nest-compiler.git ~/nestc-upstream
docker run -dit --name nestc -v ~/nestc-upstream:/root/nestc leejaymin/nestc-ssh:latest bash
docker exec -it nestc bash
```

컨테이너 안에서 빌드한다. 절차는 `docs/03-build-notes.md` 를 따른다.
Concat 패치도 적용한다.

```sh
python3 /root/nestc/ci/patch_concat.py /root/nestc
cd /root/nestc/build && ninja model-compiler image-classifier
```

완료 확인:

```sh
ls -l ~/nestc-upstream/build/glow/bin/model-compiler
```

이 바이너리는 컨테이너 환경에서만 실행된다. 호스트에서 직접 돌리면
`libprotobuf.so.17` 을 못 찾는다. 워크플로가 도커로 감싸 실행하는 이유다.

---

## 7. 윈도우 서버인 경우에만 — WSL2 네트워크

WSL2 는 기본적으로 NAT 안에 있어 같은 망의 다른 기기에 닿지 못한다.
미러 모드로 바꾼다. 윈도우의 `%UserProfile%\.wslconfig` 에:

```
[wsl2]
networkingMode=mirrored
```

그다음 `wsl --shutdown` 하고 다시 연다. 2장의 확인을 다시 해 본다.

---

## 8. GitHub Actions 러너 등록

저장소 Settings → Actions → Runners → New self-hosted runner 에서 Linux / x64
를 고르면 다운로드와 설정 명령이 토큰까지 포함되어 나온다. 그 페이지의 명령을
그대로 쓴다. 토큰은 한 시간 만료이며 등록용으로만 쓰인다.

```sh
mkdir -p ~/actions-runner && cd ~/actions-runner
# 페이지의 curl / tar / config.sh 명령을 붙여넣는다
sudo ./svc.sh install
sudo ./svc.sh start
sudo ./svc.sh status
```

`active (running)` 이면 된다. 라벨은 기본값(`self-hosted,Linux,X64`)을 그대로
둔다. 워크플로가 `[self-hosted, linux, x64]` 로 요구하는데 대소문자는 무시된다.

**개인 PC 의 러너는 이 시점에 제거한다.** 둘을 동시에 두면 작업이 아무 데나
배정되어 혼란스럽다.

```sh
# 개인 PC 에서
cd ~/actions-runner && sudo ./svc.sh stop && sudo ./svc.sh uninstall
./config.sh remove --token <제거용 토큰>
```

---

## 9. 저장소 시크릿

Settings → Secrets and variables → Actions 에서 네 개를 넣는다.
이미 있다면 `BOARD_SSH_KEY` 만 서버에서 만든 키로 교체한다.

| 이름 | 값 |
|---|---|
| `BOARD_HOST` | `sota.pusan.ac.kr` |
| `BOARD_PORT` | `25022` |
| `BOARD_USER` | `xilinx` |
| `BOARD_SSH_KEY` | 서버의 `cat ~/.ssh/board_ci` 출력 전체 |

개인키는 `-----BEGIN` 줄부터 `-----END` 줄까지 줄바꿈 포함 전부 넣는다.
터미널에서 복사해 입력란에 바로 붙여넣고, 중간에 다른 곳을 거치지 않는다.

---

## 10. 동작 확인

순서대로 돌려 본다. 뒤로 갈수록 오래 걸린다.

**1단계 — 상태 점검.** Actions 에서 `board-health` 를 `Run workflow`.
1분 안에 끝난다. 가동 시간, `/dev/xlnk`, 메모리, 디스크가 찍히면 서버에서
보드까지 길이 뚫린 것이다.

**2단계 — 모델 추론.** `model-test` 를 `resnet18` 로 실행.
번들 생성, 전송, 보드 빌드, 추론, 판정이 순서대로 돈다. 결과가
`cat_285 281 / dog_207 207 / zebra_340 340` 이면 전 구간 정상이다.

**3단계 — 상류 회귀.** `board-test` 를 실행. generic 과 aarch64 두 경로로
ZCU102 테스트 51개를 돌린다. 첫 실행은 빌드를 처음부터 하므로 한두 시간.

---

## 11. 운영 — 유지보수

**디스크 정리.** 러너 작업 공간과 도커 이미지가 쌓인다. 주 1회 정도.

```sh
docker system prune -f
rm -rf ~/actions-runner/_work/*/_temp/*
```

cron 에 걸어두면 편하다.

```
0 4 * * 0 docker system prune -f >/dev/null 2>&1
```

**보드는 한 대뿐이다.** 모든 워크플로가 `concurrency: zcu102-board` 그룹에
있어 동시에 돌지 않는다. 앞 작업을 취소하지 않고 기다린다 — VTA 실행 중
취소는 FPGA 와 xlnk 를 정리하지 못해 재부팅 전까지 보드를 못 쓰게 만든다.

**CI 는 한 저장소에만 건다.** `concurrency` 는 저장소 단위라 두 저장소가
같은 보드를 향하면 서로를 막지 못한다. `udonghun/nestc` 는 미러로 두고
Actions 를 꺼 둔다.

---

## 12. 팀원이 쓰는 법

권한을 받은 사람은 설정 파일 하나를 푸시하면 된다.

1. `models/<이름>.conf` 를 만든다 (`models/resnet18.conf` 를 복사해 수정)
2. ONNX 를 `models/onnx/` 에 넣는다 (100MB 초과 시 URL 로 지정)
3. 푸시한다 — `models/**` 변경이면 `model-test` 가 자동 기동
4. Actions 에서 결과를 본다

설정 파일을 채우기 전에 확인할 세 가지가 있다. 입력 이름과 형상, 연산자 목록,
전처리다. 전부 `onnx.load()` 로 읽는다. 자세한 절차와 막히는 지점별 원인은
`docs/08-resnet18-manual.md` 5.7절에 있다.

**기준값은 직접 떠야 한다.** 새 모델은 정답을 모르므로, PC 에서
`image-classifier -backend=Interpreter` 로 먼저 돌려 그 결과를 `EXPECT` 에
적는다. 이것이 없으면 CI 가 무엇을 기준으로 판단할지 알 수 없다.

**알아 둘 것.** 보드가 한 대라 앞 작업이 끝날 때까지 기다린다. 번들을 처음부터
만드는 작업이 앞에 있으면 한 시간 넘게 걸릴 수 있다.

---

## 13. 관련 문서

전체 매뉴얼은 `docs/08-resnet18-manual.md`, 상류 빌드를 뺀 축약판은
`docs/09-quickstart.md`, 경과와 로드맵은 `docs/07-progress-report.md`,
빌드 옵션은 `docs/03-build-notes.md` 를 참조한다.
CI 워크플로 정의는 `ci-templates/` 에 있다.
