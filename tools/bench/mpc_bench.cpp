// SPDX-License-Identifier: GPL-3.0-or-later
// mpc_bench: loads a VST2 plugin the way the MPC does and measures what it costs the audio thread, on the device.
//
//   mpc_bench <plugin.so> [--seconds 3] [--every 25 | --programs 2,50,300] [--notes 8] [--state-hz 4] [--fast]
//
// For each chosen program (patch) it selects the program, plays a chord, and times every processReplacing call
// against the block's real-time budget (128 frames at 44.1 kHz = 2.9 ms). A second thread asks for the plugin's
// state (effGetChunk) several times a second while it plays, which is what the MPC's autosave does, and the longest
// block that overlapped a state call is reported. Nothing is written but this report; no audio device is opened.
//
// Cross-compile: tools/zig64/zcxx -std=c++17 -O2 -o mpc_bench tools/bench/mpc_bench.cpp -ldl -lpthread
// Run it on the MPC (ssh) with the MPC otherwise quiet: it shares the CPU with the MPC application.
#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <algorithm>
#include <atomic>
#include <string>
#include <vector>

// The VST 2.4 binary interface, as much of it as a host needs.
struct AEffect;
typedef intptr_t (*Master)(AEffect *, int32_t, int32_t, intptr_t, void *, float);
typedef intptr_t (*Dispatch)(AEffect *, int32_t, int32_t, intptr_t, void *, float);
typedef void (*Process)(AEffect *, float **, float **, int32_t);
struct AEffect {
  int32_t magic;
  Dispatch dispatcher;
  Process process;
  void (*setParameter)(AEffect *, int32_t, float);
  float (*getParameter)(AEffect *, int32_t);
  int32_t numPrograms, numParams, numInputs, numOutputs, flags;
  intptr_t resvd1, resvd2;
  int32_t initialDelay, realQualities, offQualities;
  float ioRatio;
  void *object, *user;
  int32_t uniqueID, version;
  Process processReplacing;
  void (*processDoubleReplacing)(AEffect *, double **, double **, int32_t);
  char future[56];
};
struct MidiEvent {
  int32_t type, byteSize, deltaFrames, flags, noteLength, noteOffset;
  char midiData[4], detune, noteOffVelocity, reserved1, reserved2;
};
struct Events {
  int32_t numEvents;
  intptr_t reserved;
  MidiEvent *events[64];
};
enum { effOpen = 0, effClose = 1, effSetProgram = 2, effSetSampleRate = 10, effSetBlockSize = 11, effMainsChanged = 12,
       effGetChunk = 23, effProcessEvents = 25, effGetProgramNameIndexed = 29, effStartProcess = 71 };

static const int kRate = 44100, kBlock = 128;
static const double kBudgetUs = kBlock * 1e6 / kRate;

static double nowUs() {
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec * 1e6 + t.tv_nsec * 1e-3;
}
static intptr_t host(AEffect *, int32_t op, int32_t, intptr_t, void *, float) {
  switch (op) {
    case 1: return 2400;               // audioMasterVersion
    case 16: return kRate;             // audioMasterGetSampleRate
    case 17: return kBlock;            // audioMasterGetBlockSize
    default: return 0;
  }
}

struct StateProbe {                    // the second thread: what autosave does
  AEffect *e;
  double hz;
  std::atomic<bool> stop{false};
  std::vector<std::pair<double, double>> calls;   // start, end (us)
  static void *run(void *p) {
    StateProbe *s = (StateProbe *)p;
    while (!s->stop) {
      double t0 = nowUs();
      void *chunk = nullptr;
      s->e->dispatcher(s->e, effGetChunk, 0, 0, &chunk, 0);
      s->calls.push_back({t0, nowUs()});
      timespec ts = {0, (long)(1e9 / s->hz)};
      nanosleep(&ts, nullptr);
    }
    return nullptr;
  }
};

static void sendNotes(AEffect *e, const std::vector<int> &notes, bool on) {
  Events ev;
  memset(&ev, 0, sizeof ev);
  std::vector<MidiEvent> m(notes.size());
  for (size_t i = 0; i < notes.size(); ++i) {
    memset(&m[i], 0, sizeof m[i]);
    m[i].type = 1;
    m[i].byteSize = sizeof(MidiEvent);
    m[i].midiData[0] = (char)(on ? 0x90 : 0x80);
    m[i].midiData[1] = (char)notes[i];
    m[i].midiData[2] = (char)(on ? 100 : 64);
    ev.events[i] = &m[i];
  }
  ev.numEvents = (int32_t)notes.size();
  e->dispatcher(e, effProcessEvents, 0, 0, &ev, 0);
}

struct Block { double start, us; };

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: mpc_bench <plugin.so> [--seconds 3] [--every 25 | --programs a,b,c] [--notes 8] "
                    "[--state-hz 4] [--fast]\n");
    return 2;
  }
  double seconds = 3, stateHz = 4;
  int every = 25, nnotes = 8;
  bool fast = false;
  std::vector<int> programs;
  for (int i = 2; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "--seconds" && i + 1 < argc) seconds = atof(argv[++i]);
    else if (a == "--every" && i + 1 < argc) every = std::max(1, atoi(argv[++i]));
    else if (a == "--notes" && i + 1 < argc) nnotes = std::max(1, std::min(16, atoi(argv[++i])));
    else if (a == "--state-hz" && i + 1 < argc) stateHz = atof(argv[++i]);
    else if (a == "--fast") fast = true;
    else if (a == "--programs" && i + 1 < argc) {
      char *s = strdup(argv[++i]);
      for (char *t = strtok(s, ","); t; t = strtok(nullptr, ",")) programs.push_back(atoi(t));
    }
  }
  void *h = dlopen(argv[1], RTLD_NOW);
  if (!h) { fprintf(stderr, "dlopen: %s\n", dlerror()); return 1; }
  typedef AEffect *(*Main)(Master);
  Main vmain = (Main)dlsym(h, "VSTPluginMain");
  if (!vmain) { fprintf(stderr, "no VSTPluginMain\n"); return 1; }
  AEffect *e = vmain(host);
  if (!e || e->magic != 0x56737450) { fprintf(stderr, "not a VST2 plugin\n"); return 1; }
  e->dispatcher(e, effOpen, 0, 0, nullptr, 0);
  e->dispatcher(e, effSetSampleRate, 0, 0, nullptr, (float)kRate);
  e->dispatcher(e, effSetBlockSize, 0, kBlock, nullptr, 0);
  e->dispatcher(e, effMainsChanged, 0, 1, nullptr, 0);
  e->dispatcher(e, effStartProcess, 0, 0, nullptr, 0);

  std::vector<std::string> names;
  for (int i = 0; i < e->numPrograms; ++i) {
    char n[64] = {0};
    e->dispatcher(e, effGetProgramNameIndexed, i, 0, n, 0);
    names.push_back(n);
  }
  if (programs.empty()) {
    for (int i = 1; i < e->numPrograms; i += every)
      if (names[i].compare(0, 3, "== ") != 0) programs.push_back(i);
    if (programs.empty()) programs.push_back(0);
  }
  printf("%s: %d programs, %d outputs; block %d frames = %.0f us; %zu patch(es), %d-note chord, %.1f s each, "
         "state call %.1f/s%s\n", argv[1], e->numPrograms, e->numOutputs, kBlock, kBudgetUs, programs.size(), nnotes,
         seconds, stateHz, fast ? ", unpaced" : ", paced in real time");
  printf("%-4s %-24s %7s %7s %7s %7s  %9s %9s %8s\n", "no.", "patch", "avg %", "p99 %", "max %", "overrun",
         "state ms", "blk@state", "blk else");

  std::vector<float> L(kBlock), R(kBlock);
  float *outs[2] = {L.data(), R.data()};
  float *ins[2] = {L.data(), R.data()};
  std::vector<int> chord;
  static const int kChord[] = {36, 43, 48, 52, 55, 60, 64, 67, 72, 76, 79, 84, 40, 47, 59, 71};
  for (int i = 0; i < nnotes; ++i) chord.push_back(kChord[i]);
  double worstAvg = 0;
  std::string worstName;
  for (int k : programs) {
    e->dispatcher(e, effSetProgram, 0, k, nullptr, 0);
    for (int b = 0; b < 100; ++b) {                // the patch loads on the plugin's own thread: let it finish
      e->processReplacing(e, ins, outs, kBlock);
      timespec ts = {0, (long)(kBudgetUs * 1000)};
      nanosleep(&ts, nullptr);
    }
    sendNotes(e, chord, true);
    StateProbe probe;
    probe.e = e;
    probe.hz = stateHz;
    pthread_t th;
    bool probing = stateHz > 0 && pthread_create(&th, nullptr, StateProbe::run, &probe) == 0;
    std::vector<Block> blocks;
    int total = (int)(seconds * kRate / kBlock);
    blocks.reserve(total);
    double t0 = nowUs(), deadline = t0;
    int overruns = 0;
    for (int b = 0; b < total; ++b) {
      double s = nowUs();
      e->processReplacing(e, ins, outs, kBlock);
      double d = nowUs() - s;
      blocks.push_back({s, d});
      if (d > kBudgetUs) ++overruns;
      deadline += kBudgetUs;
      if (!fast) {
        double wait = deadline - nowUs();
        if (wait > 0) { timespec ts = {0, (long)(wait * 1000)}; nanosleep(&ts, nullptr); }
        else deadline = nowUs();                   // late: do not try to catch up
      }
    }
    if (probing) { probe.stop = true; pthread_join(th, nullptr); }
    sendNotes(e, chord, false);
    for (int b = 0; b < 60; ++b) e->processReplacing(e, ins, outs, kBlock);
    std::vector<double> us;
    double sum = 0;
    for (auto &b : blocks) us.push_back(b.us), sum += b.us;
    std::sort(us.begin(), us.end());
    double avg = sum / us.size(), p99 = us[(size_t)(us.size() * 0.99)], mx = us.back();
    double stateMs = 0, atState = 0, elseMax = 0;
    for (auto &c : probe.calls) stateMs = std::max(stateMs, (c.second - c.first) / 1000);
    for (auto &b : blocks) {
      bool over = false;
      for (auto &c : probe.calls) over = over || (b.start < c.second && b.start + b.us > c.first);
      (over ? atState : elseMax) = std::max(over ? atState : elseMax, b.us);
    }
    std::string nm = names[k].substr(0, 24);
    printf("%-4d %-24s %6.1f%% %6.1f%% %6.1f%% %7d  %9.2f %8.1f%% %7.1f%%\n", k, nm.c_str(), avg / kBudgetUs * 100,
           p99 / kBudgetUs * 100, mx / kBudgetUs * 100, overruns, stateMs, atState / kBudgetUs * 100,
           elseMax / kBudgetUs * 100);
    if (avg > worstAvg) worstAvg = avg, worstName = nm;
  }
  printf("\nheaviest patch: %s at %.1f%% of a block on average. Instances add up: N copies use about N times that.\n"
         "overrun = blocks that took longer than their budget (an audible dropout when the MPC has no slack).\n"
         "state ms = the longest state call; blk@state = the longest block that overlapped one; blk else = the longest "
         "block that did not.\n", worstName.c_str(), worstAvg / kBudgetUs * 100);
  e->dispatcher(e, effMainsChanged, 0, 0, nullptr, 0);
  e->dispatcher(e, effClose, 0, 0, nullptr, 0);
  return 0;
}
