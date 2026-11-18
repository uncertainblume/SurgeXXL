SurgeXXL 0.3.5 for MPC OS 3.10.1 or later: **the first save after a project loads, and a better benchmark.**

**Experimental. Not fully tested. No warranty and no liability: you use it at your own risk.** A plugin fault can
take the MPC down, so save and back up your project before loading it. See the README's warning, Known problems and
Disclaimer.

## What this version changes, step by step

1. **The first save after a project loads no longer costs a full save.** On a Live III that first save took 9.5 ms
   (about three audio blocks) because Surge recalculates a flag on some FX parameters on the first audio block after a
   load, which made a patch look edited. The check no longer counts that flag.
2. **Tests for it**: 46 untouched library patches must save as a file reference after a second of audio, and 30 edited
   patches must save again from the kept text after being restored and played (both now pass in full; each failed for
   some patches before the fix).
3. **The benchmark** keeps its state calls off the chord's first blocks, and has `--edited` for the full-save path.
4. **First measurements on a Live III** are in the README: an 8-note chord costs 4.7% to 35.8% of a block per
   instance (median 17%).

## What this does not change

A full save of an edited patch still takes about 9.5 ms on the device, once per edit and instance. It has not been shown
that this is what causes the glitch around autosave. Not yet tried on an MPC.

## Install

Unzip, copy the folder `L'Cronx - VST - SurgeXXL` into `VST2 Plugins` on the internal drive (replacing the old one),
turn on Preferences > Enhancements > Enable Enhancements and External Plugin Support, restart the MPC. About 450 MB
unpacked. Projects made with earlier versions load unchanged.

MPE in SurgeXXL builds on the mpc-mpe project: https://github.com/uncertainblume/mpc-mpe
SurgeXXL is by L'Cronx (https://github.com/sunskiefer/SurgeXXL). Surge XT is by the Surge Synth Team.
GPL-3.0-or-later; free, not to be sold.
