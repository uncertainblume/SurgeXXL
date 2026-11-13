# SurgeXXL for the MPC plugin SDK

![SurgeXXL, its eight pages](docs/img/pages.png)

**Surge XT, the Surge Synth Team's open-source hybrid synthesizer, as an instrument plugin for Akai MPC standalone
hardware, packaged the official way: for MPC OS 3.10.1's External Plugin Support, built to Akai's
[MPC plugin SDK](https://github.com/inmusic-software/mpc-plugin-builder-sdk).**

*Dedicated to Roger Linn and Geert Bevin, creators of MPE and the Linnstrument, and to Wendy Carlos, the pioneer without
whom none of this would be possible.*

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
voice handling built in. Those patches are wonderful in Surge on a computer, and I wanted them, played with
per-note slides and pressure, on an MPC, perhaps as my main LinnStrument synth. Version 0.3.0 adds what that needs;
see [MPE](#mpe) for what is there and what is still unproven.

## Status

> [!WARNING]
> **Experimental. Not fully tested, and it may have major usability problems.** Treat this as an early test build,
> not a finished instrument.
>
> - It has had very little use on real hardware: one MPC Live III, briefly. Nearly all the checks behind it ran
>   offline, under an emulator.
> - Third-party plugins have no crash protection on the MPC. A fault in the plugin can take the MPC down and lose
>   unsaved work. **Save your project before you load it**, and do not use it in a session you cannot afford to lose.
> - The pages and Q-Link layout were designed for the Akai Force. How well they work on other MPCs is not known.
> - Some things are awkward or missing; see [Known problems](#known-problems).
> - **No warranty, no liability.** See [Disclaimer](#disclaimer).

Both libraries pass the original project's engine test and the SDK's validator. The SDK's host simulator still
reports a few browser and popup selector parameters that do not read back exactly (see [Testing](#testing)). Please
report what you find under Issues.

| | |
|---|---|
| Runs on | MPC OS **3.10.1 or later**. 64-bit: MPC Live III, MPC XL. 32-bit: MPC X, X SE, Live, Live II, One, One+, Key 61, Key 37, Force |
| Package | `L'Cronx - VST - SurgeXXL/`, about 450 MB unpacked (Surge's whole patch and wavetable library is inside) |
| Engine | Surge XT 1.3.4, unchanged, with 3008 patches and 761 wavetables |
| Tested on a device | The original 0.2.0 on an Akai Force (firmware 3.9.1, community loader). This SDK package: 0.3.0 loaded and played with MPE on one MPC Live III; little else, see the warning above |

## Known problems

- **Browsing patches is slow.** Only the 63 LinnStrument MPE patches are in the MPC's own preset list (since
  0.3.1; see [Presets](#presets)). For the other 2945 the only way is the PATCH page (the last tab): `<` and `>`
  step one patch at a time, `CAT` jumps to the next or previous category, and the PATCH Q-Link turns through the
  list; then press `LOAD`. There is no list view and no search.
- **MPE has had one first test on hardware**, and it worked there: per-note bends and polyphonic aftertouch came
  through on an MPC Live III with a LinnStrument. Audio glitches were heard, and nothing else about it has been
  checked on a device. See [MPE](#mpe).
- **Tested on almost no hardware.** Voice counts and CPU load on each MPC model are unmeasured.
- **A large package.** About 450 MB on the drive, and it can be slow to copy and to scan.
- **A few selector parameters** are flagged by the SDK's host simulator (see [Testing](#testing)).

If you hit something else, it is probably not you. Open an Issue.

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

Three versions so far. **0.2.0** (`sdk-v0.2.0`) is the original instrument, repackaged for the SDK with a 64-bit
build and two crash fixes, and nothing else. **0.3.0** adds MPE. **0.3.1** puts the LinnStrument MPE patches in the
MPC's preset list.

### 0.3.1: the LinnStrument MPE patches as MPC presets

1. The plugin now offers **76 programs** to the MPC, which lists them in its own preset selector: a first entry for
   the PATCH page's own sound, then Surge's 12 LinnStrument MPE folders, each as a heading (`== Basses ==`) followed
   by its patches, 63 in all.
2. Picking a patch there loads it, exactly as LOAD on the PATCH page does. The headings and the first entry do
   nothing.
3. The chosen entry is saved in the project with the sound, so reopening a project does not reload the factory
   patch over your edits.
4. One parameter was added at the end of the list (`preset`); projects made with 0.2.0 and 0.3.0 still load.
5. Tests: the list's length and order, a heading doing nothing, the first and last patches loading and playing, the
   entry kept in the project state, and the wrapper's own program checks.

Details: [Presets](#presets). Not yet tried on an MPC.

### 0.3.0: MPE

1. An **MPE switch** and an **MPE Bend Range** control on the PATCH page, on Q-Link page PATCH 3, saved in the
   project. Three parameters were added at the end of the list, so projects made with 0.2.0 still load.
2. **The plugin's own MIDI port**, "SurgeXXL MIDI In", so that each note keeps its channel. The technique and the
   port code come from [mpc-mpe](https://github.com/uncertainblume/mpc-mpe).
3. A **status line** that shows the port and the channels arriving by each route.
4. The plugin answers yes to the MPC's "MPE" capability question.
5. **Tests**: per-note bend at two bend ranges, up and down; a bend on another channel leaving the note alone;
   the global channel; per-note pressure; polyphonic aftertouch; per-note timbre; MPE off; the setting kept across a
   patch load and restored from the project state. All pass on the 64-bit build under an emulator.

Details and set-up: [MPE](#mpe).

### What comes next

- **More testing of MPE on real MPCs**, and tracking down the audio glitches heard in the first test.
- **Every patch in the MPC's preset list**, if the list of 76 works well on a device. See [Presets](#presets) for
  what stands in the way.

### 0.2.0: the SDK package

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

New in 0.3.0. **First test on a device: it works.** On an MPC Live III with a LinnStrument, playing Surge's
LinnStrument MPE "Bell Piano" patch, per-note bends and polyphonic aftertouch came through. That is one device, one
controller and one sitting, with some audio glitches; everything else below is still as tested offline.

In that test the LinnStrument went **straight into the plugin track**, with no MIDI track and no port in between.
So on MPC OS 3.10.1, at least on that device, the MPC does hand this plugin each note's channel. That is the
simplest set-up and the one to try first. The port route below is the fallback if notes all bend together.

### What is there

- **An MPE switch** on the PATCH page (the last tab), with a **Bend Range** control beside it (1 to 96 semitones,
  48 by default, which is the MPE standard and the LinnStrument's MPE setting). Both are saved with the MPC project
  and stay as they are when you load another patch.
- With MPE on, Surge's own MPE voice handling does the work: each note's **pitch bend** (slides), **pressure**
  (channel pressure on the note's channel) and **timbre** (CC 74, the LinnStrument's Y axis) reach that note's voice
  and no other. **Polyphonic aftertouch** reaches the key it was sent for, with MPE on or off. Channel 1 is the
  global channel: a bend there moves every note, by the scene's own Bend Up / Down.
- If the controller announces its bend range (the LinnStrument does, with RPN 0), Surge follows it and the Bend
  Range control shows it.
- **The plugin's own MIDI port, "SurgeXXL MIDI In".** This is the part that makes MPE possible on an MPC at all,
  and it comes from the [mpc-mpe](https://github.com/uncertainblume/mpc-mpe) project (see below).
- A **status line** under the switch: whether the port opened, and the channels note-ons have arrived on by each
  route (`SurgeXXL MIDI In: ch 2-8 | track: ch 1`). It is there so the routing can be checked on the device.

### Why a MIDI port

The MPC's sequencer records and plays each event's MIDI channel, so one track can hold an MPE performance. But a
plugin on a track is handed every note-on on channel 1, whatever channel it was played on. MPE depends on each
note's channel, so MPE through the track cannot work. A plugin can, however, open a MIDI port of its own at
operating-system level; the MPC lists it beside the hardware ports, and data sent to it keeps its channels. The
evidence, the measurements and the design are in mpc-mpe's
[docs/research.md](https://github.com/uncertainblume/mpc-mpe/blob/main/docs/research.md); the port code here
(`src/alsa_seq.h`) is that project's.

Once the port has delivered anything, SurgeXXL ignores MIDI from its own track (so nothing plays twice) until the
plugin is loaded again.

### Setting it up with a LinnStrument

The simple way (the one that has worked on a device): add a **plugin track** with SurgeXXL, set the track's MIDI
input to the LinnStrument with Input Channel **All**, turn **MPE** on on the PATCH page, load a patch from the
**LinnStrument MPE** folders, and play.

If every note bends together that way, use the plugin's own port instead. On the MPC:

1. Preferences > MIDI / Sync > Input Ports, the LinnStrument's row: **Track** on, **Global** off.
2. Add a **plugin track** with SurgeXXL. On its PATCH page turn **MPE** on.
3. Add a **MIDI track**: MIDI Input Port the LinnStrument, Input Channel **All**; MIDI Output Port **SurgeXXL MIDI
   In**, Output Channel **Any**; Monitor **Auto** (or Merge); record-arm it.
4. Load a patch from the **LinnStrument MPE** category and play. Record on the MIDI track: every event is stored
   with its channel and plays back as played.

On the LinnStrument (Per-Split Settings): MIDI Mode **ChPerNote**, Main channel 1, Per-Note channels 2 and up, Bend
Range **48**, Timbre/Y **CC 74**, Loudness/Z **Channel Pressure** (Poly Pressure also works). Holding the ChPerNote
button sets the firmware's MPE setup in one go.

Then read the status line. `track: ch 2-8` (the simple way) or `SurgeXXL MIDI In: ch 2-8` (the port), or however
many channels you play, is right. `ch 1` only
means the channels were flattened on the way: check Input Channel All and Output Channel Any. `No MIDI port` means
the MPC did not let the plugin open a port. mpc-mpe's
[routing guide](https://github.com/uncertainblume/mpc-mpe/blob/main/docs/routing-guide.md) has more recipes and a
troubleshooting table.

### What is not known yet

- How this behaves on any MPC other than the one Live III it was tried on, and on the 32-bit models at all.
- The port is an operating-system feature that Akai's SDK neither documents nor forbids. A later MPC OS could stop
  listing such ports.
- Whether recording and playing back an MPE take behaves the same as playing live.
- Where the audio glitches heard in the first test come from.
- The plugin now answers yes when the MPC asks whether it understands MPE. What the MPC does differently for a
  plugin that says yes is undocumented. If the status line ever shows `track: ch 2-8`, the MPC is passing channels
  to the track directly and the port is not needed.
- MIDI through the port is timed to the audio block: about 3 ms at the default 128 frames.

## Presets

New in 0.3.1. **Built and tested offline; not yet tried on an MPC.**

The MPC shows a plugin's built-in programs in its own preset selector. SurgeXXL now offers 76 of them: Surge's
LinnStrument MPE patches, grouped by folder.

```
- Patch page -
== Basses ==
Bass Guitar
Legato Bass
String Bass
== Brass ==
FM Horn
...
```

- Pick a patch and it loads. Remember to turn **MPE** on (PATCH page) for these patches; the switch is not part of
  a patch.
- `- Patch page -` means "whatever the PATCH page loaded". It is what the list shows after you load some other
  patch from the PATCH page, and picking it does nothing.
- The headings are only there to group the list; picking one does nothing.
- Names are cut at 23 characters, the plugin format's limit, so "Vintage Demonic Brass Ensemble" reads
  "Vintage Demonic Brass E".

### Why only these, and what about folders

- **Real folders are not available.** The MPC's own instruments group their presets in folders, but that is Akai's
  own preset system. A third-party plugin can only hand the MPC one flat, numbered list of program names. The
  headings are the nearest thing to folders that a flat list allows.
- **The 1024 limit is not the MPC's.** It is a cap in the wrapper this plugin is built on, and it can be lifted.
  What is unknown is how the MPC's preset selector copes with a list of 3000 entries, and whether it can be scrolled
  quickly enough to be an improvement: nobody has reported trying.
- So this version starts with the patches the fork exists for. If the list of 76 behaves well on a device, the
  same mechanism can carry every patch (about 3400 entries with the headings), and that is the plan.

## About this fork, and how it was made

I am not a coder. This is vibe coded to the max: I describe what I want and an AI writes all of it. This port was
made with [Claude](https://claude.com/claude-code), Anthropic's AI model,
working in the repository: Claude read the original project and Akai's SDK, wrote the build changes, found and
fixed the two crashes above, and ran the tests. I chose the goal and the direction, and the testing on real hardware falls to me.

I will do my best to work with Claude on changes and updates when they seem necessary. If something is broken, open
an Issue and say which MPC and which MPC OS version you have and what you did; the crash log (below) helps a great
deal. Please be patient, and please do not take problems with this fork to L'Cronx or the Surge Synth Team: they
did not make this package.

## Disclaimer

This software is provided **as is, with no warranty of any kind**, and you use it **entirely at your own risk**.
It is experimental, written by an AI at the direction of someone who cannot review the code, and barely tested on
real hardware. It runs inside the MPC's own application with no crash protection.

To the fullest extent the law allows, nobody involved in this fork accepts any liability for anything that
happens as a result of using it: crashes, freezes, lost or corrupted projects, lost work, lost time, a ruined
session or performance, damage to equipment or hearing, or anything else. That holds for the original authors of
SurgeXXL, Surge XT and the libraries too, none of whom made this package. Sections 15 and 16 of the GNU General
Public License, version 3 (`LICENSE`), which cover exactly this, apply in full.

Back up your projects. Keep the volume down the first time you load it. Do not rely on it on stage.

This is an unofficial community project. It is not made, tested, supported or endorsed by Akai Professional,
inMusic, the Surge Synth Team, L'Cronx or Roger Linn Design. Using third-party plugins is at your own risk as far
as Akai is concerned too: read the warning the MPC shows when you turn External Plugin Support on.

## Thanks

- **[mpc-mpe](https://github.com/uncertainblume/mpc-mpe)**, the earlier project in which the way to get MPE into
  an MPC plugin was researched and built (also with Claude): the finding that a plugin's own MIDI port keeps its
  channels, the port code used here, and the routing recipes. It in turn rests on what the community measured in
  [mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins) and on the LinnStrument's open-source firmware.

- **L'Cronx**, for [SurgeXXL](https://github.com/sunskiefer/SurgeXXL). This fork is a repackaging of their
  instrument.
- **The Surge Synth Team** and every contributor to Surge XT, and Claes Johanson, who wrote Surge and gave it away.
- **Akai Professional and inMusic**, for opening the MPC to third-party plugins and publishing the SDK.
- **sd88me** and the mpc-vst-plugins contributors, whose wrapper and skin builder this plugin is built on.
- **Roger Linn and Geert Bevin**, creators of MPE and the Linnstrument, and **Wendy Carlos**, the pioneer without
  whom none of this would be possible. This is dedicated to the three of them.

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
| PATCH | The patch browser (`<` `>`, `CAT`, PATCH on a Q-Link), LOAD, SAVE, the loaded patch's name; the 8 macros with the patch's own names; Scene A/B, Scene Mode, Play Mode, Poly Limit, Portamento, Bend Up / Down, Split Point, Volume, PANIC; MPE (Off / On), MPE Bend Range and the MPE status line (this fork, 0.3.0). The MPC's own preset selector lists the LinnStrument MPE patches (0.3.1) |

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
- MPE (0.3.0), in the same test: on a sine patch, the pitch of a held note is measured while per-note bend,
  pressure, polyphonic aftertouch and timbre are sent on its own channel and on another one, with MPE on and off.
  The MIDI port itself cannot be tested offline (the emulator has no ALSA sequencer); it is the mpc-mpe code,
  which that project tests against the ALSA headers.
- mpc-vst-plugins' `tools/host_test.c` on the whole plugin (wrapper and engine): names, set/get, option stepping by
  wheel and Q-Link, the long lists' stepping, popups, note to audio, MPC's STOP quirk, the chunk, CC Q-Links, two threads.
- The SDK package: the SDK's `tools/validate.py` on the folder (0 errors; one warning, that `Content/` is over
  256 MB), and its `tools/host-sim` built for ARM and run under qemu on both libraries. `host_sim` passes its load,
  render, block size and teardown checks, and reports FAIL on a few parameters in its random sweep and state round
  trip; which ones depends on its random seed. All of them are browser and popup selectors (Patch, Wavetable,
  Wavetable Picture, and the "List" parameters that say whether a popup is open), which step through a list or are
  not part of the saved sound. The saved state itself comes back byte for byte. That they are harmless on a device
  is not yet confirmed.
- The skin: `./build.sh --skin-only`, then mpc-vst-plugins' `tools/studio.py preview` (the pages above are its output).

## Credits and licences

- **SurgeXXL** (the plugin layer, the pages, the tools): L'Cronx, GPL-3.0-or-later
  ([sunskiefer/SurgeXXL](https://github.com/sunskiefer/SurgeXXL)).
- **This fork** (the SDK packaging, the 64-bit build and its fixes, MPE): made with Claude, under the same licence,
  GPL-3.0-or-later (`LICENSE`).
- **The MIDI port** (`src/alsa_seq.h`): from [mpc-mpe](https://github.com/uncertainblume/mpc-mpe), MIT licence
  (in the file's header and `src/VENDORED.md`).
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
