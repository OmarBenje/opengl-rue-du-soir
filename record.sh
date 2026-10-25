#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
command -v ffmpeg >/dev/null || { echo "Installer FFmpeg pour exporter la video." >&2; exit 1; }
bash build.sh
mkdir -p captures
output="${1:-captures/virtual-visit.mp4}"
./build/dusk-street --record - --fps 30 --duration 60 --width 1280 --height 720 --hidden --no-hud |
  ffmpeg -hide_banner -loglevel warning -y -f rawvideo -pixel_format rgb24 \
    -video_size 1280x720 -framerate 30 -i - -an -c:v libx264 -preset fast \
    -crf 18 -pix_fmt yuv420p -movflags +faststart "$output"
echo "Video enregistree : $output"
