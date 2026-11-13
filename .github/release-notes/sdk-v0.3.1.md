SurgeXXL 0.3.1 for MPC OS 3.10.1 or later: **the LinnStrument MPE patches in the MPC's preset list.**

**Experimental. Not fully tested. No warranty and no liability: you use it at your own risk.** A plugin fault can
take the MPC down, so save and back up your project before loading it. See the README's warning, Known problems and
Disclaimer.

## What this version adds, step by step

1. **76 programs in the MPC's own preset selector**: a first entry for the PATCH page's own sound, then Surge's 12
   LinnStrument MPE folders, each as a heading (`== Basses ==`) followed by its patches, 63 in all.
2. **Picking a patch loads it**, as LOAD on the PATCH page does. The headings and the first entry do nothing.
3. **The chosen entry is saved in the project**, so reopening a project does not reload the factory patch over your
   edits.
4. Tests for the list, the loading and the project state, passing on the 64-bit build under an emulator.

Not yet tried on an MPC. Turn MPE on (PATCH page) when you use these patches; the switch is not part of a patch.

## Why only these patches

A third-party plugin can only give the MPC one flat list of program names; real preset folders are not available to
it. This version starts with the 63 patches this fork exists for. If the list behaves well on a device, the same
mechanism can carry all 3008 patches, and that is the plan.

## Also since 0.3.0

MPE had its first test on a device and worked: per-note bends and polyphonic aftertouch came through on an MPC Live
III with a LinnStrument. Some audio glitches were heard; nothing else has been checked on hardware yet.

## Install

Unzip, copy the folder `L'Cronx - VST - SurgeXXL` into `VST2 Plugins` at the root of a drive (replacing the old
one), turn on Preferences > Enhancements > Enable Enhancements and External Plugin Support, restart the MPC. About
450 MB unpacked. Projects made with 0.2.0 and 0.3.0 load unchanged.

MPE in SurgeXXL builds on the mpc-mpe project: https://github.com/uncertainblume/mpc-mpe
SurgeXXL is by L'Cronx (https://github.com/sunskiefer/SurgeXXL). Surge XT is by the Surge Synth Team.
GPL-3.0-or-later; free, not to be sold.
