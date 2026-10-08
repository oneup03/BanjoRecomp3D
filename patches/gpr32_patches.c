#include "patches.h"

// @recomp 3DS: the game's only code that keeps 64-bit values in registers
// (hand-written MIPS III in core1: an 8-byte copy, an 8-byte fill and the
// checksum generator's shift), redone with 32-bit operations so the
// recompiled code can use 32-bit registers (rt64-3ds RECOMP_GPR32). The 3DS
// build stubs the originals (platform/3ds/Makefile). The desktop keeps them.
#ifdef BK_3DS

// Copies size / 8 doublewords (size a multiple of 8, never 0).
// volatile: the copy stays a copy (no call to memcpy).
RECOMP_PATCH void func_80253010(void *dest, void *src, s32 size) {
    volatile u32 *d = (volatile u32 *)dest;
    volatile u32 *s = (volatile u32 *)src;
    u32 n = (u32)size >> 3;
    do {
        d[0] = s[0];
        d[1] = s[1];
        d += 2;
        s += 2;
    } while (--n != 0);
}

// Fills size / 8 doublewords with the register holding value: value
// sign-extended to 64 bits (the game only clears with it).
RECOMP_PATCH void func_80253034(void *dest, s32 value, s32 size) {
    volatile u32 *d = (volatile u32 *)dest;
    u32 hi = value < 0 ? 0xFFFFFFFF : 0;
    u32 n = (u32)size >> 3;
    do {
        d[0] = hi;
        d[1] = (u32)value;
        d += 2;
    } while (--n != 0);
}

// The checksum's generator step on its 64-bit seed (big-endian: seed[0] the
// high word): the original's 33-bit shift register, x' = (x >> 1 | (x & 1)
// << 32) ^ ((x & 0xFFFFF) << 12), folded by its bits 20-31, its low word
// returned. The high word keeps only bit 32 (x's bit 0).
RECOMP_PATCH u32 func_8025C29C(u32 *seed) {
    u32 hi = seed[0], lo = seed[1];
    u32 t = ((lo >> 1) | (hi << 31)) ^ ((lo & 0xFFFFF) << 12);
    t ^= (t >> 20) & 0xFFF;
    seed[0] = lo & 1;
    seed[1] = t;
    return t;
}

#endif
