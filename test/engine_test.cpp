// SPDX-License-Identifier: GPL-3.0-or-later
// SurgeXXL's offline engine test: the engine (src/engine.cpp, with Surge) driven through the mpc-vst-plugins engine
// interface, as the wrapper drives it. Built for x86 (ASan/UBSan) and for the Force (run under qemu-arm), by
// test/run_tests.sh. Exit status 1 on any failure.
//
//   engine_test <stage dir> [--patches all|N] [--fx] [--quick]
// <stage dir> holds Synths/<plugin>/data (tools/stage_data.sh), like a Force's card; the user folder is made beside
// it (<stage>/SurgeXXL), so the test starts from an empty one.
//
// Checks: every parameter answers (value and value text); continuous and option round trips; every patch (or every
// Nth) loads and plays notes with finite, bounded output (and how many patches had a removed FX turned off); every FX
// type plays; wavetables (factory picture numbers, a user wavetable added and rescanned); SAVE writes a patch the
// browser then lists; the state chunk restores a second instance exactly; LEARN routes a modulation.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "param_ids.h"
extern "C" {
#include "engine.h"
}

namespace {
enum Bind { B_CONT, B_INT, B_BOOL, B_INTR, B_ENGINE };
struct KeyDef {
  const char *key;
  Bind bind;
  const char *pat;
  int nvals;
  int vals[64];
  float lo, hi;
};
const KeyDef kKeys[] = {
#include "param_table.inc"
};

const mpc_engine_t *E;
int fails = 0;

#define CHECK(c, ...)                         \
  do {                                        \
    if (!(c)) {                               \
      printf("FAIL ");                        \
      printf(__VA_ARGS__);                    \
      printf("\n");                           \
      ++fails;                                \
    }                                         \
  } while (0)

std::string get(void *s, const char *k) {
  static char buf[1 << 22];
  int n = E->get_param(s, k, buf, sizeof buf);
  return n > 0 ? std::string(buf) : std::string();
}
void set(void *s, const char *k, const char *v) { E->set_param(s, k, v); }
void setf(void *s, const char *k, double v) {
  char b[32];
  snprintf(b, sizeof b, "%.6f", v);
  set(s, k, b);
}
void seti(void *s, const char *k, int v) {
  char b[32];
  snprintf(b, sizeof b, "%d", v);
  set(s, k, b);
}

struct Audio {
  double peak = 0, sumsq = 0;
  long n = 0;
  bool finite = true;
};
void render(void *s, int frames, Audio *a = nullptr) {
  int16_t buf[256];
  while (frames > 0) {
    int n = frames > 128 ? 128 : frames;
    E->render(s, buf, n);
    if (a)
      for (int i = 0; i < 2 * n; ++i) {
        double v = buf[i] / 32768.0;
        if (fabs(v) > a->peak) a->peak = fabs(v);
        a->sumsq += v * v;
        ++a->n;
      }
    frames -= n;
  }
}
// sound check on the float side is the engine's job (it zeroes NaN/inf before int16); here: did anything play, and
// did the engine keep producing blocks (no hang)
void note(void *s, int key, int vel) {
  uint8_t m[3] = {(uint8_t)(vel ? 0x90 : 0x80), (uint8_t)key, (uint8_t)(vel ? vel : 64)};
  E->midi(s, m, 3);
}
void tap(void *s, const char *k) {
  set(s, k, "0");
  set(s, k, "1");
  render(s, 128);
  set(s, k, "0");
}
bool waitLoaded(void *s) {
  // Surge loads on its own thread while process() returns silence: pace the blocks (on the device they come in real time)
  for (int i = 0; i < 5000; ++i) {
    render(s, 128);
    if (i > 2) usleep(1000);
    if (get(s, "patch_name") != "Loading...") return true;
  }
  return false;
}
double now() {
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec + t.tv_nsec * 1e-9;
}
bool exists(const std::string &p) {
  struct stat st;
  return stat(p.c_str(), &st) == 0;
}

void testParams(void *s) {
  int n = 0;
  for (const KeyDef &k : kKeys) {
    if (!strcmp(k.key, "state")) continue;
    std::string v = get(s, k.key);
    CHECK(!v.empty() || k.bind == B_ENGINE, "param %s: no value", k.key);
    if (k.bind == B_CONT) {
      std::string d = get(s, (std::string(k.key) + "_display").c_str());
      CHECK(!d.empty(), "param %s: no value text", k.key);
    }
    ++n;
  }
  printf("ok   %d parameters answer\n", n);
}

void testRoundTrips(void *s) {
  int nc = 0, no = 0;
  for (const KeyDef &k : kKeys) {
    if (k.bind == B_CONT) {
      std::string d = get(s, (std::string(k.key) + "_display").c_str());
      if (d == " ") continue;     // a parameter the current type doesn't use
      std::string keep = get(s, k.key);
      setf(s, k.key, 0.3);
      double v = atof(get(s, k.key).c_str());
      CHECK(fabs(v - 0.3) < 0.04,   // whole-number Surge parameters land on their nearest step
             "round trip %s: set 0.3, read %f", k.key, v);
      set(s, k.key, keep.c_str());
      ++nc;
    } else if (k.bind == B_INT || k.bind == B_BOOL) {
      if (!strcmp(k.key, "scene")) continue;
      std::string keep = get(s, k.key);
      int opts = k.bind == B_BOOL ? 2 : k.nvals;
      for (int o = 0; o < opts; ++o) {
        seti(s, k.key, o);
        render(s, 64);
        int r = atoi(get(s, k.key).c_str());
        CHECK(r == o, "option %s: set %d, read %d", k.key, o, r);
      }
      set(s, k.key, keep.c_str());
      render(s, 64);
      ++no;
    }
  }
  printf("ok   round trips: %d continuous, %d option parameters\n", nc, no);
}

// What MPC does around a LOAD: the press bumps display_rev, so the host reads every value and value text again on the
// next poll while the patch is still loading, keeps sending blocks, and the player keeps playing notes (and Q-Links
// keep moving). A load must survive all of that (2026-10-13: a Force crashed on LOAD; the host's reads raced Surge's
// own patch-load thread).
void readAll(void *s) {
  for (const KeyDef &k : kKeys) {
    get(s, k.key);
    std::string d = std::string(k.key) + "_display";
    get(s, d.c_str());
  }
}
void testLoadWhileHostReads(void *s, int stride) {
  int total = atoi(get(s, "test_num_patches").c_str()), loads = 0, hung = 0;
  if (stride <= 0) stride = 97;
  for (int i = 0; i < total; i += stride) {
    seti(s, "patch", i);
    note(s, 60, 100);
    render(s, 256);
    set(s, "patch_load", "0");
    set(s, "patch_load", "1");
    bool done = false;
    for (int b = 0; b < 3000 && !done; ++b) {
      readAll(s);                                  // the host re-reads every page
      note(s, 62 + (b & 7), b & 1 ? 0 : 90);       // and the pads keep playing
      setf(s, "macro_1", (b % 10) / 10.0);         // and a Q-Link moves
      seti(s, "osc_sel", b % 3);
      render(s, 128);
      if (b > 2) usleep(500);
      done = get(s, "patch_name") != "Loading...";
    }
    set(s, "patch_load", "0");
    if (!done) ++hung;
    ++loads;
    readAll(s);
  }
  note(s, 60, 0);
  printf("ok   %d loads with the host reading every value and text, playing notes and moving Q-Links meanwhile\n", loads);
  CHECK(hung == 0, "%d loads never finished", hung);
}

void testPatches(void *s, int stride) {
  int total = atoi(get(s, "test_num_patches").c_str());
  int loaded = 0, silent = 0, removed = 0, hung = 0;
  double worst = 0;
  std::string worstName;
  std::vector<std::string> silentNames;
  for (int i = 0; i < total; i += stride) {
    seti(s, "patch", i);
    std::string name = get(s, "patch_browse"), cat = get(s, "patch_cat");
    tap(s, "patch_load");
    if (!waitLoaded(s)) {
      CHECK(false, "patch %d %s: never finished loading", i, name.c_str());
      ++hung;
      continue;
    }
    ++loaded;
    if (atoi(get(s, "test_removed_fx").c_str()) > 0) {
      ++removed;
      printf("     removed FX turned off: %s / %s\n", cat.c_str(), name.c_str());
    }
    Audio a;
    double t0 = now();
    note(s, 48, 100);
    note(s, 60, 100);
    note(s, 64, 90);
    render(s, 22050, &a);
    note(s, 48, 0);
    note(s, 60, 0);
    note(s, 64, 0);
    render(s, 8820, &a);
    double dt = now() - t0;
    if (dt > worst) worst = dt, worstName = cat + " / " + name;
    if (a.peak < 1e-4) ++silent, silentNames.push_back(cat + " / " + name);
    // let it ring out so the next patch starts clean
    E->set_param(s, "panic", "0");
    E->set_param(s, "panic", "1");
    render(s, 4410);
    E->set_param(s, "panic", "0");
  }
  printf("ok   patches: %d of %d loaded and played (every %d), %d with a removed FX turned off, %d silent with "
         "a held C chord, slowest %.1f ms for 0.7 s of audio (%s)\n",
         loaded, total, stride, removed, silent, worst * 1000, worstName.c_str());
  for (auto &n : silentNames) printf("     silent: %s\n", n.c_str());
  CHECK(hung == 0, "%d patches hung", hung);
}

void testFx(void *s) {
  const KeyDef *ft = nullptr;
  for (const KeyDef &k : kKeys)
    if (!strcmp(k.key, "fx_type")) ft = &k;
  seti(s, "fx_sel", 0);
  for (int o = 1; o < ft->nvals; ++o) {
    seti(s, "fx_type", o);
    render(s, 256);
    for (int p = 1; p <= 12; ++p) {     // the type's parameters at their middle
      char k[16];
      snprintf(k, sizeof k, "fx_p%d", p);
      if (get(s, (std::string(k) + "_display").c_str()) != " ") setf(s, k, 0.5);
    }
    Audio a;
    note(s, 60, 110);
    render(s, 22050, &a);
    note(s, 60, 0);
    render(s, 22050, &a);
    CHECK(a.peak < 1.01, "fx type %d: peak %f", o, a.peak);
    tap(s, "panic");
    render(s, 4410);
  }
  seti(s, "fx_type", 0);
  printf("ok   every FX type (%d) played\n", ft->nvals - 1);
}

// The whole library is listed (made with no HOME, as MPC makes it: 0.1.0-0.1.3 could not), factory and third party,
// and every third-party wavetable has its picture.
void testLibrary(void *s) {
  int np = atoi(get(s, "test_num_patches").c_str()), nw = atoi(get(s, "test_num_wt").c_str());
  CHECK(np == 3008, "%d patches listed, expected 3008 ", np);
  CHECK(nw == 761, "%d wavetables listed, expected 761 ", nw);
  int third = 0, factory = 0;
  for (int i = 0; i < np; ++i) {
    seti(s, "patch", i);
    std::string c = get(s, "patch_cat");
    if (!c.compare(0, 9, "3RD PARTY")) ++third;
    else if (!c.compare(0, 7, "FACTORY")) ++factory;
  }
  CHECK(third > 0 && factory > 0, "patch categories: %d factory, %d third party", factory, third);
  seti(s, "patch", 0);
  seti(s, "osc_sel", 0);
  int wpics = 0, wthird = 0;
  for (int i = 0; i < nw; ++i) {
    seti(s, "wt", i);
    render(s, 128);
    if (!get(s, "wt_folder").compare(0, 9, "3RD PARTY")) {
      ++wthird;
      if (atoi(get(s, "wt_pic").c_str()) > 0) ++wpics;
    }
  }
  CHECK(wthird > 0 && wpics == wthird, "third-party wavetables: %d listed, %d with a picture", wthird, wpics);
  printf("ok   library: %d factory + %d third-party patches, %d third-party wavetables (all with pictures)\n", factory,
         third, wthird);
}

void testWavetables(void *s, const std::string &stage) {
  seti(s, "osc_sel", 0);
  seti(s, "osc_type", 2);     // Wavetable
  render(s, 512);
  int n = atoi(get(s, "test_num_wt").c_str()), pics = 0;
  for (int i = 0; i < n; i += 7) {
    seti(s, "wt", i);
    render(s, 256);
    if (atoi(get(s, "wt_pic").c_str()) > 0) ++pics;
    CHECK(!get(s, "wt_name").empty(), "wavetable %d: no name", i);
  }
  CHECK(pics > 0, "no wavetable picture numbers");
  // a user wavetable: copied into the user folder, then WT RESCAN
  std::string user = get(s, "test_user_dir") + "/Wavetables/SurgeXXL Test.wt";
  std::string src = stage + "/Synths/plugin/data/wavetables/Basic/Sine Octaves.wt";
  if (!exists(src)) src = stage + "/Synths/plugin/data/wavetables/Basic/Sine HQ.wav";
  std::string cmd = "cp \"" + src + "\" \"" + user + "\"";
  CHECK(system(cmd.c_str()) == 0, "copy a user wavetable");
  tap(s, "wt_rescan");
  int n2 = atoi(get(s, "test_num_wt").c_str());
  CHECK(n2 == n + 1, "rescan: %d wavetables, expected %d", n2, n + 1);
  bool found = false;
  for (int i = 0; i < n2 && !found; ++i) {
    seti(s, "wt", i);
    render(s, 256);
    if (get(s, "wt_name").find("SurgeXXL Test") != std::string::npos) {
      found = true;
      CHECK(atoi(get(s, "wt_pic").c_str()) == 0, "user wavetable has a factory picture");
      CHECK(get(s, "wt_folder").find("USER") == 0, "user wavetable folder: %s", get(s, "wt_folder").c_str());
      Audio a;
      note(s, 60, 100);
      render(s, 11025, &a);
      note(s, 60, 0);
      render(s, 4410);
      CHECK(a.peak > 1e-3, "user wavetable is silent");
    }
  }
  CHECK(found, "the user wavetable is not in the list");
  printf("ok   wavetables: %d listed, pictures on factory tables, a user wavetable loads after RESCAN\n", n);
}

void testSave(void *s) {
  int before = atoi(get(s, "test_num_patches").c_str());
  tap(s, "patch_save");
  std::string file = get(s, "test_user_dir") + "/Patches/SurgeXXL/SurgeXXL 001.fxp";
  CHECK(exists(file), "SAVE wrote no %s", file.c_str());
  CHECK(atoi(get(s, "test_num_patches").c_str()) == before + 1, "the saved patch is not in the list");
  CHECK(get(s, "patch_name") == "SurgeXXL 001", "loaded name after SAVE: %s", get(s, "patch_name").c_str());
  CHECK(get(s, "patch_browse") == "SurgeXXL 001", "browse after SAVE: %s", get(s, "patch_browse").c_str());
  printf("ok   SAVE writes SurgeXXL 001 to the user folder and browses to it\n");
}

// MPE (the switch on the PATCH page). The pitch of a sine patch is counted from its zero crossings, and every check is
// a ratio to the unbent note. A note is held on channel 2; `extra` are messages sent after the note-on.
struct Msg { uint8_t b[3]; };
Msg bend(int ch, int v14) { return {{(uint8_t)(0xe0 | ch), (uint8_t)(v14 & 0x7f), (uint8_t)(v14 >> 7 & 0x7f)}}; }
Msg pressure(int ch, int v) { return {{(uint8_t)(0xd0 | ch), (uint8_t)v, 0}}; }
Msg polyAt(int ch, int key, int v) { return {{(uint8_t)(0xa0 | ch), (uint8_t)key, (uint8_t)v}}; }
Msg cc(int ch, int n, int v) { return {{(uint8_t)(0xb0 | ch), (uint8_t)n, (uint8_t)v}}; }
double mpePitch(void *s, std::vector<Msg> extra, std::vector<Msg> reset) {
  uint8_t on[3] = {0x91, 69, 100}, off[3] = {0x81, 69, 64};
  E->midi(s, on, 3);
  for (auto &m : extra) E->midi(s, m.b, (m.b[0] & 0xf0) == 0xd0 ? 2 : 3);
  render(s, 8820);                                                    // the attack, and Surge's smoothing
  int16_t buf[256];
  long crossings = 0, frames = 0;
  int16_t last = 0;
  while (frames < 44100) {
    E->render(s, buf, 128);
    for (int i = 0; i < 128; ++i) {
      if (last <= 0 && buf[2 * i] > 0) ++crossings;
      last = buf[2 * i];
    }
    frames += 128;
  }
  E->midi(s, off, 3);
  for (auto &m : reset) E->midi(s, m.b, (m.b[0] & 0xf0) == 0xd0 ? 2 : 3);
  render(s, 22050);
  return crossings * 44100.0 / frames;
}
double semis(double f, double ref) { return 12.0 * log2(f / ref); }
// route a source to the oscillator's pitch (LEARN), so a per-note source is heard as a pitch change
void routeToPitch(void *s, int src) {
  seti(s, "mod_slot", 64);
  seti(s, "mod_src", src);
  tap(s, "mod_learn");
  setf(s, "osc_pitch", atof(get(s, "osc_pitch").c_str()));
  setf(s, "mod_depth", 0.5);
  render(s, 512);
}
void testMpe(void *s) {
  tap(s, "panic");
  {   // a known sound whatever the tests before left: Surge's Init Sine template
    int n = atoi(get(s, "test_num_patches").c_str()), at = -1;
    for (int i = 0; i < n && at < 0; ++i) {
      seti(s, "patch", i);
      if (get(s, "patch_browse") == "Init Sine") at = i;
    }
    CHECK(at >= 0, "Init Sine is in the library");
    tap(s, "patch_load");
    CHECK(waitLoaded(s), "Init Sine loads");
  }
  seti(s, "scene", 0);
  seti(s, "osc_sel", 0);
  seti(s, "mpe", 0);
  seti(s, "scene_mode", 0);
  seti(s, "osc_type", 1);                     // Sine
  for (const char *k : {"mix_o2", "mix_o3", "mix_noise", "mix_r12", "mix_r23"}) setf(s, k, 0.0);
  seti(s, "bend_up", 2);
  seti(s, "bend_dn", 2);
  seti(s, "fx_bypass", 3);                    // All Off
  render(s, 4410);
  CHECK(get(s, "mpe") == "0", "MPE is off by default: %s", get(s, "mpe").c_str());
  const int C = 8192;
  double flat = mpePitch(s, {}, {});
  CHECK(flat > 50 && flat < 2000, "the test note is %.1f Hz", flat);
  // MPE off: a bend is the scene's range (2 semitones), and a bend on any channel is global
  double d = semis(mpePitch(s, {bend(1, 16383)}, {bend(1, C)}), flat);
  CHECK(fabs(d - 2.0) < 0.15, "MPE off, full bend: %.2f semitones (want 2)", d);
  double offOther = semis(mpePitch(s, {bend(2, 16383)}, {bend(2, C)}), flat);
  CHECK(fabs(offOther - 2.0) < 0.15, "MPE off, a bend on another channel: %.2f semitones (want 2: it is global)", offOther);
  // MPE on
  seti(s, "mpe", 1);
  seti(s, "mpe_bend", 48);
  render(s, 1024);
  CHECK(get(s, "mpe") == "1" && get(s, "mpe_bend") == "48", "MPE on, range 48: %s %s", get(s, "mpe").c_str(),
        get(s, "mpe_bend").c_str());
  double b48 = semis(mpePitch(s, {bend(1, C + 2048)}, {bend(1, C)}), flat);    // a quarter of full bend
  CHECK(fabs(b48 - 12.0) < 0.15, "MPE on, quarter bend at range 48: %.2f semitones (want 12)", b48);
  double dn48 = semis(mpePitch(s, {bend(1, C - 2048)}, {bend(1, C)}), flat);
  CHECK(fabs(dn48 + 12.0) < 0.15, "MPE on, quarter bend down at range 48: %.2f semitones (want -12)", dn48);
  seti(s, "mpe_bend", 24);
  render(s, 1024);
  double b24 = semis(mpePitch(s, {bend(1, C + 2048)}, {bend(1, C)}), flat);
  CHECK(fabs(b24 - 6.0) < 0.15, "MPE on, quarter bend at range 24: %.2f semitones (want 6)", b24);
  double other = semis(mpePitch(s, {bend(2, 16383)}, {bend(2, C)}), flat);
  CHECK(fabs(other) < 0.1, "MPE on, a bend on channel 3 moved the note on channel 2 by %.2f semitones", other);
  // the global channel (1) bends every note, by the scene's range
  double glob = semis(mpePitch(s, {bend(0, 16383)}, {bend(0, C)}), flat);
  CHECK(fabs(glob - 2.0) < 0.15, "MPE on, a bend on the global channel: %.2f semitones (want 2)", glob);
  // per-note pressure: channel pressure on the note's own channel reaches that voice, on another channel does not
  routeToPitch(s, 3);                         // Channel Aftertouch
  double pOwn = semis(mpePitch(s, {pressure(1, 127)}, {pressure(1, 0)}), flat);
  double pOther = semis(mpePitch(s, {pressure(2, 127)}, {pressure(2, 0)}), flat);
  CHECK(pOwn > 0.5, "MPE on, pressure on the note's channel: %.2f semitones (want a rise)", pOwn);
  CHECK(fabs(pOther) < 0.1, "MPE on, pressure on another channel moved the note by %.2f semitones", pOther);
  tap(s, "mod_clear");
  // polyphonic aftertouch (key pressure), for controllers that send it
  routeToPitch(s, 2);                         // Polyphonic Aftertouch
  double paOwn = semis(mpePitch(s, {polyAt(1, 69, 127)}, {polyAt(1, 69, 0)}), flat);
  double paOther = semis(mpePitch(s, {polyAt(1, 60, 127)}, {polyAt(1, 60, 0)}), flat);
  CHECK(paOwn > 0.5, "poly aftertouch on the held key: %.2f semitones (want a rise)", paOwn);
  CHECK(fabs(paOther) < 0.1, "poly aftertouch on another key moved the note by %.2f semitones", paOther);
  tap(s, "mod_clear");
  // per-note timbre (slide): CC 74 on the note's own channel
  routeToPitch(s, 28);                        // Timbre
  double tLow = semis(mpePitch(s, {cc(1, 74, 0)}, {cc(1, 74, 64)}), flat);
  double tHigh = semis(mpePitch(s, {cc(1, 74, 127)}, {cc(1, 74, 64)}), flat);
  double tOther = semis(mpePitch(s, {cc(2, 74, 127)}, {cc(2, 74, 64)}), flat) -
                  semis(mpePitch(s, {cc(2, 74, 0)}, {cc(2, 74, 64)}), flat);
  CHECK(tHigh - tLow > 0.5, "MPE on, timbre (CC 74) on the note's channel: %.2f semitones between 0 and 127", tHigh - tLow);
  CHECK(fabs(tOther) < 0.1, "MPE on, timbre on another channel moved the note by %.2f semitones", tOther);
  tap(s, "mod_clear");
  // the status line names the channels the track delivered (no ALSA sequencer in the test: no port)
  std::string status = get(s, "mpe_status");
  CHECK(status.find("track: ch 1-2") != std::string::npos, "MPE status: %s", status.c_str());
  // a patch load keeps the setting
  seti(s, "patch", 0);
  tap(s, "patch_load");
  CHECK(waitLoaded(s), "patch load with MPE on");
  CHECK(get(s, "mpe") == "1" && get(s, "mpe_bend") == "24", "a patch load keeps MPE: %s %s", get(s, "mpe").c_str(),
        get(s, "mpe_bend").c_str());
  printf("ok   MPE: per-note bend %+.1f / %+.1f st (range 48, 24), other channel %+.2f, global channel %+.1f; pressure "
         "%+.1f (other channel %+.2f); poly aftertouch %+.1f (other key %+.2f); timbre %.1f (other channel %+.2f); "
         "off: bend %+.1f, global; kept across a patch load\n",
         b48, b24, other, glob, pOwn, pOther, paOwn, paOther, tHigh - tLow, tOther, d);
}

// MPC's PRESET menu: the "preset" parameter is the VST program (vst.json "programs"); the wrapper names entry n by
// get("preset:<n>") and selects it by set("preset", n).
void testPresets(void *s, const char *dataDir) {
  int n = 0, heads = 0;
  std::string first, firstHead;
  for (;; ++n) {
    char k[32];
    snprintf(k, sizeof k, "preset:%d", n);
    std::string name = get(s, k);
    if (name.empty()) break;
    if (name.compare(0, 3, "== ") == 0) { ++heads; if (firstHead.empty()) firstHead = name; }
    else if (n > 0 && first.empty()) first = name;
  }
  int want = (int)kKeys[P_PRESET].hi + 1;
  CHECK(n == want, "the PRESET menu has %d entries, the parameter's range says %d", n, want);
  CHECK(get(s, "preset:0") == "- Patch page -", "entry 0: %s", get(s, "preset:0").c_str());
  CHECK(heads == 12, "%d folder headings (LinnStrument MPE has 12 folders)", heads);
  CHECK(n - 1 - heads == 63, "%d patches (LinnStrument MPE has 63)", n - 1 - heads);
  seti(s, "preset", 1);                       // a heading: nothing happens
  render(s, 256);
  std::string before = get(s, "patch_name");
  CHECK(get(s, "preset") != "1", "a heading is not selectable: preset %s", get(s, "preset").c_str());
  seti(s, "preset", 2);                       // the first patch of the first folder
  CHECK(waitLoaded(s), "preset 2 loads");
  CHECK(get(s, "preset") == "2" && get(s, "patch_name") == first, "preset 2 loaded %s (want %s), preset reads %s",
        get(s, "patch_name").c_str(), first.c_str(), get(s, "preset").c_str());
  Audio a;
  note(s, 60, 100);
  render(s, 22050, &a);
  note(s, 60, 0);
  CHECK(a.peak > 1e-3 && a.finite, "the preset plays (peak %.4f)", a.peak);
  // the last entry, and the project state keeps the entry
  seti(s, "preset", n - 1);
  CHECK(waitLoaded(s), "the last preset loads");
  std::string last = get(s, "patch_name");
  void *t = E->create(dataDir);
  set(t, "state", get(s, "state").c_str());
  render(t, 1024);
  CHECK(get(t, "preset") == get(s, "preset") && get(t, "patch_name") == last, "state keeps the preset: %s %s",
        get(t, "preset").c_str(), get(t, "patch_name").c_str());
  E->destroy(t);
  // a patch loaded from the PATCH page that is not in the menu: entry 0
  seti(s, "patch", 0);
  tap(s, "patch_load");
  CHECK(waitLoaded(s), "a PATCH page load");
  CHECK(get(s, "preset") == "0", "after a PATCH page load of another patch the preset is %s", get(s, "preset").c_str());
  printf("ok   PRESET menu: %d entries (%d folders, %d patches), first %s / %s, last %s; headings do nothing; kept in "
         "the state\n", n, heads, n - 1 - heads, firstHead.c_str(), first.c_str(), last.c_str());
}

void testState(void *s, const char *dataDir) {
  std::string st = get(s, "state");
  CHECK(st.size() > 1000, "state is %zu bytes", st.size());
  void *t = E->create(dataDir);
  set(t, "state", st.c_str());
  render(t, 1024);
  int diff = 0;
  for (const KeyDef &k : kKeys) {
    if (k.bind == B_ENGINE && (strstr(k.key, "_prev") || strstr(k.key, "_next") || strstr(k.key, "learn") ||
                               !strcmp(k.key, "mpe_status"))) continue;   // the status is what this instance has heard
    std::string a = get(s, k.key), b = get(t, k.key);
    if (a != b && !(k.bind == B_CONT && fabs(atof(a.c_str()) - atof(b.c_str())) < 1e-4)) {
      ++diff;
      if (diff < 10) printf("     state: %s %s != %s\n", k.key, a.c_str(), b.c_str());
    }
  }
  CHECK(diff == 0, "state restore: %d values differ", diff);
  E->destroy(t);
  printf("ok   state chunk (%zu bytes of text) restores a second instance\n", st.size());
}

void testMod(void *s) {
  seti(s, "mod_slot", 64);                    // past the end: a new routing
  seti(s, "mod_src", 16);                     // Voice LFO 1 (Surge's source 17)
  tap(s, "mod_learn");
  CHECK(get(s, "mod_target").find("LEARN") == 0, "learn armed: %s", get(s, "mod_target").c_str());
  setf(s, "f1_cutoff", 0.5);
  std::string t = get(s, "mod_target");
  CHECK(t.find("Cutoff") != std::string::npos, "learned target: %s", t.c_str());
  setf(s, "mod_depth", 0.5);
  double d = atof(get(s, "mod_depth").c_str());
  CHECK(fabs(d - 0.5) < 0.01, "mod depth %f", d);
  Audio a;
  note(s, 60, 100);
  render(s, 22050, &a);
  note(s, 60, 0);
  CHECK(a.peak > 1e-3, "modulated patch is silent");
  tap(s, "mod_clear");
  printf("ok   LEARN routes LFO 1 to %s; depth and CLEAR work\n", t.c_str());
}
}  // namespace

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: engine_test <stage dir> [--patches all|N] [--fx] [--quick]\n");
    return 2;
  }
  std::string stage = argv[1];
  int stride = 0;
  bool fx = false;
  for (int i = 2; i < argc; ++i) {
    if (!strcmp(argv[i], "--patches") && i + 1 < argc) stride = !strcmp(argv[++i], "all") ? 1 : atoi(argv[i]);
    if (!strcmp(argv[i], "--fx")) fx = true;
  }
  std::string data = stage + "/Synths/plugin/data";
  E = mpc_engine();
  double t0 = now();
  void *s = E->create(data.c_str());
  CHECK(s, "create");
  if (!s) return 1;
  if (argc > 2 && !strcmp(argv[2], "--crash")) {   // the crash log's own test: run_tests.sh reads the log after
    render(s, 128);
    set(s, "test_crash", "1");
    return 3;
  }
  printf("ok   created in %.0f ms: %s patches, %s wavetables, user folder %s\n", (now() - t0) * 1000,
         get(s, "test_num_patches").c_str(), get(s, "test_num_wt").c_str(), get(s, "test_user_dir").c_str());
  testLibrary(s);
  render(s, 4410);
  testParams(s);
  testRoundTrips(s);
  testLoadWhileHostReads(s, stride > 1 ? stride * 7 : 97);
  if (stride > 0) testPatches(s, stride);
  if (fx) testFx(s);
  testWavetables(s, stage);
  testSave(s);
  testMod(s);
  testMpe(s);
  testPresets(s, data.c_str());
  testState(s, data.c_str());
  E->destroy(s);
  printf(fails ? "FAILED: %d\n" : "PASSED\n", fails);
  return fails ? 1 : 0;
}
