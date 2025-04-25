# API

## PlayNote

- `playNote(int note, float velocity, float duration)`
- PUB/SUB
- Client → PYNQ
- `note.play?>[frequency] [velocity] [duration]`
- `note.play?>52 127 0.5`

---

## PlayChord

- `playChord(std::string chordName, float velocity, float duration)`
- PUB/SUB
- Client → PYNQ
- `chord.play?>[chordName] [velocity] [duration]`
- `chord.play?>Cmaj7 0.6 1.0`

---

## SetInstrument

- `setInstrument(std::string instrument)`
- PUB/SUB
- Client → PYNQ
- `instrument.set?>[instrumentName]`
- `instrument.set?>sine`

---

## SetEnvelope

- `setEnvelope(float attack, float decay, float sustain, float release)`
- PUB/SUB
- Client → PYNQ
- `envelope.set?>[attack] [decay] [sustain] [release]`
- `envelope.set?>0.01 0.1 0.8 0.2`

---

## SetVolume

- `setVolume(float volume)`
- PUB/SUB
- Client → PYNQ
- `volume.set?>[volume]`
- `volume.set?>0.7`

---

## GetSettings

- `getSetting(std::string name) → std::string`
- REQ/REP
- Client ↔ PYNQ
- **ZMQ Format**:
  - Request: `settings.get?>[settingName]`
  - Reply: `settings.reply!>[value(s)]`
- **Example**:
  - Request: `settings.get?>envelope`
  - Response: `settings.reply!>0.01 0.1 0.8 0.2`

---

## GetStatus

- `getStatus() → std::string`
- REQ/REP
- Client ↔ PYNQ
- **ZMQ Format**:
  - Request: `status.get?>`
  - Reply: `status.reply!>[status info]`
- **Example**:
  - Request: `status.get?>`
  - Response: `status.reply!>alive poly=4 cpu=23%`