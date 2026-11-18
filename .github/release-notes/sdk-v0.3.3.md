SurgeXXL 0.3.3 for MPC OS 3.10.1 or later: **a lighter project save, and a benchmark.**

**Experimental. Not fully tested. No warranty and no liability: you use it at your own risk.** A plugin fault can
take the MPC down, so save and back up your project before loading it. See the README's warning, Known problems and
Disclaimer.

## What this version changes, step by step

1. **A save of an untouched library patch is now tiny.** The MPC asks the plugin for its state when it saves a
   project and at every autosave, and until now that meant Surge writing out the whole patch (about 45 KB of text),
   during which that instance's audio waited. An unedited patch from Surge's library is now saved as its file name
   (about 90 bytes), and restored by loading that file.
2. **A patch you have edited is saved in full, as before**, but the saved text is kept and handed out again while
   nothing changes, so an autosave of something you are only playing costs almost nothing.
3. **It never saves a stale sound.** The text is rebuilt whenever anything that can change a patch is touched, and a
   fingerprint of every parameter value catches the rest. The test restores a save after each kind of edit and after
   26 random edits and requires every value to match.
4. **The log** (`surgexxl.log`, in the `SurgeXXL` folder at the root of the drive) now records every save and what it
   cost. After the next autosave it shows the real figure on your MPC.
5. **`tools/bench/mpc_bench.cpp`**, a benchmark to run on the MPC over SSH: per patch, how much of the 2.9 ms audio
   block Surge uses, how many blocks overran, and how long a block is held up by a state call. See the README.

## What this does not claim

It has not been shown that autosave causes the audio glitches. This removes one suspect and adds the tools to look at
the rest. Surge's own CPU use is untouched. Not yet tried on an MPC.

## Install

Unzip, copy the folder `L'Cronx - VST - SurgeXXL` into `VST2 Plugins` on the internal drive (replacing the old one),
turn on Preferences > Enhancements > Enable Enhancements and External Plugin Support, restart the MPC. About 450 MB
unpacked. Projects made with earlier versions load unchanged. Projects saved by 0.3.3 may not open in 0.3.2 or earlier.

MPE in SurgeXXL builds on the mpc-mpe project: https://github.com/uncertainblume/mpc-mpe
SurgeXXL is by L'Cronx (https://github.com/sunskiefer/SurgeXXL). Surge XT is by the Surge Synth Team.
GPL-3.0-or-later; free, not to be sold.
