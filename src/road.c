#include "game.h"
#include <math.h>

void Road_Init(Road *r) {
    r->length = TOTAL_TRACK_LENGTH;
    r->checkpoints[0] = 3500.0f;
    r->checkpoints[1] = 7200.0f;
    r->checkpoints[2] = 11000.0f;
    r->checkpoints[3] = TOTAL_TRACK_LENGTH;
    r->next_checkpoint = 0;
    r->checkpoint_alert_timer = 0.0f;
}

float Road_GetCenter(const Road *r, float track_dist) {
    (void)r;
    // Smooth sinusoidal curves creating winding road segments
    // Combination of low frequency sweeping bends and medium curves
    float curve1 = sinf(track_dist * 0.0016f) * 48.0f;
    float curve2 = sinf(track_dist * 0.0006f) * 28.0f;
    float curve3 = cosf(track_dist * 0.0032f) * 14.0f;

    return (VIRTUAL_W / 2.0f) + curve1 + curve2 + curve3;
}

bool Road_IsOffroad(const Road *r, float x, float track_dist) {
    float center = Road_GetCenter(r, track_dist);
    float half_width = ROAD_WIDTH / 2.0f;
    return (x < center - half_width || x > center + half_width);
}

void Road_CheckPoints(Road *r, Player *p) {
    if (r->checkpoint_alert_timer > 0.0f) {
        r->checkpoint_alert_timer -= GetFrameTime();
    }

    if (r->next_checkpoint < NUM_CHECKPOINTS) {
        if (p->distance >= r->checkpoints[r->next_checkpoint]) {
            // Checkpoint reached!
            r->next_checkpoint++;
            r->checkpoint_alert_timer = 2.0f;
            // Refuel bonus
            p->fuel = fminf(p->fuel + 25.0f, 100.0f);
            p->score += 2500;
        }
    }
}
