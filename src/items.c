#include "game.h"
#include <stdlib.h>
#include <math.h>

void Items_Init(ItemSystem *is) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        is->items[i].active = false;
    }
    is->spawn_timer = 2.0f;
}

void Items_Spawn(ItemSystem *is, float player_dist, const Road *r) {
    (void)r;
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!is->items[i].active) {
            Item *item = &is->items[i];
            item->active = true;

            int roll = GetRandomValue(0, 99);
            if (roll < 45) {
                item->type = ITEM_OIL_SLICK;
                item->width = 18.0f;
                item->height = 14.0f;
            } else if (roll < 75) {
                item->type = ITEM_FUEL_CAN;
                item->width = 12.0f;
                item->height = 12.0f;
            } else {
                item->type = ITEM_ROADWORK;
                item->width = 22.0f;
                item->height = 12.0f;
            }

            // Random position within road lanes
            float offset = (float)GetRandomValue(-65, 65);
            item->lateral_offset = offset;
            item->track_dist = player_dist + (float)GetRandomValue(450, 700);
            break;
        }
    }
}

void Items_Update(ItemSystem *is, Player *p, const Road *r, float dt) {
    is->spawn_timer -= dt;
    if (is->spawn_timer <= 0.0f) {
        Items_Spawn(is, p->distance, r);
        is->spawn_timer = (float)GetRandomValue(20, 45) / 10.0f;
    }

    Rectangle player_rect = Player_GetBounds(p);

    for (int i = 0; i < MAX_ITEMS; i++) {
        Item *item = &is->items[i];
        if (!item->active) continue;

        float dist_diff = item->track_dist - p->distance;

        // Despawn when passed far behind or ahead
        if (dist_diff < -100.0f || dist_diff > 900.0f) {
            item->active = false;
            continue;
        }

        float screen_y = p->y - dist_diff * 0.95f;
        float road_center = Road_GetCenter(r, item->track_dist);
        float screen_x = road_center + item->lateral_offset;

        if (screen_y >= -30 && screen_y <= VIRTUAL_H + 30 && !p->is_crashed) {
            Rectangle item_rect = {
                screen_x - (item->width / 2.0f),
                screen_y - (item->height / 2.0f),
                item->width,
                item->height
            };

            if (CheckCollisionRecs(player_rect, item_rect)) {
                if (item->type == ITEM_OIL_SLICK) {
                    // Triggers violent skid requiring counter-steering
                    int skid_dir = (GetRandomValue(0, 1) == 0) ? -1 : 1;
                    Player_TriggerSlide(p, skid_dir);
                } else if (item->type == ITEM_FUEL_CAN) {
                    // Refuel bonus
                    p->fuel = fminf(p->fuel + 18.0f, 100.0f);
                    p->score += 800;
                    item->active = false;
                } else if (item->type == ITEM_ROADWORK) {
                    // Hard obstacle -> Crash!
                    Player_Crash(p);
                    item->active = false;
                }
            }
        }
    }
}
