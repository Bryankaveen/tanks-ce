#include <stdbool.h>
#include <stdint.h>
#include <tice.h>
#include <math.h>

#include <keypadc.h>
#include "util.h"
#include "graphics.h"

int24_t sin_table[TRIG_PRECISION / 4];

void gen_lookups(void) {
    for(uint24_t i = 0; i < TRIG_PRECISION / 4; i++) {
        sin_table[i] = sin(i * M_PI / (TRIG_PRECISION / 2)) * TRIG_SCALE;
    }
}

int24_t fast_sin(angle_t angle) {
    angle >>= 24 - TRIG_PRECISION_BITS;
    if(angle < TRIG_PRECISION / 4) return sin_table[angle];
    if(angle == TRIG_PRECISION / 4) return TRIG_SCALE;
    if(angle < TRIG_PRECISION / 2) return sin_table[TRIG_PRECISION / 2 - angle];
    if(angle < TRIG_PRECISION * 3 / 4) return -sin_table[angle - TRIG_PRECISION / 2];
    if(angle == TRIG_PRECISION * 3 / 4) return -TRIG_SCALE;
    return -sin_table[TRIG_PRECISION - angle];
}

#define Q_FROM_FLOAT(f) ((int24_t)((float)(f) * (float)(1l << 23l)))

inline int24_t qmul(int24_t a, int24_t b) {
    return (a >> 12) * (b >> 11);
}

inline int24_t qdiv(int24_t a, int24_t b) {
    int24_t a1 = a * (1 << 7);
    int24_t d = (a1 / b);
    return d * (1 << 16);
}

inline int24_t nabs(int24_t x) {
    if(x > 0) x = -x;
    return x;
}

// Credit: https://geekshavefeelings.com/posts/fixed-point-atan2
uint24_t fast_atan2(int24_t y, int24_t x) {
    if(x == y) { // x/y or y/x would return -1 since 1 isn't representable
        if(y > 0) { // 1/8
            return DEGREES_TO_ANGLE(360 / 8);
        } else if(y < 0) { // 5/8
            return DEGREES_TO_ANGLE(360 / 8 * 5);
        } else { // x = y = 0
            return 0;
        }
    }
    int24_t nabs_y = nabs(y);
    int24_t nabs_x = nabs(x);
    if(nabs_x < nabs_y) { // octants 1, 4, 5, 8
        int24_t y_over_x = qdiv(y, x);
        int24_t correction = qmul(Q_FROM_FLOAT(0.273 * M_1_PI), nabs(y_over_x));
        int24_t unrotated = qmul(Q_FROM_FLOAT(0.25 + 0.273 * M_1_PI) + correction, y_over_x);
        if(x > 0) { // octants 1, 8
            return unrotated;
        } else { // octants 4, 5
            return DEGREES_TO_ANGLE(180) + unrotated;
        }
    } else { // octants 2, 3, 6, 7
        int24_t x_over_y = qdiv(x, y);
        int24_t correction = qmul(Q_FROM_FLOAT(0.273 * M_1_PI), nabs(x_over_y));
        int24_t unrotated = qmul(Q_FROM_FLOAT(0.25 + 0.273 * M_1_PI) + correction, x_over_y);
        if(y > 0) { // octants 2, 3
            return DEGREES_TO_ANGLE(90) - unrotated;
        } else { // octants 6, 7
            return DEGREES_TO_ANGLE(270) - unrotated;
        }
    }
}

void init_timer(void) {
    timer_Control &= ~TIMER1_ENABLE;
    timer_1_ReloadValue = timer_1_Counter = 32768 / TARGET_FPS;
    timer_Control |= TIMER1_ENABLE | TIMER1_32K | TIMER1_0INT | TIMER1_DOWN;
}

void limit_framerate(void) {
    while(!(timer_IntStatus & TIMER1_RELOADED));
    timer_IntAcknowledge |= TIMER1_RELOADED;
}

void wait_ms_or_keypress(uint24_t ms) {
    timer_Control &= ~TIMER1_ENABLE;
    timer_1_ReloadValue = timer_1_Counter = 33 * ms;
    timer_Control |= TIMER1_ENABLE | TIMER1_32K | TIMER1_0INT | TIMER1_DOWN;

    while(true) {
        if(timer_IntStatus & TIMER1_RELOADED) {
            timer_IntAcknowledge = TIMER1_RELOADED;
            break;
        }
        kb_Scan();
        if(kb_Data[1] & kb_2nd || kb_Data[1] & kb_Del || kb_Data[6] & kb_Clear) {
            while(kb_Data[1] || kb_Data[6]) kb_Scan();
            break;
        }
    }
}
