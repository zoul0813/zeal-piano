#!/usr/bin/env python3

import argparse
from pathlib import Path
import mido

parser = argparse.ArgumentParser("midi2zeal")
parser.add_argument('-o', '--output', help='Output filename', required=True)
parser.add_argument("filename", nargs='+', help="Input Filename")
args = parser.parse_args()
print(args)
# exit(0)


# Open a MIDI file
mid = mido.MidiFile(args.filename[0])

TICKS_PER_QUARTER = mid.ticks_per_beat
TICKS_PER_FRAME = 32 # roughly?
TICK_DIVISOR = TICKS_PER_QUARTER / TICKS_PER_FRAME


def note2freq(note):
  return int(round(440 * 2 ** ((note - 69) / 12)))

print("PPQ", TICKS_PER_QUARTER, "PPF", TICKS_PER_FRAME, "Div", TICK_DIVISOR)
# exit(0)

# print(mid)
print("Length ", mid.length)

# # Print track names
# for i, track in enumerate(mid.tracks):
#     print(f'Track {i}: {track.name}')

# # Iterate over messages in a track
# for msg in mid.tracks[0]:
#     print(msg)

for index, track in enumerate(mid.tracks):
  for msg in track:
    if not msg.is_meta:
      msg.channel = index

tracks = mido.merge_tracks(mid.tracks, skip_checks=False)


frames = 1
ticks = 0
binary = bytearray()
records = 0
allowed_messages = ['note_on', 'note_off']
for message in tracks:
  time = message.time + ticks
  ticks = time
  offset = ticks / TICK_DIVISOR
  frames = int(offset)

  if message.is_meta or message.is_cc():
    # print(message.type, "is_meta", message.is_meta, "is_cc", message.is_cc)
    continue

  if message.type not in allowed_messages:
    # print(message.type, "not in", allowed_messages)
    continue

  print(vars(message))
  # print(dir(message))
  # print(message)

  records += 1
  freq = note2freq(message.note)
  voice = message.channel
  wave = message.channel
  voice_wave = (voice << 4) | wave
  on_off = ' on'
  if message.type == 'note_off':
    on_off = 'off'
    freq = 0

  bin = []
  bin += frames.to_bytes(2, 'little')
  bin += freq.to_bytes(2, 'little')
  bin += voice_wave.to_bytes(1, 'little')

  output = f"{frames:05d} {on_off}({freq:04d}) {voice:02x} {wave:02x} {voice_wave:02x} [{message.note}]"
  print(output)

  binary += bytes(bin)

  if records >= 2048:
    break

# print(binary)
with open(args.output, "wb") as f:
  # header
  records += 1 # end marker
  f.write(records.to_bytes(2, 'little'))

  # records
  f.write(binary)

  # end marker
  frames += 1
  print("end marker", frames, 0xff, 0xff)
  f.write((frames).to_bytes(2, 'little'))
  f.write((0xFF).to_bytes(2, 'little'))
  f.write((0xFF).to_bytes(1, 'little'))

print("Records: ", records)