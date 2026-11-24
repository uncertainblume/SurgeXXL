SurgeXXL 0.3.6 for MPC OS 3.10.1 or later: **pads on the MPE patches.**

**Experimental. Not fully tested. No warranty and no liability: you use it at your own risk.** A plugin fault can
take the MPC down, so save and back up your project before loading it. See the README's warning, Known problems and
Disclaimer.

## What this version changes

1. **With MPE on, a note that arrives with no pressure now starts at a pressure equal to its velocity.** Surge's MPE
   patches take their loudness from each note's pressure. A LinnStrument sends it before every note; the MPC's pads (and
   a recorded take that lost it) do not, so on 34 of the 63 LinnStrument MPE patches such a note was less than half as
   loud, and on several nearly silent. A note that brings its own pressure is untouched, and with MPE off nothing is added.
2. **Tests**: every one of the 63 patches now plays a pad note at least half as loud as with pressure.

## What this does not claim

It does not explain why pad notes reach a SurgeXXL track set to take MIDI from the bridge only; that is the MPC's routing,
and the PATCH page's status line shows whether notes arrive by the track or by SurgeXXL's own port. Not yet tried on an MPC.

## Install

Unzip, copy the folder `L'Cronx - VST - SurgeXXL` into `VST2 Plugins` on the internal drive (replacing the old one),
turn on Preferences > Enhancements > Enable Enhancements and External Plugin Support, restart the MPC. About 450 MB
unpacked. Projects made with earlier versions load unchanged.

MPE in SurgeXXL builds on the mpc-mpe project: https://github.com/uncertainblume/mpc-mpe
SurgeXXL is by L'Cronx (https://github.com/sunskiefer/SurgeXXL). Surge XT is by the Surge Synth Team.
GPL-3.0-or-later; free, not to be sold.
