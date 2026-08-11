# IANIBOY

**Sonic Architecture // Multiband Character Engine**

A multiband, oversampled saturation/distortion plugin (VST3 + AU) built on JUCE.
One box for vocals, bass, drums, synths, FX, or the master bus — with the
distortion *type* and *degree* fitted to each source through presets. Built
directly from the *Sonic Architecture Blueprint* and the serial-stage /
asymmetric-waveshaping recipes from the reference notes.

---

## Signal flow

```
 INPUT TRIM
   │
   ├─────────────────────────── DRY (delay-compensated to OS latency) ──┐
   │                                                                     │
   ▼   ── oversampled, anti-aliased nonlinear domain ──                  │
 3-BAND LINKWITZ–RILEY SPLIT (low band phase-aligned via allpass)        │
   ├── LOWS   : 20 Hz brickwall → strict MONO → clean / soft / tube      │
   ├── MIDS   : accent pre-EQ → waveshaper → dynamic "Tame" de-harsh     │
   └── HIGHS  : tape air / clean clip → high-shelf bite                  │
   │                                                                     │
   ▼   ── downsample ──                                                  │
 WET  ───────────────────────────(+ parallel DRY inject 0–50%)──────────┘
   │
   ▼
 OUTPUT CLIPPER (loudness density)  →  OUTPUT TRIM
```

Everything nonlinear runs **oversampled** (1×/2×/4×/8×) so aliasing never
reaches your mix. The parallel dry path is pushed through a delay line matched
to the oversampler's latency, so the raw transient locks in phase with the
saturated body — no comb filtering, exactly as the blueprint's
*Phase-Alignment Calibration* step demands.

## Distortion characters (Mid band)

| Type       | Character                         | Good for                    |
|------------|-----------------------------------|-----------------------------|
| Tape       | symmetric soft clip (tanh)        | glue, master, gentle warmth |
| Tube       | asymmetric, even harmonics        | vocals, bass                |
| Hard Clip  | brickwall                         | drum transients, loudness   |
| Foldback   | wavefolder                        | synth screeches / stabs     |
| Diode      | asymmetric rectifier              | industrial grit             |
| Sine Fold  | sinusoidal fold                   | metallic FM-style edge      |
| Bitcrush   | bit + sample-rate reduction       | digital sizzle, lo-fi       |

## Factory presets

Init (Transparent) · Vocals – Parallel Heat · Bass – Anchor & Growl ·
Kick – Rumble Engine · Drums – Bus Crush · Synth – Screech Fold ·
Synth – Digital Sizzle · FX – Industrial Texture · Master – Density Glue

## The mascot

The panel on the right is **Ianiboy** himself (vector-drawn, zero image
assets). Sweep the **Dry Inject** control up and he crouches, then leaps —
arms up, grin wide open, sparkles at full tilt — and he hops on the output
transients. Pure *Eternal Happiness* energy. Turn it down and he settles.

---

## Build it in the cloud (no local toolchain)

The fastest way to get finished binaries: this repo ships a GitHub Actions
workflow (`.github/workflows/build.yml`) that compiles **Windows VST3,
macOS VST3 + AU, and Linux VST3** on GitHub's free runners.

1. Create a new GitHub repo and push this folder to it.
2. Open the **Actions** tab — the build runs automatically (or click
   *Run workflow*).
3. When it finishes, download the `Ianiboy-Windows` / `Ianiboy-macOS` /
   `Ianiboy-Linux` artifacts. Those are your plugins.

No CMake, compiler, or JUCE install required on your side.

## Building locally

Prerequisites: **CMake ≥ 3.22**, a C++17 compiler, and Git. JUCE 7.0.12 is
fetched automatically — no manual JUCE install needed.

```bash
cd Ianiboy
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The build produces VST3 (all platforms), AU (macOS), and a Standalone app.
`COPY_PLUGIN_AFTER_BUILD` installs them to your user plugin folders.

### Windows (VST3)
- Install Visual Studio 2022 (Desktop C++). Then:
  ```
  cmake -B build -G "Visual Studio 17 2022" -A x64
  cmake --build build --config Release
  ```
- Output: `build/Ianiboy_artefacts/Release/VST3/Ianiboy.vst3`
- Copy to `C:\Program Files\Common Files\VST3\` (auto-copied by default).

### macOS (VST3 + AU)
```
cmake -B build -G Xcode
cmake --build build --config Release
```
- Universal build: add `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`.
- AU lands in `~/Library/Audio/Plug-Ins/Components/`, VST3 in `.../VST3/`.
- For distribution you'll want to codesign + notarize.

### Linux (VST3)
Install dev packages first:
```
sudo apt install build-essential cmake git pkg-config \
  libasound2-dev libx11-dev libxext-dev libxinerama-dev libxrandr-dev \
  libxcursor-dev libfreetype6-dev libgl1-mesa-dev libcurl4-openssl-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```
- Output: `build/Ianiboy_artefacts/Release/VST3/Ianiboy.vst3` → copy to `~/.vst3/`.

## Loading in Ableton Live
Live reads **VST3** on Windows/Mac (and AU on Mac). After building, point
Live's plugin folder at the VST3 location, rescan, and Ianiboy appears under
Audio Effects. It reports latency to the host, so delay compensation keeps
everything in time.

## Notes / roadmap
- Changing **Oversampling** re-initialises the engine (a brief allocation);
  fine as a user action, avoid automating it.
- The **Tame** control is a lightweight dynamic de-harsher (level-tracked
  band subtraction), not a full spectral processor — tune `Tame Freq` to the
  harsh spot.
- Ideas for v2: per-band solo/mute, spectrum display, A/B compare, oversampled
  metering, and a photo-based sprite sheet for the mascot if you want a hand-
  drawn look instead of the vector chibi.

## Project layout
```
Ianiboy/
  CMakeLists.txt          FetchContent JUCE, VST3/AU/Standalone targets
  Source/
    PluginProcessor.*     APVTS params, latency, presets-as-programs
    PluginEditor.*        UI, layout, control attachments
    Parameters.h          parameter IDs, ranges, choice lists
    Presets.h             per-source factory presets
    LookAndFeel.h         Safety-Orange-on-black theme
    Mascot.h              animated vector chibi
    dsp/
      Waveshapers.h       stateless transfer functions + bitcrusher
      MultibandEngine.h   crossover, oversampling, dry-inject, clipper
```
