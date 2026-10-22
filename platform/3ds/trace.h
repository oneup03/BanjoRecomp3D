// Included by the recompiled game functions (trace_mode, set in the 3DS
// build's copy of the config: Makefile's codegen): TRACE_ENTRY at each
// function's start, TRACE_RETURN before each return.
#ifndef BK_TRACE_H
#define BK_TRACE_H

// The preemption check (rt64-3ds's runtime patches, nmr 0025): when a host
// thread has queued an external event (a VI retrace, a task's completion, a
// timer), the runtime delivers it here and switches to a woken
// higher-priority game thread, as the console's interrupt would have.
// Without it a long stretch of game logic held the audio thread back until
// it next touched a message queue: on an Old 3DS the game handed over too
// few audio frames, and the music slowed.
extern unsigned int ultramodern_preempt_pending;
extern void ultramodern_preempt(unsigned char* rdram);
// The function running, for rt64-3ds's sampling profile of the game
// (settings.ini's dbg_profile=2, game_profile.h): set at entry, the
// caller's put back at return, so a sample names the function executing.
extern const char* volatile recomp3ds_trace_func;
#define TRACE_ENTRY() \
    if (*(volatile unsigned int*)&ultramodern_preempt_pending) { \
        ultramodern_preempt(rdram); \
    } \
    const char* const trace_prev_ = recomp3ds_trace_func; \
    recomp3ds_trace_func = __func__;
#define TRACE_RETURN() \
    recomp3ds_trace_func = trace_prev_;

#endif
