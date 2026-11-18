SurgeXXL 0.3.4 for MPC OS 3.10.1 or later: **hanging notes after the MPC's glitch: recovery, and a way to find the cause.**

**Experimental. Not fully tested. No warranty and no liability: you use it at your own risk.** A plugin fault can
take the MPC down, so save and back up your project before loading it. See the README's warning, Known problems and
Disclaimer.

## What this version changes, step by step

1. **A resume after a suspend lets go of every held note.** If the MPC stops the plugin around a project save and
   starts it again, a note-off sent in between cannot arrive. A first start with no suspend before it does nothing.
2. **`surgexxl.log` says what happened**: the MPC suspending, resuming, starting and stopping the plugin; gaps of more
   than 50 ms between audio blocks, with the keys held; the keys held at every save; and a note-on for a key still
   held, which proves its note-off never arrived.
3. Tests for the above, passing on the 64-bit build under an emulator.

## What this does not claim

It is not known where the note-off is lost. If the MPC's own MIDI input queue overflows while the audio is stopped,
the note-off is gone before the plugin sees it and this cannot bring it back. After the next hang, the log shows which
case it was. Not yet tried on an MPC.

## Install

Unzip, copy the folder `L'Cronx - VST - SurgeXXL` into `VST2 Plugins` on the internal drive (replacing the old one),
turn on Preferences > Enhancements > Enable Enhancements and External Plugin Support, restart the MPC. About 450 MB
unpacked. Projects made with earlier versions load unchanged. Projects saved by 0.3.3 or 0.3.4 may not open in 0.3.2
or earlier.

MPE in SurgeXXL builds on the mpc-mpe project: https://github.com/uncertainblume/mpc-mpe
SurgeXXL is by L'Cronx (https://github.com/sunskiefer/SurgeXXL). Surge XT is by the Surge Synth Team.
GPL-3.0-or-later; free, not to be sold.
