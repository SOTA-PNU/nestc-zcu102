#!/usr/bin/env bash
# 모델 기술서(models/*.conf)를 읽어 VTA 번들을 생성한다.
#
# PC(또는 SDK 컨테이너)에서 실행한다. model-compiler 와 image-classifier 가
# 필요하며, 둘 다 상류를 빌드해야 생긴다. 도커 이미지에는 들어 있지 않다.
#
#   scripts/gen-bundle.sh \
#     --conf models/resnet18.conf \
#     --tools ~/nestc-upstream/build/glow/bin \
#     --upstream ~/nestc-upstream \
#     --out /tmp/bundle-resnet18
#
# 산출물: <out>/ 에 <model>.cpp, <model>.h, <model>.weights.bin,
#         VTARuntime.h, Main.cpp, bundle.conf
set -euo pipefail

CONF=; TOOLS=; UPSTREAM=; OUT=
while [ $# -gt 0 ]; do
  case "$1" in
    --conf)     CONF=$2; shift 2 ;;
    --tools)    TOOLS=$2; shift 2 ;;
    --upstream) UPSTREAM=$2; shift 2 ;;
    --out)      OUT=$2; shift 2 ;;
    *) echo "모르는 인자: $1" >&2; exit 2 ;;
  esac
done
[ -n "$CONF" ] && [ -n "$TOOLS" ] && [ -n "$UPSTREAM" ] && [ -n "$OUT" ] || {
  sed -n '2,14p' "$0" >&2; exit 2; }

REPO=$(cd "$(dirname "$0")/.." && pwd)
UPSTREAM=$(cd "$UPSTREAM" && pwd)
TOOLS=$(cd "$TOOLS" && pwd)
mkdir -p "$OUT"; OUT=$(cd "$OUT" && pwd)

# shellcheck disable=SC1090
. "$CONF"

: "${MODEL_NAME:?conf 에 MODEL_NAME 이 없다}"
: "${MODEL_ONNX:?conf 에 MODEL_ONNX 가 없다}"
: "${INPUT_NAME:?conf 에 INPUT_NAME 이 없다}"
: "${INPUT_SHAPE:?conf 에 INPUT_SHAPE 가 없다}"
: "${IMAGE_LAYOUT:=NCHW}"
: "${IMAGE_MODE:=0to1}"
: "${USE_IMAGENET_NORMALIZATION:=1}"
: "${CALIB_IMAGES:?conf 에 CALIB_IMAGES 가 없다}"
: "${MAIN_CPP:?conf 에 MAIN_CPP 가 없다}"

# upstream: / repo: / http(s): 접두사를 실제 경로로 바꾼다.
resolve() {
  case "$1" in
    upstream:*) echo "$UPSTREAM/${1#upstream:}" ;;
    repo:*)     echo "$REPO/${1#repo:}" ;;
    http://*|https://*)
      local dst="$OUT/$(basename "${1%%\?*}")"
      curl -fL "$1" -o "$dst" >&2
      echo "$dst" ;;
    *) echo "$1" ;;
  esac
}

ONNX=$(resolve "$MODEL_ONNX")
MAIN=$(resolve "$MAIN_CPP")
IMG_GLOB=$(resolve "$CALIB_IMAGES")

[ -f "$ONNX" ] || { echo "ONNX 없음: $ONNX" >&2; exit 1; }
[ -f "$MAIN" ] || { echo "Main.cpp 없음: $MAIN" >&2; exit 1; }

# 글롭을 펼친다. 따옴표를 쓰면 안 되는 유일한 자리다.
# shellcheck disable=SC2206
IMAGES=( $IMG_GLOB )
[ ${#IMAGES[@]} -gt 0 ] || { echo "보정 이미지 없음: $IMG_GLOB" >&2; exit 1; }

MC="$TOOLS/model-compiler"
IC="$TOOLS/image-classifier"
for t in "$MC" "$IC"; do
  [ -x "$t" ] || { echo "실행 파일 없음: $t
상류를 먼저 빌드해야 한다. docs/03-build-notes.md 참조." >&2; exit 1; }
done

echo "== 모델 $MODEL_NAME"
echo "   ONNX   $ONNX"
echo "   이미지 ${#IMAGES[@]}장"
echo "   출력   $OUT"

# ── 1단계 보정 프로파일 ────────────────────────────────────────────────
# 레이어별 활성값 분포를 수집한다. VTAInterpreter 는 FPGA 없이
# VTA 동작을 흉내 내는 백엔드라 PC 에서 돌아간다.
NORM_OPT=()
[ "$USE_IMAGENET_NORMALIZATION" = "1" ] && NORM_OPT=(-use-imagenet-normalization)

echo "== 1단계 보정 프로파일"
"$IC" "${IMAGES[@]}" \
  -m="$ONNX" \
  -model-input-name="$INPUT_NAME" \
  -backend=VTAInterpreter \
  -image-layout="$IMAGE_LAYOUT" \
  -image-mode="$IMAGE_MODE" \
  "${NORM_OPT[@]}" \
  -dump-profile="$OUT/calib.yaml" \
  -quantization-schema=symmetric_with_power2_scale

[ -s "$OUT/calib.yaml" ] || { echo "calib.yaml 이 비었다" >&2; exit 1; }
echo "   calib.yaml $(wc -l < "$OUT/calib.yaml") 줄"

# ── 2단계 번들 생성 ───────────────────────────────────────────────────
# 양자화 스케일이 이 시점에 코드로 박힌다. 실행 시점에 바꿀 수 없다.
echo "== 2단계 번들 생성"
"$MC" -g \
  -model="$ONNX" \
  -backend=VTA \
  -emit-bundle="$OUT" \
  -bundle-api=dynamic \
  -model-input="$INPUT_NAME,float,$INPUT_SHAPE" \
  -load-profile="$OUT/calib.yaml" \
  -quantization-schema=symmetric_with_power2_scale

cp "$MAIN" "$OUT/Main.cpp"

# 보드 쪽 스크립트가 읽을 요약본. conf 전체를 넘기지 않기 위함이다.
cat > "$OUT/bundle.conf" <<EOF
MODEL_NAME=$MODEL_NAME
EXPECT="${EXPECT:-}"
EOF

echo "== 완료"
ls -la "$OUT"

# weights.bin 이 없으면 번들 생성이 조용히 실패한 것이다.
ls "$OUT"/*.weights.bin >/dev/null 2>&1 || {
  echo "weights.bin 이 없다. 번들 생성이 실패했다." >&2; exit 1; }
