#!/usr/bin/env python3
"""Film a real clean build's timed output, then append the OpenGL visit.

The first segment is a rendered build log, not a desktop screen recording.
Requires Pillow and FFmpeg. Run record.sh first for the 60-second scene.
"""
from pathlib import Path
import json
import subprocess
import time
import textwrap
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "captures"
OUT.mkdir(exist_ok=True)
VISIT = OUT / "virtual-visit.mp4"
if not VISIT.exists():
    raise SystemExit("Run bash record.sh first.")

fonts = [Path("/System/Library/Fonts/Menlo.ttc"),
         Path("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf")]
font_path = next((p for p in fonts if p.exists()), None)
def font(size):
    return ImageFont.truetype(str(font_path), size) if font_path else ImageFont.load_default()

events = []
start = time.monotonic()
commands = [
    ["cmake", "-S", ".", "-B", "build/video-proof", "-DCMAKE_BUILD_TYPE=Release"],
    ["cmake", "--build", "build/video-proof", "--clean-first", "--parallel", "4"],
]
for command in commands:
    events.append({"time": time.monotonic() - start, "text": "$ " + " ".join(command)})
    with subprocess.Popen(command, cwd=ROOT, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, text=True, bufsize=1) as process:
        for line in process.stdout:
            events.append({"time": time.monotonic() - start, "text": line.rstrip()})
        if process.wait():
            raise SystemExit("Compilation failed. No success video was produced.")
elapsed = time.monotonic() - start
events.append({"time": elapsed, "text": "Compilation terminee avec succes (code de sortie 0)."})
(OUT / "compilation-events.json").write_text(json.dumps(events, indent=2), encoding="utf-8")
(OUT / "compilation.log").write_text("\n".join(e["text"] for e in events)+"\n", encoding="utf-8")

duration = max(10, elapsed + 4)
video = OUT / "compilation.mp4"
command = ["ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-f", "rawvideo",
           "-pix_fmt", "rgb24", "-s", "1280x720", "-r", "30", "-i", "-",
           "-an", "-c:v", "libx264", "-preset", "fast", "-crf", "18", "-pix_fmt", "yuv420p", str(video)]
encoder = subprocess.Popen(command, stdin=subprocess.PIPE)
for frame in range(int(duration * 30)):
    t = frame / 30
    image = Image.new("RGB", (1280,720), (15,20,30))
    d = ImageDraw.Draw(image)
    d.text((48,32), "RUE DU SOIR", font=font(37), fill=(245,222,181))
    d.text((48,87), "01 / COMPILATION DU PROGRAMME C++", font=font(18), fill=(170,181,200))
    d.rounded_rectangle((40,132,1240,640), radius=12, fill=(23,30,42))
    lines=[]
    for event in events:
        if event["time"] <= t:
            lines.extend(textwrap.wrap(event["text"], width=105) or [""])
    for i,line in enumerate(lines[-19:]):
        color=(143,215,179) if "succes" in line or "Built target" in line else (210,218,230)
        d.text((58,148+i*24),line,font=font(17),fill=color)
    d.text((48,670), "Sortie reelle de CMake et du compilateur, enregistree pendant une reconstruction.",
           font=font(15),fill=(141,153,175))
    encoder.stdin.write(image.tobytes())
encoder.stdin.close()
if encoder.wait():
    raise SystemExit("FFmpeg compilation segment failed")
playlist = OUT / "segments.txt"
playlist.write_text("file 'compilation.mp4'\nfile 'virtual-visit.mp4'\n")
subprocess.run(["ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-f", "concat", "-safe", "0",
                "-i", str(playlist), "-c", "copy", "-movflags", "+faststart", str(OUT/"homework-video.mp4")], check=True)
print(f"Created {OUT/'homework-video.mp4'} (build + complete visit)")
