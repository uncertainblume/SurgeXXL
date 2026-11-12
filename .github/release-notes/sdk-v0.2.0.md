SurgeXXL 0.2.0 as an MPC plugin SDK package, for MPC OS 3.10.1 or later.

**Install:** unzip, copy the folder `L'Cronx - VST - SurgeXXL` into `VST2 Plugins` at the root of a drive, turn on
Preferences > Enhancements > Enable Enhancements and External Plugin Support, restart the MPC. It is under External
Plugins. About 450 MB unpacked.

- 64-bit (MPC Live III, MPC XL) and 32-bit (MPC X, Live, One, Key, Force) libraries in one folder.
- Fixed on 64-bit: a crash in formula modulators (LuaJIT built with the wrong byte order).
- Fixed: a crash when a host unloads the plugin.
- Your patches and wavetables are in `SurgeXXL` at the root of the drive.

**Experimental: not fully tested, and it may have major usability problems.** The checks ran offline, under an
emulator; it has had very little use on real hardware. A plugin fault can take the MPC down, so save your project
before loading it. Known problems: patches can only be browsed one at a time or by category on the PATCH page (they
are not MPC presets), and MPE mode has no switch yet. See the README.

SurgeXXL is by L'Cronx (https://github.com/sunskiefer/SurgeXXL). Surge XT is by the Surge Synth Team.
GPL-3.0-or-later; free, not to be sold.
