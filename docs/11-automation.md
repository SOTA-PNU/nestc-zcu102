# 자동화 구조 (2026-10-30)

이 저장소는 보드에 손으로 들어가지 않고도 모델을 올리고 결과를 받을 수 있게
짜여 있다. 이 문서는 무엇이 저절로 돌고, 무엇이 아직 사람 손을 타며, 고장났을
때 어디를 보는지를 적는다.

## 한 줄 요약

`models/` 에 conf 하나를 올리고 푸시하면, 번들 생성부터 보드 추론과 정답 판정까지
저절로 돌고, 결과가 `results/latest.md` 로 저장소에 다시 커밋된다.

## 기계가 넷이다

헷갈리기 쉬우므로 먼저 정리한다.

**보드** (`xilinx@pynq`, ZCU102). FPGA 와 EVTA 가 있는 곳이다. 추론은 여기서만
일어난다. Ubuntu 18.04, 가용 메모리 1.1GB. Node 20 이 돌지 않아 GitHub Actions
러너를 둘 수 없다.

**러너 PC** (현재 동훈님 WSL). 무거운 일을 한다 — 번들 생성(도커), 컴파일러
빌드, 보드로 SSH. GitHub 가 여기에 작업을 내려보낸다. **이 기계가 꺼져 있으면
model-test 와 multievta 는 돌지 않는다.**

**GitHub 클라우드 러너**. 쓰지 않는다. `board-health` 를 여기로 옮겨 보았으나
보드에 닿지 못했다 — 첫 SSH 에서 종료 코드 255 였다. 보드가
`sota.pusan.ac.kr:25022` 로 공개되어 있는데도 그렇다. 교내 방화벽이 외부 IP 를
막는 것으로 보인다. **보드에 닿으려면 교내망 안의 기계가 필요하다.**

**컨테이너** (`leejaymin/nestc-ssh:latest`). 러너 PC 안에서 뜬다. 컴파일러가
Ubuntu 20.04 에서 빌드되어 러너의 22.04 에서는 `libprotobuf.so.17` 을 찾지 못한다.
그래서 번들 생성은 반드시 이 이미지 안에서 돈다.

## 워크플로 넷

### board-health — 2시간마다, 러너 PC

보드에 SSH 해서 가동 시간, `/dev/xlnk`, 메모리, 디스크, 남은 VTA 프로세스를 본다.
VTA 를 건드리지 않는 가벼운 점검이다. 디스크가 90% 를 넘으면 실패한다.
실패하면 GitHub 가 메일을 보낸다 — "보드가 죽은 것 같은데"를 직접 확인할 일이
없어진다.

### model-test — `models/`, `ci/`, `scripts/` 가 바뀌면

하는 일이 순서대로 이렇다.

상류에 우리 패치를 적용한다(`ci/patch_concat.py`, `ci/patch_tiling.py`). 전부
멱등하므로 매번 돌려도 안전하다. 컴파일러를 증분 빌드한다. `models/` 아래 conf 를
**전부** 돌린다 — 어떤 모델을 돌릴지 고를 필요가 없다. 모델마다 보정 프로파일을
뜨고 번들을 만들어 보드로 보내고, 보드에서 컴파일해 추론하고, conf 의 `EXPECT` 와
대조한다. 결과를 `results/latest.md` 에 쓰고 커밋한다.

conf 에 `KNOWN_FAIL=1` 이 있으면 결과는 기록하되 워크플로 전체를 실패로 만들지
않는다. 아직 통과하지 못하는 모델 때문에 매번 빨개지는 것을 막기 위함이다.

### multievta — `models-multi/` 가 바뀌면

같은 모델을 코어 수만큼 컴파일해 EVTA 코어에 하나씩 묶고 동시에 돌린다.
처리량을 잰다. 지연시간은 줄지 않는다. 결과는 `results/multievta.md`.

### board-recover — 수동 실행만

보드가 이상할 때 쓴다. 점검 / 남은 프로세스 정리 / 재부팅.

VTA 실행이 비정상 종료되면 FPGA 연속 메모리(CMA)가 반납되지 않아 이후 모든
실행이 막힌다. 2026-10-30 에 4코어 시도가 죽으면서 CMA 256MB 중 239MB 가
물렸고, 단일 코어 ResNet-18 까지 크래시했다. 프로세스를 죽여서는 풀리지 않고
재부팅해야 한다.

### debug-dump — 수동 실행만

보드 결과가 틀릴 때 쓴다. 번들을 `-instrument-debug` 로 만들어 층마다 중간
텐서를 떨구고, 실행 순서대로 분포를 훑어 처음 망가지는 층을 짚는다.
결과는 `results/debug-<모델>.md`.

## 새 모델 올리기

`models/<이름>.conf` 하나를 만들어 푸시하면 된다. 기존 conf 를 복사해 고치는 것이
빠르다. 값을 정하는 방법은 `docs/08-resnet18-manual.md` 의 C 경로에 자세히 있다.
요점만 적으면

입력 이름과 모양은 **추측하지 말고 ONNX 에서 읽는다**. 모델마다 다르다.
ResNet-18 은 NHWC `[1,224,224,3]`, SqueezeNet 은 NCHW `[1,3,224,224]` 다.

전처리는 모델이 품고 있는지 먼저 본다. 첫 노드가 바로 Conv 면 정규화가 없는
것이므로 `IMAGE_MODE=0to1` 에 `USE_IMAGENET_NORMALIZATION=1` 이 맞다. 앞에
`elementsub`/`elementdiv` 가 있으면 모델이 품고 있으므로 `0to255` 다. 틀리면
활성값이 폭주해 softmax 가 NaN 이 되거나, 조용히 엉뚱한 답이 나온다.

`OUTPUT_NAME` 은 첫 실행 때 스크립트가 후보를 출력한다. 그걸 보고 채우면 된다.

## 고장났을 때

**작업이 Queued 에서 안 움직인다.** 러너 PC 가 꺼져 있거나 러너 서비스가
내려갔다. WSL 에서 `cd ~/actions-runner && sudo ./svc.sh status`. `Listening for
Jobs` 가 아니면 `sudo ./svc.sh start`. "registration has been deleted" 가 보이면
GitHub 가 오래 끊긴 러너를 지운 것이므로 토큰을 새로 받아 다시 등록한다
(`docs/10-lab-server-setup.md`).

**WSL 이 재부팅 뒤 안 올라온다.** Windows 는 WSL 을 자동으로 띄우지 않는다.
시작 프로그램에 `wsl -d Ubuntu-22.04 -u root /bin/true` 를 넣어 두었다.

**보드가 응답하지 않는다.** VTA 실행 중에 Ctrl+C 를 눌렀거나 워크플로를 취소하면
FPGA 와 xlnk 가 정리되지 않아 재부팅 전까지 보드를 쓸 수 없다. `board-health` 가
남은 프로세스를 보여 준다. **실행 중인 추론은 끝까지 두어야 한다.**

**번들 생성이 라이브러리를 못 찾는다.** 컨테이너 밖에서 `model-compiler` 를
돌렸을 때 나는 증상이다. 반드시 `docker run ... leejaymin/nestc-ssh:latest` 안에서
돌린다.

## 아직 사람이 하는 일

`git push` 하나다. 그리고 새 모델 conf 의 값을 ONNX 를 열어 정하는 일.

가장 약한 고리는 러너가 개인 PC 에 있다는 점이다. 꺼지면 아무것도 돌지 않고,
오래 꺼져 있으면 GitHub 가 러너 등록을 지운다(실제로 한 번 겪었다). 항상 켜진
연구실 서버로 옮기는 순서는 `docs/10-lab-server-setup.md` 에 있다.
