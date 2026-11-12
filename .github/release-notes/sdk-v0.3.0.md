SurgeXXL 0.3.0 for MPC OS 3.10.1 or later: **MPE**.

**Experimental. Not fully tested, and not yet confirmed on an MPC. No warranty and no liability: you use it at your
own risk.** A plugin fault can take the MPC down, so save and back up your project before loading it. See the
README's warning, Known problems and Disclaimer.

## What this version adds, step by step

1. **An MPE switch and an MPE Bend Range control** on the PATCH page (and Q-Link page PATCH 3). With MPE on, each
   note's pitch bend, pressure and timbre (CC 74) reach its own voice, through Surge's own MPE voice handling.
   Polyphonic aftertouch reaches its key. Bend range 1 to 96 semitones, 48 by default; a controller that announces
   its range sets it. Saved with the project, kept when you load another patch.
2. **The plugin's own MIDI port, "SurgeXXL MIDI In".** The MPC hands a plugin on a track every note-on on channel
   1, so MPE cannot arrive through the track. A MIDI track that sends to this port, output channel Any, keeps each
   note's channel.
3. **A status line** under the switch that shows whether the port opened and which channels are arriving by each
   route, so the routing can be checked on the device.
4. The plugin tells the MPC it understands MPE when asked.
5. **Tests** for all of the above except the port itself, passing on the 64-bit build under an emulator.

The way to get MPE into an MPC plugin (the finding about channels, the MIDI port technique and the port code) comes
from the **mpc-mpe** project: https://github.com/uncertainblume/mpc-mpe

## Using it with a LinnStrument

Plugin track: SurgeXXL, MPE on. MIDI track: input the LinnStrument, channel All; output port SurgeXXL MIDI In,
channel Any; record-armed. LinnStrument: ChPerNote, Bend Range 48, Y as CC 74, Z as Channel Pressure. Load a patch
from the LinnStrument MPE category. Full steps in the README.

## Not known yet

- Whether MPC OS 3.10.1 lists the plugin's port (the technique was measured by the community on 3.9.1), and whether
  live monitoring keeps the channels. The status line will tell.
- Everything in the README's Known problems still applies, patch browsing first among them.

## Next

- Confirming MPE on a real MPC, and fixing what that turns up.
- Surge's patches in the MPC's own preset list, with their folder structure. Not started.

## Install

Unzip, copy the folder `L'Cronx - VST - SurgeXXL` into `VST2 Plugins` at the root of a drive, turn on Preferences >
Enhancements > Enable Enhancements and External Plugin Support, restart the MPC. About 450 MB unpacked. Projects made
with 0.2.0 load unchanged, with MPE off.

SurgeXXL is by L'Cronx (https://github.com/sunskiefer/SurgeXXL). Surge XT is by the Surge Synth Team.
GPL-3.0-or-later; free, not to be sold.
