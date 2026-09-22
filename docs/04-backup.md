# SD카드 백업

## SD카드 구조

128 GB 카드에 64 GB만 파티션되어 있다.

| 파티션 | 크기 | 타입 | 드라이브 | 내용 |
|--------|------|------|----------|------|
| 1 | 100 MiB | FAT32 | Windows에서 보임 | BOOT.BIN, image.ub |
| 2 | 59.53 GiB | ext4 | **Windows에서 안 보임** | rootfs (15 GB 사용) |

**정작 중요한 nest-compiler는 ext4에 있다.** Windows 탐색기로는 접근할 수 없어
파일 복사 방식으로는 부트 파일 2개밖에 건지지 못한다.

## WSL --mount는 동작하지 않는다

USB 카드리더기는 이동식 미디어로 보고되는데, `wsl --mount` 는 내부적으로
Hyper-V 디스크 패스스루를 쓰므로 이를 지원하지 않는다.

```
wsl --mount \\.\PHYSICALDRIVE4 --partition 2 --type ext4 --options ro
→ 시스템이 지정된 드라이브를 찾을 수 없습니다. (0x8007000f)
```

`--bare` 도 같은 결과다. 이 경로는 포기하는 편이 빠르다.

### 디스크 번호 확인은 반드시

`wsl --mount` 에 잘못된 번호를 넣으면 시스템 SSD를 건드릴 수 있다.
반드시 파티션 구성으로 확인한다.

```powershell
Get-Disk | Select-Object Number, FriendlyName, Size, BusType, OperationalStatus
Get-Partition | Select-Object DiskNumber, PartitionNumber, DriveLetter, Size, Type
```

SD카드는 100 MiB FAT32 파티션과 59.5 GiB `Unknown` 파티션을 가진 USB 디스크다.
`Unknown` 은 Windows가 ext4를 인식하지 못해서 나오는 표시다.

## 채택한 방법 — 보드에서 tar로 묶어 네트워크 전송

카드를 뽑지 않고, 보드를 부팅한 상태에서 작업한다.

### 1. 부트 파티션

보드에서 직접 마운트해 확인할 수 있다.

```bash
sudo mkdir -p /mnt/bootpart
sudo mount -o ro /dev/mmcblk0p1 /mnt/bootpart
ls -la /mnt/bootpart
sudo umount /mnt/bootpart
```

실제 파일은 `BOOT.BIN` 과 `image.ub` 둘뿐이다. `.fseventsd`,
`.Spotlight-V100`, `System Volume Information` 은 macOS/Windows가 만든
메타데이터라 백업 대상이 아니다.

### 2. /home/xilinx

```bash
sudo tar cpf /home/backup-xilinx.tar -C /home xilinx
```

`p` 는 권한·소유자 보존이다. gzip(`z`)은 쓰지 않는다. ARM A53에서
압축하면 오래 걸리는데, 어차피 `rsync -z` 가 전송 중 압축한다.

tar 파일을 `/home/` 에 두고 `/home/xilinx` 를 묶으므로 자기 자신을
삼키는 문제는 없다.

### 3. 나머지 rootfs

```bash
sudo tar cpf /home/rootfs-rest.tar \
  --exclude=/proc --exclude=/sys --exclude=/dev --exclude=/run \
  --exclude=/tmp --exclude=/mnt --exclude=/media \
  --exclude=/var/swap \
  --exclude=/home \
  /
```

제외 항목의 근거:

| 경로 | 이유 |
|------|------|
| `/proc` `/sys` `/dev` | 커널이 메모리에 만드는 가상 파일시스템 |
| `/run` `/tmp` | 재부팅하면 사라지는 임시 파일 |
| `/mnt` `/media` | 마운트 지점 |
| `/var/swap` | 1 GB 스왑 파일 |
| `/home` | 2번에서 별도로 백업 |

`--exclude=/home` 을 쓰기 전에 `/home` 아래에 `xilinx` 외의 사용자가
없는지 확인해야 한다. 있으면 어디에도 백업되지 않는다.

```bash
ls -la /home
```

### 4. 전송

```bash
mkdir -p /mnt/d/sd-backup
rsync -avzP xilinx@<IP>:/home/backup-xilinx.tar /mnt/d/sd-backup/
rsync -avzP xilinx@<IP>:/home/rootfs-rest.tar   /mnt/d/sd-backup/
```

`-P` 로 중간에 끊겨도 같은 명령으로 이어받는다.

전송 완료 시 `received` 바이트가 파일 크기보다 작게 나오는데,
`-z` 압축 때문이며 디스크에는 원본 크기로 저장된다.

## 검증

크기 일치만으로는 부족하다. 체크섬을 대조한다.

```bash
# 보드
md5sum /home/backup-xilinx.tar /home/rootfs-rest.tar

# PC
md5sum /mnt/d/sd-backup/*.tar
```

아카이브 자체가 온전한지도 확인한다. 압축을 풀지 않고 전체를 읽어본다.

```bash
tar tf backup-xilinx.tar > /dev/null && echo OK
```

## 정합성 확인

숫자가 맞는지 대조해두면 누락을 잡을 수 있다.

```
원래 사용량          15 GB   (df -h /)
  − 스왑 파일         1 GB   (제외 항목)
  = 실제 데이터      14 GB

home-xilinx.tar    6.58 GB
system-rest.tar    7.20 GB
  합계            13.78 GB
```

차이 0.2 GB는 `/tmp`, 소켓, `du` 의 블록 단위 올림으로 설명된다.

## 복원

1. 새 SD카드에 PYNQ 이미지를 굽는다
2. FAT32 파티션에 `BOOT.BIN`, `image.ub` 를 덮어쓴다
3. ext4 파티션에서 tar를 푼다

```bash
sudo tar xpf home-xilinx.tar -C /home
sudo tar xpf system-rest.tar -C /
```

**파티션 테이블과 raw 섹터는 백업에 포함되지 않는다.** 따라서 카드를
통째로 복제하는 것은 불가능하고 파일 단위 복원만 된다.

부팅 가능한 완전 복제가 필요하면 Win32DiskImager로 카드 전체 이미지를
따로 떠야 한다(128 GB).

## 보관 위치

```
SOTA\nestc\backups\zcu102-pynq-backup-YYYYMMDD\
├── boot\
│   ├── BOOT.BIN
│   └── image.ub
└── rootfs\
    ├── home-xilinx.tar
    └── system-rest.tar
```

**저장소에는 올리지 않는다.** 용량 제한을 넘고, `system-rest.tar` 에는
SSH 호스트 키와 `/etc/shadow` 등 인증정보가 들어 있다.
`.gitignore` 에서 `backups/` 를 제외하고 있다.
