#!/bin/bash

set -e

shopt -s nullglob

# Defaults
START_NUM=1
OUT_DIR="11"
MAX_NUM=255
DONE_DIR="done"

# Parse options
while getopts ":n:f:" opt; do
  case $opt in
    n) START_NUM="$OPTARG" ;;
    f) OUT_DIR="$OPTARG" ;;
    *)
      echo "Usage: $0 [-n start_number] [-f output_folder]"
      exit 1
      ;;
  esac
done

# Validate start number
if ! [[ "$START_NUM" =~ ^[0-9]+$ ]] || (( START_NUM < 1 || START_NUM > MAX_NUM )); then
  echo "Error: start number must be between 1 and $MAX_NUM"
  exit 1
fi

mkdir -p "$DONE_DIR"

# Function: get next available counter in folder
get_next_counter() {
  local dir="$1"
  local last

  last=$(ls "$dir"/*.mp3 2>/dev/null \
    | sed -E 's/.*\/([0-9]{3})\.mp3/\1/' \
    | sort -n \
    | tail -1)

  if [[ -n "$last" ]]; then
    echo $((10#$last + 1))
  else
    echo "$START_NUM"
  fi
}

# Initialize output folder and counter
while :; do
  mkdir -p "$OUT_DIR"
  COUNTER=$(get_next_counter "$OUT_DIR")

  if (( COUNTER <= MAX_NUM )); then
    break
  fi

  OUT_DIR=$((OUT_DIR + 1))
done

# File types to process
INPUT_FILES=(
  *.wav *.ogg *.flac *.aiff *.aif *.mp3
  *.mp4 *.mkv *.avi *.mov *.webm *.m4a
)

# Process files
for file in "${INPUT_FILES[@]}"; do
  [[ -d "$file" ]] && continue

  # Rollover if needed
  if (( COUNTER > MAX_NUM )); then
    OUT_DIR=$((OUT_DIR + 1))
    mkdir -p "$OUT_DIR"
    COUNTER=$(get_next_counter "$OUT_DIR")
  fi

  printf -v NUM "%03d" "$COUNTER"

  echo "Converting '$file' -> $OUT_DIR/$NUM.mp3"

  ffmpeg -y -i "$file" -vn \
    -acodec libmp3lame -q:a 0 \
    -map_metadata 0 -write_xing 1 \
    "$OUT_DIR/$NUM.mp3"

  mv "$file" "$DONE_DIR/"

  ((COUNTER++))
done

echo "All files processed ✔"
