#!/usr/bin/env python3

import mido

# Open a MIDI file
mid = mido.MidiFile('./tools/sample.mid')

TICKS_PER_QUARTER = 96
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
binary = bytearray()
records = 0
for message in tracks:
  # print(vars(message))
  # print(dir(message))
  # print(message)
  if message.is_meta:
    continue

  records += 1
  offset = message.time / TICK_DIVISOR
  frames += int(offset)
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

  output = f"{frames:05d} {on_off}({freq:04d}) {voice:02x} {wave:02x} {voice_wave:02x}"
  print(output)

  binary += bytes(bin)

# print(binary)
with open("sample.ptz", "wb") as f:
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
