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
# MAIN_CPP 는 선택이다. 없으면 틀에서 생성한다(OUTPUT_NAME 필요).
: "${MAIN_CPP:=__template__}"

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
if [ "$MAIN_CPP" = "__template__" ]; then MAIN=__template__; else MAIN=$(resolve "$MAIN_CPP"); fi
IMG_GLOB=$(resolve "$CALIB_IMAGES")

[ -f "$ONNX" ] || { echo "ONNX 없음: $ONNX" >&2; exit 1; }
[ "$MAIN" = "__template__" ] || [ -f "$MAIN" ] || { echo "Main.cpp 없음: $MAIN" >&2; exit 1; }

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
OPTS=()
[ "$USE_IMAGENET_NORMALIZATION" = "1" ] && OPTS+=(-use-imagenet-normalization)
# CALIB_EXTRA 는 공백으로 나뉜 추가 옵션이다(-compute-softmax -topk=5 등).
# shellcheck disable=SC2206
[ -n "${CALIB_EXTRA:-}" ] && OPTS+=( ${CALIB_EXTRA} )

echo "== 1단계 보정 프로파일"
"$IC" "${IMAGES[@]}" \
  -m="$ONNX" \
  -model-input-name="$INPUT_NAME" \
  -backend=VTAInterpreter \
  -image-layout="$IMAGE_LAYOUT" \
  -image-mode="$IMAGE_MODE" \
  "${OPTS[@]}" \
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

# ── Main.cpp ──────────────────────────────────────────────────────────
# 상류는 번들마다 전용 main 을 두지만, 실제로 모델에 종속된 식별자는
# 접두사 하나에서 파생되는 여섯 군데뿐이다.
#   <접두사>.h  <접두사>_config  <접두사>()  <접두사>_load_module
#   <접두사>_destroy_module  "<접두사>.weights.bin"
# 나머지는 BundleConfig 와 symbolTable 로 실행 시점에 심볼을 찾는 범용 코드다.
# 그래서 ResNet-18 의 main 을 틀로 삼아 접두사만 바꾸면 다른 분류 모델에도 쓸 수 있다.
#
# 접두사에서 파생되지 않는 것이 하나 있다 — 출력 텐서 이름이다.
# conf 의 OUTPUT_NAME 으로 받는다.
NET=$(ls "$OUT"/*.cpp 2>/dev/null | grep -v '/Main\.cpp$' | head -1)
[ -n "$NET" ] || { echo "번들 소스를 찾지 못했다" >&2; exit 1; }
BASE=$(basename "$NET" .cpp)

if [ "$MAIN" != "__template__" ]; then
  # conf 가 전용 main 을 지정했다. 그대로 쓴다.
  cp "$MAIN" "$OUT/Main.cpp"
  echo "   main   $MAIN (그대로)"
else
  : "${OUTPUT_NAME:?전용 main 을 쓰지 않으려면 conf 에 OUTPUT_NAME 이 필요하다}"
  TMPL=$(resolve "${MAIN_TEMPLATE:-upstream:vta/bundles/Resnet18Test/mxnet_exported_resnet18BundleMain.cpp}")
  [ -f "$TMPL" ] || { echo "틀 main 없음: $TMPL" >&2; exit 1; }
  sed -e "s/mxnet_exported_resnet18/$BASE/g" \
      -e "s/resnetv10_dense0_fwd__1/$OUTPUT_NAME/g" \
      "$TMPL" > "$OUT/Main.cpp"
  echo "   main   틀에서 생성 (접두사 $BASE, 출력 $OUTPUT_NAME)"
fi

# OUTPUT_NAME 을 잘못 적으면 추론은 되는데 결과를 못 읽는다.
# 번들이 내보내는 심볼 이름을 보여 주어 확인할 수 있게 한다.
echo "== 번들이 내보내는 심볼 (OUTPUT_NAME 후보)"
grep -oE '"[A-Za-z0-9_]+__[0-9]+"' "$NET" | sort -u | head -20 || true

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
