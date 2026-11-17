SurgeXXL 0.3.2 for MPC OS 3.10.1 or later: **Surge's factory patches in the MPC's preset list, and Auto Load.**

**Experimental. Not fully tested. No warranty and no liability: you use it at your own risk.** A plugin fault can
take the MPC down, so save and back up your project before loading it. See the README's warning, Known problems and
Disclaimer.

## What this version adds, step by step

1. **The MPC's preset list grows from 76 to 700 entries.** The LinnStrument MPE patches stay first, exactly where
   they were (so projects saved with 0.3.1 keep their entry), followed by Surge's 15 factory folders, each a heading
   followed by its patches: 609 more patches. Templates and Tutorials are left out.
2. **Auto Load** on the PATCH page (the lit switch beside SAVE, and on Q-Link page PATCH 3): a patch chosen with
   `<`, `>`, `CAT` or the PATCH knob loads by itself once you stop changing the choice for about 0.3 seconds, with no
   LOAD press. On by default. Note that an auto load replaces the sound and any edits to it, as LOAD does; switch
   Auto Load off if you would rather load by hand.
3. Tests for the list and for Auto Load, passing on the 64-bit build under an emulator.

Not yet tried on an MPC. The list of 700 has not been seen on a device: how the MPC's selector copes with a list
that long is not known.

## What is not here

- **Favorites and real folders.** Programs in a plugin's list are not files, so the MPC cannot star them. The MPC's
  own preset files might allow both; that needs a look at how the MPC stores presets first. See the README.
- Surge's 2,336 third-party patches are on the PATCH page only (with Auto Load).

## Install

Unzip, copy the folder `L'Cronx - VST - SurgeXXL` into `VST2 Plugins` on the internal drive (replacing the old one),
turn on Preferences > Enhancements > Enable Enhancements and External Plugin Support, restart the MPC. About 450 MB
unpacked. Projects made with earlier versions load unchanged.

MPE in SurgeXXL builds on the mpc-mpe project: https://github.com/uncertainblume/mpc-mpe
SurgeXXL is by L'Cronx (https://github.com/sunskiefer/SurgeXXL). Surge XT is by the Surge Synth Team.
GPL-3.0-or-later; free, not to be sold.
