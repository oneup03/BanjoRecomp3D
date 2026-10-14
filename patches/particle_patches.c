// Particle emission scaled down (the 3DS Game page's Particles row: Full,
// Half, Quarter; an Old 3DS starts at Quarter). Each particle is a draw of its
// own, with its own sprite frame and colour, so the renderer's cost per
// particle is a draw's: in a capture of a jiggy's sparkles, 225 of the
// frame's 360 draws were sparkles. The counts an emitter is asked for are
// divided (never below one, or two for a line) and its spawn interval is
// multiplied, so timed emitters put out as many fewer. Nothing is switched
// off: effects end when their particles run out, and the emitters'
// capacities are untouched.
#ifdef BK_3DS
#include "patches.h"
#include "patch_helpers.h"
#include "functions.h"
#include "rand.h"

DECLARE_FUNC(s32, recomp_get_particle_divisor);

// particle.c's own (structs.h leaves the struct incomplete).
typedef struct particle {
    f32 acceleration[3];
    f32 fade;
    f32 frame;
    f32 framerate;
    f32 position[3];
    f32 rotation[3];
    f32 scale;
    f32 initialSize_34;
    f32 finalSizeDiff;
    f32 angluar_velocity[3];
    f32 age_48;
    f32 lifetime_4C;
    f32 velocity_50[3];
    u8 unk5C;
} Particle;

void __particleEmitter_initParticle(ParticleEmitter *this, Particle *particle);

// n particles asked for, divided by the setting's divisor, never below floor.
static s32 scaled_count(s32 n, s32 floor) {
    s32 d = recomp_get_particle_divisor();
    s32 s;
    if (d <= 1 || n <= floor) return n;
    s = (n + d - 1) / d;
    return s < floor ? floor : s;
}

// @recomp Patched: the count divided.
RECOMP_PATCH void particleEmitter_emitInVolume(ParticleEmitter *this, f32 position_min[3], f32 position_max[3], s32 count){
    count = scaled_count(count, 1);
    for(count; count > 0; count--){
        if(this->pList_end_128 < this->pList_capacity_12C){
            __particleEmitter_initParticle(this, this->pList_end_128);
            this->pList_end_128->position[0] = randf2(position_min[0], position_max[0]);
            this->pList_end_128->position[1] = randf2(position_min[1], position_max[1]);
            this->pList_end_128->position[2] = randf2(position_min[2], position_max[2]);
            this->pList_end_128++;
        }
    }
}

// @recomp Patched: the count divided (a line keeps both ends).
RECOMP_PATCH void particleEmitter_emitUniformLine(ParticleEmitter *this, f32 start[3], f32 end[3], s32 count) {
    f32 d_position[3];
    s32 i;

    count = scaled_count(count, 2);
    count -= 1;
    d_position[0] = end[0] - start[0];
    d_position[1] = end[1] - start[1];
    d_position[2] = end[2] - start[2];
    for(i = 0; i <= count; i++){
        if (this->pList_end_128 < this->pList_capacity_12C) {
            __particleEmitter_initParticle(this, this->pList_end_128);
            this->pList_end_128->position[0] = start[0] + d_position[0]*((f32)i / count);
            this->pList_end_128->position[1] = start[1] + d_position[1]*((f32)i / count);
            this->pList_end_128->position[2] = start[2] + d_position[2]*((f32)i / count);
            this->pList_end_128++;
        }
    }
}

// @recomp Patched: the count divided.
RECOMP_PATCH void particleEmitter_emitN(ParticleEmitter *this, int n){
    n = scaled_count(n, 1);
    for(n; n > 0; n--){
        if(this->pList_end_128 < this->pList_capacity_12C){
            __particleEmitter_initParticle(this, this->pList_end_128++);
        }
    }
}

// @recomp Patched: the interval multiplied, so a timed emitter spawns as
// many fewer.
RECOMP_PATCH void particleEmitter_setSpawnIntervalRange(ParticleEmitter *this, f32 min, f32 max){
    s32 d = recomp_get_particle_divisor();
    if (d > 1) {
        min *= (f32)d;
        max *= (f32)d;
    }
    this->spawnIntervalRange_D4_min = min;
    this->spawnIntervalRange_D4_max = max;
    if(0.0f == this->particleSpawnTimer_24 || max < this->particleSpawnTimer_24)
        this->particleSpawnTimer_24 = randf2(this->spawnIntervalRange_D4_min, this->spawnIntervalRange_D4_max);
}
#endif
