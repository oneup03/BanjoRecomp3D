// Included by the recompiled game functions (trace_mode, set in the 3DS
// build's copy of the config: Makefile's codegen): TRACE_ENTRY at each
// function's start, TRACE_RETURN before each return. Simple leaf functions
// (no call, no loop, no jump table) have neither (trace_skip_simple_leaves):
// they finish in a few instructions, and the check would be much of their
// code.
#ifndef BK_TRACE_H
#define BK_TRACE_H

// The preemption check (rt64-3ds's runtime patches, nmr 0025): when a host
// thread has queued an external event (a VI retrace, a task's completion, a
// timer), the runtime delivers it here and switches to a woken
// higher-priority game thread, as the console's interrupt would have.
// Without it a long stretch of game logic held the audio thread back until
// it next touched a message queue: on an Old 3DS the game handed over too
// few audio frames, and the music slowed. rt64_3ds_preempt.h's form: the
// flag read from below RDRAM, with no address to load and no call the
// compiler sees. The preemptions cost an Old 3DS frame rate, so they happen
// only with the Audio row's High priority (rt64-3ds's preempt_gate).
#include "rt64_3ds_preempt.h"
#define TRACE_PREEMPT_CHECK() RT64_3DS_PREEMPT_CHECK(rdram)

#ifdef BK_GAME_PROFILE
// The function running, for rt64-3ds's sampling profile of the game
// (settings.ini's dbg_profile=2, game_profile.h): set at entry, the
// caller's put back at return, so a sample names the function executing.
// Only in a build with BK_GAME_PROFILE=ON (CMakeLists.txt): in every call
// it made the game's code 8% bigger and cost two stores a call.
extern const char* volatile recomp3ds_trace_func;
#define TRACE_ENTRY() \
    TRACE_PREEMPT_CHECK() \
    const char* const trace_prev_ = recomp3ds_trace_func; \
    recomp3ds_trace_func = __func__;
#define TRACE_RETURN() \
    recomp3ds_trace_func = trace_prev_;
#else
#define TRACE_ENTRY() TRACE_PREEMPT_CHECK()
#define TRACE_RETURN()
#endif

#endif
