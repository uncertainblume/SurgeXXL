# SurgeXXL for the MPC plugin SDK

![SurgeXXL, its eight pages](docs/img/pages.png)

**Surge XT, the Surge Synth Team's open-source hybrid synthesizer, as an instrument plugin for Akai MPC standalone
hardware, packaged the official way: for MPC OS 3.10.1's External Plugin Support, built to Akai's
[MPC plugin SDK](https://github.com/inmusic-software/mpc-plugin-builder-sdk).**

*Dedicated to Wendy Carlos, the pioneer without whom none of this would be possible.*

This is a fork of [SurgeXXL](https://github.com/sunskiefer/SurgeXXL) by **L'Cronx**, who did the real work: they
put Surge XT 1.3.4's sound engine on the Akai Force, drew its eight pages, mapped every control to the Q-Links and
wrote the tools and tests around it. Everything that makes this an instrument is theirs. This fork changes how it is
built and packaged, and adds a 64-bit build.

## Why this fork exists

Two reasons.

**To make SurgeXXL SDK compliant.** With MPC OS 3.10.1, Akai opened the MPC to third-party plugins: a
`VST2 Plugins` folder on any drive, a switch in Preferences, and a public SDK that describes the plugin folder
format. The original SurgeXXL predates that. It was made for the Force and installed through the community
[mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins) loader. The goal here was to take the same instrument
and deliver it through the new open route Akai provides: no installer script, no changed system files, one folder
you copy to a drive, and a build for the 64-bit MPCs (MPC Live III, MPC XL) as well as the 32-bit ones.

**To play MPE on the MPC.** Surge ships a set of patches written for the LinnStrument, and Surge's engine has MPE
voice handling built in. I wanted those sounds, played expressively, on an MPC. See [MPE](#mpe) for how far that
has got, which is not all the way yet.

## Status

> [!NOTE]
> **Built and tested offline. Not yet confirmed on a device in this form.** Both libraries pass the original
> project's engine test and the SDK's validator. The SDK's host simulator still reports four selector parameters
> that do not read back after a state restore (see [Testing](#testing)). Third-party plugins have no crash
> protection on the MPC: save your project before you load it, and please report what you find under Issues.

| | |
|---|---|
| Runs on | MPC OS **3.10.1 or later**. 64-bit: MPC Live III, MPC XL. 32-bit: MPC X, X SE, Live, Live II, One, One+, Key 61, Key 37, Force |
| Package | `L'Cronx - VST - SurgeXXL/`, about 450 MB unpacked (Surge's whole patch and wavetable library is inside) |
| Engine | Surge XT 1.3.4, unchanged, with 3008 patches and 761 wavetables |
| Tested on a device | The original 0.2.0 on an Akai Force (firmware 3.9.1, community loader). This SDK package: not yet |

## Install

1. Download `surgexxl-<version>-mpc-sdk.zip` from [Releases](../../releases) and unzip it.
2. Copy the folder `L'Cronx - VST - SurgeXXL` into a folder named exactly `VST2 Plugins` at the root of a USB
   drive, an SD card or the MPC's internal drive.
3. On the MPC: **Preferences > Enhancements**, turn on **Enable Enhancements** and **External Plugin Support**.
4. Restart the MPC. SurgeXXL is in the plugin browser under **External Plugins**.

Over the network instead, with the SDK's own tool (the MPC needs developer mode on; the SDK's
`docs/install-and-test.md` explains it):

```
python3 tools/mpc_remote.py <mpc ip address> add surgexxl-<version>-mpc-sdk.zip
```

Your own patches and wavetables live in a `SurgeXXL` folder at the root of the same drive, beside `VST2 Plugins`.
Updating or removing the plugin never touches it.

## What this fork changed

- **The SDK plugin folder.** `./build.sh --sdk` builds `plugin.json`, `version.xml`, `VST/surgexxl-arm32.so`,
  `VST/surgexxl-arm64.so`, `Plugin Skins/` and Surge's data as `Content/`, as the SDK's
  `docs/plugin-folder-format.md` describes. It passes the SDK's `tools/validate.py`.
- **A 64-bit build.** Surge's engine for aarch64 with NEON, tuned for the Cortex-A55 baseline the SDK names. The
  original built 32-bit ARM only.
- **A crash fixed on 64-bit.** Surge's formula modulators run on LuaJIT. The build never told LuaJIT's code
  generator the byte order, so on aarch64 the interpreter read its own bytecode wrongly and crashed at the first
  branch. The original project's own test found it under an emulator.
- **A crash fixed at unload.** A host that unloaded the plugin left Surge's exit-time clean-up pointing at code that
  was no longer there. The library now stays loaded.
- **The user folder moved** to the root of the drive for the SDK layout, outside the plugin folder that an update
  replaces.
- The crash log names registers on aarch64; `test/run_tests.sh arm64` runs the tests on the 64-bit build.

The Force build through the community loader (`./build.sh` with no option) is still there and unchanged in what it
produces.

## MPE

The honest state of it:

- Surge's MPE voice handling is compiled in, and the plugin passes each note's MIDI channel, pitch bend, channel
  pressure and controllers on to Surge.
- Surge's 63 **LinnStrument MPE** patches are in the browser, under that category.
- **There is no switch to turn Surge's MPE mode on yet.** Surge keeps MPE off until told otherwise, and the pages
  have no control for it. Until that is added, those patches play as ordinary patches: per-note bend, pressure and
  timbre do not reach individual voices.
- Whether the MPC passes per-note channels through to an external plugin has not been tested either.

Adding that switch, and testing it with a LinnStrument, is the next piece of work.

## About this fork, and how it was made

I am not a coder. This port was made with [Claude](https://claude.com/claude-code), Anthropic's AI model,
working in the repository: Claude read the original project and Akai's SDK, wrote the build changes, found and
fixed the two crashes above, and ran the tests. I chose the goal and the direction, and the testing on real hardware falls to me.

I will do my best to work with Claude on changes and updates when they seem necessary. If something is broken, open
an Issue and say which MPC and which MPC OS version you have and what you did; the crash log (below) helps a great
deal. Please be patient, and please do not take problems with this fork to L'Cronx or the Surge Synth Team: they
did not make this package.

## Thanks

- **L'Cronx**, for [SurgeXXL](https://github.com/sunskiefer/SurgeXXL). This fork is a repackaging of their
  instrument.
- **The Surge Synth Team** and every contributor to Surge XT, and Claes Johanson, who wrote Surge and gave it away.
- **Roger Linn**, for the LinnStrument, and for a lifetime of instruments, the MPC among them.
- **Geert Bevin**, for the LinnStrument's firmware and for his work on MPE.
- **Akai Professional and inMusic**, for opening the MPC to third-party plugins and publishing the SDK.
- **sd88me** and the mpc-vst-plugins contributors, whose wrapper and skin builder this plugin is built on.
- **Wendy Carlos**, to whom this is dedicated.

---

# The instrument

What follows is L'Cronx's documentation of SurgeXXL, kept as written apart from the places this fork changed
(folders, building, testing). The Q-Link layout and the tested voice counts were made for the Force's eight knobs.

## What's in it

- **Surge XT's engine, unchanged**: 2 scenes, 3 oscillators each (Classic, Sine, Wavetable, S&H Noise, Audio In, FM3,
  FM2, Window, Modern, String, Twist, Alias), 2 filters per voice in 8 configurations with all of Surge's filter types,
  the waveshaper, Amp and Filter EG, 6 voice and 6 scene LFOs (Step Seq, MSEG, Formula and the rest), the modulation
  matrix, 16 FX slots (every Surge effect but Spring Reverb), the 8 macros; MIDI notes and velocity, pitch bend, channel and poly pressure, mod wheel and the other CCs, sustain.
- **All the patches and wavetables**: Surge's factory and third-party collections, browsed by category.
- **Your own**: SAVE writes the sound as `SurgeXXL 001`, `002`... into `/sdcard/SurgeXXL/Patches/SurgeXXL`;
  wavetables (`.wt` or `.wav`) you copy into `/sdcard/SurgeXXL/Wavetables` appear in the list (RESCAN while the
  plugin is open). Your files come first in both lists.
- **Pictures of the wavetables**: the OSC page draws the selected table (all 761 shipped tables have one).
- **Every page on Q-Links**, 8 knobs per Q-Link page, in an order you can change with the SurgeXXL Q-Link Map.
- **Left out**: Spring Reverb (Surge's own build flags it as not working on 32-bit Linux). The 16 patches that use it
  load with that slot off. Airwindows (Surge's effect with 56 Airwindows algorithms inside) is in: 538 patches use it.
  MTS-ESP microtuning is off (it needs a host-wide library); Scala tunings in patches work.

## The pages

MPC shows eight tabs. Most pages show **the edited scene** (A or B: the SCENE buttons on MOD and PATCH, Surge's own
"Active Scene") and, where there are several, **the selected** oscillator, LFO or FX slot: pick one and every control
shows that one.

| Page | Controls |
|---|---|
| OSC | OSC 1/2/3, TYPE (12), the type's seven parameters (sliders, named for the type), Keytrack, Retrigger, Pitch, Octave; the wavetable: its name, folder and picture, `<` `>` (one table), `FOLDER` (the folder before / after), RESCAN, TABLE (a Q-Link through the list) |
| MIXER | Osc 1-3, Ring 1x2, Ring 2x3, Noise: level (fader, value in the window), MUTE, SOLO, ROUTE (F1 / BOTH / F2); Pre-Filter gain; FM routing and depth, Noise Color, Osc Drift; the scene's Volume, Pan, Width |
| FILTER | Filter configuration (8), waveshaper type and drive; Filter 1 and 2: type, subtype, cutoff, resonance, FEG amount, keytrack; Feedback, Balance, Highpass; Link Resonance, Filter 2 Offset Mode |
| ENV / LFO | Amp EG and Filter EG: A, D, S, R, Digital / Analog, the three shapes; LFO 1-3 and S-LFO 1-3 (Surge's voice LFOs 1-3 and scene LFOs 1-3): shape, rate, phase, amplitude, deform, trigger, unipolar, and its envelope (delay, attack, hold, decay, sustain, release) |
| STEP SEQ | The selected LFO's 16 steps (bipolar), loop start and end. The LFO plays them when its shape is Step Seq |
| MOD | The scene's modulation routings: ROUTING (turn through them; past the last one: NEW), SOURCE, TARGET (LEARN, then touch or turn any control on any page), DEPTH, CLEAR |
| FX | The 16 slots (A and B inserts, sends, globals 1-4); the selected slot's type, its twelve parameters (named for the type), On; the scene's four send levels and the four returns; FX Chain bypass |
| PATCH | The patch browser (`<` `>`, `CAT`, PATCH on a Q-Link), LOAD, SAVE, the loaded patch's name; the 8 macros with the patch's own names; Scene A/B, Scene Mode, Play Mode, Poly Limit, Portamento, Bend Up / Down, Split Point, Volume, PANIC |

The other six voice LFOs and scene LFOs (Surge's 4-6) still run as the patches use them; the pages show 1-3 of each
because the Force's own automation covers the rest. MSEG and Formula LFOs play as the patch made them (they are
not edited here).

## Folders

**SDK package (this fork):** Surge's data is the plugin folder's `Content/`, and your files are in `SurgeXXL` at the
root of the drive that holds `VST2 Plugins` (`Patches/`, `Wavetables/` and the crash log `surgexxl.log`). Where the
text below says `/sdcard/SurgeXXL`, read that folder.

**Force package (community loader):** the plugin finds its data next to its library (`<plugin folder>/data`: Surge's patches, wavetables, presets,
tunings) and keeps your files in a `SurgeXXL` folder beside the `Synths` folder it is installed in:
`/sdcard/SurgeXXL` for `/sdcard/Synths`, `/media/<card>/SurgeXXL` on a card. It makes `Patches/` and
`Wavetables/` there the first time it runs. Reinstalling or updating the plugin never touches it.

## Crash log

`/sdcard/SurgeXXL/surgexxl.log` (next to your Patches and Wavetables) records each instance, each LOAD and project
restore, and, if the plugin brings MPC down, where it happened. Attach it to an Issue.

## The project, presets and state

The whole sound is saved with the MPC project: every parameter, the modulation, the wavetables, the step sequences
and MSEGs (Surge's own patch format), plus what the pages are showing. It is larger than most plugins' state
(50-210 KB of text); see docs/UPSTREAM.md.

## CPU

SurgeXXL runs at MPC's 44.1 kHz with Surge's own 2x oversampled oscillators, on the Force's Cortex-A17 with NEON (the
build targets it). The polyphony limit is 2-32 (Surge's default 16; half of Surge's 64). A patch with many unison
voices costs more per note. The first device test measures it (mpc-vst-plugins `tools/bench.sh`).

## Q-Links

Each page's controls are on its Q-Link pages, 8 knobs each (the Force's eight knobs), in the order of `qlinks.json`.
Change the order with the **SurgeXXL Q-Link Map** (number the controls on a picture of each page, save
`surgexxl-qlinks.json`), then build with it as `qlinks.json`. Triggers (LOAD, SAVE, PANIC, LEARN...) are touch only.

## Building

On x86-64 Linux (the build runs small Linux helper programs, so not on macOS or Windows directly):

```
git clone --recursive https://github.com/uncertainblume/SurgeXXL && cd SurgeXXL
pip install ziglang Pillow cairosvg       # plus cmake, ninja and a host gcc
./build.sh --sdk                          # -> build/sdk/L'Cronx - VST - SurgeXXL/   (MPC OS 3.10.1+, the SDK folder)
./build.sh                                # -> build/package/L'Cronx - VST - SurgeXXL/   (the Force, community loader)
```
No Docker. The **SDK release** workflow (`.github/workflows/sdk-release.yml`) does the `--sdk` build on GitHub and
publishes the zip. `SXXL_WT_PICTURES=0 ./build.sh` builds without the wavetable pictures (a lighter skin).

### For MPC OS 3.10.1+ (External Plugin Support, every MPC)

`./build.sh --sdk` builds the plugin folder of Akai's [MPC plugin SDK](https://github.com/inmusic-software/mpc-plugin-builder-sdk)
instead: `build/sdk/L'Cronx - VST - SurgeXXL/` with `plugin.json`, `VST/surgexxl-arm32.so` (MPC X, Live, One, Key,
Force) and `VST/surgexxl-arm64.so` (MPC Live III, MPC XL), `Plugin Skins/` and Surge's data as `Content/`.
`SDK_ARCHS=arm64 ./build.sh --sdk` builds one of them only. Copy the folder into `VST2 Plugins` at the root of a
drive, or install it with the SDK's `tools/mpc_remote.py <ip> add "build/sdk/L'Cronx - VST - SurgeXXL"`; on the MPC,
Preferences > Enhancements: Enable Enhancements and External Plugin Support. Your files are then in `SurgeXXL` at the
root of that drive (beside `VST2 Plugins`). The 64-bit build is Surge's engine as it is for aarch64 (NEON, tuned for
the Cortex-A55 the SDK names); it is tested offline only (`test/run_tests.sh arm64`, the SDK's `host_sim` under
qemu-aarch64), not yet on a device.
Releases: mpc-vst-plugins' `tools/release.py` (docs/RELEASING.md there), license `GPL-3.0-or-later`.

## Testing

`test/run_tests.sh [x86|arm|arm64|all] [--patches all|N]`:
- `test/engine_test.cpp`: every parameter answers; continuous and option round trips; every Nth patch (or all) loads
  and plays a chord with bounded output, counting the patches that had a removed FX; every FX type plays; the
  wavetables (picture numbers, a user wavetable added and rescanned); SAVE; the state restored on a second instance;
  LEARN. On x86 and as the Force build under `qemu-arm`.
- mpc-vst-plugins' `tools/host_test.c` on the whole plugin (wrapper and engine): names, set/get, option stepping by
  wheel and Q-Link, the long lists' stepping, popups, note to audio, MPC's STOP quirk, the chunk, CC Q-Links, two threads.
- The SDK package: the SDK's `tools/validate.py` on the folder (0 errors; one warning, that `Content/` is over
  256 MB), and its `tools/host-sim` built for ARM and run under qemu on both libraries. `host_sim` passes its load,
  render, block size, parameter sweep and teardown checks and fails its state round trip on four parameters:
  Wavetable, Wavetable Picture, Mod Source List and FX Type List. These are browser and popup selectors, not part of
  the sound, and the saved state itself comes back byte for byte; that they are harmless on a device is not yet
  confirmed.
- The skin: `./build.sh --skin-only`, then mpc-vst-plugins' `tools/studio.py preview` (the pages above are its output).

## Credits and licences

- **SurgeXXL** (the plugin layer, the pages, the tools): L'Cronx, GPL-3.0-or-later
  ([sunskiefer/SurgeXXL](https://github.com/sunskiefer/SurgeXXL)).
- **This fork** (the SDK packaging, the 64-bit build and its fixes): made with Claude, under the same licence,
  GPL-3.0-or-later (`LICENSE`).
- **Surge XT**: the Surge Synth Team and contributors, originally by Claes Johanson (Vember Audio), GPL-3.0-or-later
  (`third_party/surge`, see `src/VENDORED.md`), with its libraries under their own licences: the sst libraries
  (GPL-3.0, MIT), LuaJIT (MIT), SIMDe (MIT), fmt (MIT), PEGTL (MIT), Mutable Instruments' eurorack code (MIT),
  libsamplerate (BSD-2), pffft (BSD-like), tuning-library (MIT), sqlite (public domain) and JUCE 7's core, audio
  basics, audio formats and DSP modules (GPL-3.0). The factory and third-party patches and wavetables are Surge's,
  by their authors, distributed with Surge under its licence.
- **Artwork**: Valley Audio's sliders and Rogan knobs (ValleyRackFree, GPL-3.0-or-later; the Rogan knobs are Valley's
  recolouring of VCV's Component Library, CC BY-NC 4.0), VCV's CKSS switch (CC BY-NC 4.0), as in InterzoneXXL;
  LXRXXL BAZZLINE's fader box, cap and lit key by Suns Kiefer; Titillium Web (SIL OFL 1.1).
  Because of the CC BY-NC 4.0 graphics, SurgeXXL is free and must not be sold.
- **mpc-vst-plugins** (the VST2 wrapper, the skin builder): sd88me and contributors.

SurgeXXL is not affiliated with or endorsed by the Surge Synth Team, Akai Professional or inMusic. This fork is not
affiliated with or endorsed by L'Cronx, Roger Linn Design, or anyone thanked above.
