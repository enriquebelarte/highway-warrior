#include "game.h"
#include <stdlib.h>
#include <math.h>

static const float LANES[4] = { -58.0f, -20.0f, 20.0f, 58.0f };

void Traffic_Init(TrafficSystem *ts) {
    for (int i = 0; i < MAX_TRAFFIC; i++) {
        ts->cars[i].active = false;
    }
    ts->spawn_timer = 0.5f;
}

void Traffic_Spawn(TrafficSystem *ts, float player_dist, const Road *r) {
    (void)r;
    for (int i = 0; i < MAX_TRAFFIC; i++) {
        if (!ts->cars[i].active) {
            TrafficCar *c = &ts->cars[i];
            c->active = true;
            
            // Choose type: ~50% yellow, ~35% blue, ~15% fuel truck
            int roll = GetRandomValue(0, 99);
            if (roll < 48) {
                c->type = TRAFFIC_YELLOW;
                c->body_color = (Color){ 245, 215, 40, 255 }; // Bright Yellow
                c->speed = (float)GetRandomValue(95, 115);
                c->width = 14.0f;
                c->height = 24.0f;
            } else if (roll < 82) {
                c->type = TRAFFIC_BLUE;
                c->body_color = (Color){ 45, 120, 230, 255 }; // Blue lane changer
                c->speed = (float)GetRandomValue(110, 136);
                c->width = 14.0f;
                c->height = 24.0f;
            } else {
                c->type = TRAFFIC_FUEL_TRUCK;
                c->body_color = (Color){ 230, 40, 70, 255 };  // Red Fuel Truck
                c->speed = (float)GetRandomValue(100, 120);
                c->width = 16.0f;
                c->height = 32.0f;
            }

            int lane_idx = GetRandomValue(0, 3);
            c->lateral_offset = LANES[lane_idx];
            c->target_offset = c->lateral_offset;
            c->track_dist = player_dist + (float)GetRandomValue(380, 580);
            c->lane_timer = (float)GetRandomValue(15, 40) / 10.0f;
            c->indicator_on = false;
            c->blink_timer = 0.0f;
            break;
        }
    }
}

void Traffic_Update(TrafficSystem *ts, Player *p, const Road *r, float dt) {
    // Spawning frequency scales with player speed
    ts->spawn_timer -= dt;
    if (ts->spawn_timer <= 0.0f) {
        Traffic_Spawn(ts, p->distance, r);
        float next_delay = 0.7f + ((float)GetRandomValue(0, 10) / 10.0f);
        ts->spawn_timer = next_delay;
    }

    Rectangle player_rect = Player_GetBounds(p);

    for (int i = 0; i < MAX_TRAFFIC; i++) {
        TrafficCar *c = &ts->cars[i];
        if (!c->active) continue;

        // Advance traffic along track
        c->track_dist += (c->speed * 0.28f) * dt * SCROLL_SPEED_MULT;

        // Despawn if out of camera range
        float dist_diff = c->track_dist - p->distance;
        if (dist_diff < -150.0f || dist_diff > 850.0f) {
            c->active = false;
            continue;
        }

        // Screen position calculation
        // player is at y = 190. Scale world units to screen pixels
        float screen_y = p->y - dist_diff * 0.95f;
        float road_center = Road_GetCenter(r, c->track_dist);
        float screen_x = road_center + c->lateral_offset;

        // Blue car AI: lane changing behavior
        if (c->type == TRAFFIC_BLUE) {
            c->lane_timer -= dt;

            // Proximity check: swerve if player is approaching rapidly from behind
            bool player_close_behind = (dist_diff > 10.0f && dist_diff < 120.0f &&
                                        fabsf(screen_x - p->x) < 35.0f);

            if (player_close_behind && c->target_offset == c->lateral_offset) {
                // Swerve to adjacent lane!
                int current_idx = 0;
                for (int l = 0; l < 4; l++) {
                    if (fabsf(c->lateral_offset - LANES[l]) < 10.0f) {
                        current_idx = l;
                        break;
                    }
                }
                int new_idx = (current_idx <= 1) ? current_idx + 1 : current_idx - 1;
                c->target_offset = LANES[new_idx];
                c->indicator_on = true;
            } else if (c->lane_timer <= 0.0f) {
                // Regular lane wander
                int new_lane = GetRandomValue(0, 3);
                c->target_offset = LANES[new_lane];
                c->lane_timer = (float)GetRandomValue(25, 55) / 10.0f;
                c->indicator_on = true;
            }

            // Smooth lateral steering towards target lane
            if (c->lateral_offset != c->target_offset) {
                float dir = (c->target_offset > c->lateral_offset) ? 1.0f : -1.0f;
                c->lateral_offset += dir * 45.0f * dt;
                if ((dir > 0 && c->lateral_offset >= c->target_offset) ||
                    (dir < 0 && c->lateral_offset <= c->target_offset)) {
                    c->lateral_offset = c->target_offset;
                    c->indicator_on = false;
                }
            }

            // Blinker animation
            if (c->indicator_on) {
                c->blink_timer += dt;
            }
        }

        // Collision Detection with Player Car
        if (screen_y >= -40 && screen_y <= VIRTUAL_H + 40 && !p->is_crashed) {
            Rectangle car_rect = {
                screen_x - (c->width / 2.0f),
                screen_y - (c->height / 2.0f),
                c->width,
                c->height
            };

            if (CheckCollisionRecs(player_rect, car_rect)) {
                if (c->type == TRAFFIC_FUEL_TRUCK) {
                    // Fuel truck bonus!
                    p->fuel = fminf(p->fuel + 30.0f, 100.0f);
                    p->score += 1500;
                    c->active = false; // Claimed
                    p->counter_steer_flash = 0.35f;
                } else {
                    // Collision with civilian car
                    float speed_diff = p->speed - c->speed;

                    // Severe high-speed rear end impact
                    if (speed_diff > 68.0f && fabsf(p->x - screen_x) < 6.0f) {
                        Player_Crash(p);
                        c->active = false;
                    } else {
                        // Signature side-brush slide mechanic!
                        int slide_dir = (p->x < screen_x) ? -1 : 1;
                        Player_TriggerSlide(p, slide_dir);

                        // Also push traffic car slightly
                        c->lateral_offset += (slide_dir == -1 ? 12.0f : -12.0f);
                    }
                }
            }
        }
    }
}
