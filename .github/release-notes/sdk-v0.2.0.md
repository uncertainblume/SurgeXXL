SurgeXXL 0.2.0 as an MPC plugin SDK package, for MPC OS 3.10.1 or later.

**Install:** unzip, copy the folder `L'Cronx - VST - SurgeXXL` into `VST2 Plugins` at the root of a drive, turn on
Preferences > Enhancements > Enable Enhancements and External Plugin Support, restart the MPC. It is under External
Plugins. About 450 MB unpacked.

- 64-bit (MPC Live III, MPC XL) and 32-bit (MPC X, Live, One, Key, Force) libraries in one folder.
- Fixed on 64-bit: a crash in formula modulators (LuaJIT built with the wrong byte order).
- Fixed: a crash when a host unloads the plugin.
- Your patches and wavetables are in `SurgeXXL` at the root of the drive.

Tested offline only (the engine test and the SDK's validator and host simulator, under an emulator). Not yet
confirmed on a device: save your project before loading it. MPE mode has no switch yet; see the README.

SurgeXXL is by L'Cronx (https://github.com/sunskiefer/SurgeXXL). Surge XT is by the Surge Synth Team.
GPL-3.0-or-later; free, not to be sold.
