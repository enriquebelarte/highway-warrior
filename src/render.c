#include "game.h"
#include "rlgl.h"
#include <math.h>

void Render_Init(RenderSystem *rs) {
    rs->target = LoadRenderTexture(VIRTUAL_W, VIRTUAL_H);
    SetTextureFilter(rs->target.texture, TEXTURE_FILTER_POINT);
    rs->crt_enabled = true;
    rs->scanline_alpha = 0.22f;
}

static void DrawCarSprite(float x, float y, float width, float height, Color body, float angle, bool is_truck, bool blink_indicator) {
    rlPushMatrix();
    rlTranslatef(x, y, 0.0f);
    rlRotatef(angle, 0.0f, 0.0f, 1.0f);

    float half_w = width / 2.0f;
    float half_h = height / 2.0f;

    // Shadow underneath
    DrawRectangle((int)(-half_w - 1), (int)(-half_h + 2), (int)(width + 2), (int)(height + 1), (Color){ 10, 10, 10, 120 });

    // Wheels (4 black corners)
    DrawRectangle((int)(-half_w - 2), (int)(-half_h + 3), 2, 5, BLACK);
    DrawRectangle((int)(half_w),      (int)(-half_h + 3), 2, 5, BLACK);
    DrawRectangle((int)(-half_w - 2), (int)(half_h - 7),  2, 5, BLACK);
    DrawRectangle((int)(half_w),      (int)(half_h - 7),  2, 5, BLACK);

    // Main Car Body
    DrawRectangle((int)(-half_w), (int)(-half_h), (int)width, (int)height, body);

    if (is_truck) {
        // Truck cab + tank trailer details
        DrawRectangle((int)(-half_w + 2), (int)(-half_h + 2), (int)(width - 4), 6, SKYBLUE); // Cab glass
        DrawRectangle((int)(-half_w + 1), (int)(-half_h + 10), (int)(width - 2), (int)(height - 12), WHITE); // Tanker
        DrawRectangle((int)(-half_w + 3), (int)(-half_h + 14), (int)(width - 6), 4, RED); // Red logo
        DrawRectangle((int)(-half_w + 1), (int)(half_h - 2), 3, 2, RED); // Tail lights
        DrawRectangle((int)(half_w - 4),  (int)(half_h - 2), 3, 2, RED);
    } else {
        // Roof highlight
        Color roof_color = (Color){ (unsigned char)(body.r * 0.8f), (unsigned char)(body.g * 0.8f), (unsigned char)(body.b * 0.8f), 255 };
        DrawRectangle((int)(-half_w + 2), (int)(-half_h + 4), (int)(width - 4), (int)(height - 9), roof_color);

        // Windshield (front)
        DrawRectangle((int)(-half_w + 2), (int)(-half_h + 4), (int)(width - 4), 4, (Color){ 160, 230, 255, 255 });
        // Rear window
        DrawRectangle((int)(-half_w + 2), (int)(half_h - 6), (int)(width - 4), 3, (Color){ 100, 180, 220, 255 });

        // Headlights & Tail lights
        DrawRectangle((int)(-half_w + 1), (int)(-half_h), 3, 2, YELLOW);
        DrawRectangle((int)(half_w - 4),  (int)(-half_h), 3, 2, YELLOW);
        DrawRectangle((int)(-half_w + 1), (int)(half_h - 2), 3, 2, RED);
        DrawRectangle((int)(half_w - 4),  (int)(half_h - 2), 3, 2, RED);
    }

    // Lane change blinkers
    if (blink_indicator) {
        DrawCircle((int)(half_w + 1), (int)(-half_h + 3), 2, ORANGE);
        DrawCircle((int)(half_w + 1), (int)(half_h - 3),  2, ORANGE);
    }

    rlPopMatrix();
}

static void DrawExplosion(float x, float y, float timer) {
    float progress = (1.4f - timer) / 1.4f; // 0.0 to 1.0
    float radius = progress * 26.0f;

    // Expanding fire rings
    DrawCircle((int)x, (int)y, radius, RED);
    DrawCircle((int)x, (int)y, radius * 0.7f, ORANGE);
    DrawCircle((int)x, (int)y, radius * 0.4f, YELLOW);
    DrawCircle((int)x, (int)y, radius * 0.2f, WHITE);

    // Particle debris sparks
    for (int i = 0; i < 8; i++) {
        float ang = (float)i * (2.0f * PI / 8.0f);
        float dist = progress * 35.0f;
        int px = (int)(x + cosf(ang) * dist);
        int py = (int)(y + sinf(ang) * dist);
        DrawRectangle(px - 1, py - 1, 3, 3, (i % 2 == 0) ? YELLOW : ORANGE);
    }
}

void Render_Game(const RenderSystem *rs, const GameContext *ctx) {
    BeginTextureMode(rs->target);
        const Player *p = &ctx->player;
        const Road *r = &ctx->road;

        // Draw Road and Biome Side-Terrain by vertical scanlines (from y = 0 to y = VIRTUAL_H)
        for (int y = 0; y < VIRTUAL_H; y++) {
            // Distance corresponding to screen Y
            float line_dist = p->distance + (p->y - y) * 1.05f;
            float center_x = Road_GetCenter(r, line_dist);
            float half_w = ROAD_WIDTH / 2.0f;

            int left_barrier_x  = (int)(center_x - half_w - ROAD_CURB_WIDTH - 3);
            int right_barrier_x = (int)(center_x + half_w + ROAD_CURB_WIDTH + 3);

            // ==========================================
            // Biome / Zone Side Terrain
            // ==========================================
            if (line_dist < 3500.0f) {
                // ZONE 0: Tropical Coastal Beach
                // Left Side: Tropical lawn & sand edge
                int left_sand_x = left_barrier_x - 20;
                if (left_sand_x > 0) {
                    DrawRectangle(0, y, left_sand_x, 1, (((int)(line_dist * 0.08f)) % 2 == 0) ? (Color){ 58, 162, 62, 255 } : (Color){ 50, 150, 55, 255 });
                }
                DrawRectangle((left_sand_x > 0 ? left_sand_x : 0), y, (left_sand_x > 0 ? 20 : left_barrier_x), 1, (Color){ 236, 208, 142, 255 });

                // Right Side: Sand beach & sparkling ocean waves
                int beach_w = 36;
                DrawRectangle(right_barrier_x, y, beach_w, 1, (Color){ 240, 214, 148, 255 });

                // Ocean water
                int ocean_start_x = right_barrier_x + beach_w;
                if (ocean_start_x < VIRTUAL_W) {
                    // Animated waves
                    float wave = sinf(y * 0.16f + line_dist * 0.04f + (float)GetTime() * 4.5f);
                    Color water_col = (Color){ 28, 122, 196, 255 };
                    if (wave > 0.82f) {
                        water_col = (Color){ 245, 252, 255, 255 }; // White seafoam crest
                    } else if (wave > 0.60f) {
                        water_col = (Color){ 80, 215, 255, 255 };  // Turquoise shimmer
                    }
                    DrawRectangle(ocean_start_x, y, VIRTUAL_W - ocean_start_x, 1, water_col);
                }
            } else if (line_dist < 7200.0f) {
                // ZONE 1: Lush Forest & Woodland
                Color forest_col = (((int)(line_dist * 0.06f)) % 2 == 0) ? (Color){ 26, 114, 38, 255 } : (Color){ 20, 98, 30, 255 };
                Color dirt_col = (Color){ 105, 72, 42, 255 };

                // Left side
                if (left_barrier_x > 0) {
                    int dirt_w = 8;
                    int lawn_w = left_barrier_x - dirt_w;
                    if (lawn_w > 0) DrawRectangle(0, y, lawn_w, 1, forest_col);
                    DrawRectangle(lawn_w > 0 ? lawn_w : 0, y, (lawn_w > 0 ? dirt_w : left_barrier_x), 1, dirt_col);
                }

                // Right side
                if (right_barrier_x < VIRTUAL_W) {
                    DrawRectangle(right_barrier_x, y, 8, 1, dirt_col);
                    DrawRectangle(right_barrier_x + 8, y, VIRTUAL_W - (right_barrier_x + 8), 1, forest_col);
                }
            } else if (line_dist < 11000.0f) {
                // ZONE 2: Rocky Mountain Pass & Canyon
                Color rock_col = (((int)(line_dist * 0.05f)) % 2 == 0) ? (Color){ 95, 86, 78, 255 } : (Color){ 85, 78, 72, 255 };
                Color gravel_col = (Color){ 122, 120, 126, 255 };

                // Left side
                if (left_barrier_x > 0) {
                    int g_w = 8;
                    int r_w = left_barrier_x - g_w;
                    if (r_w > 0) DrawRectangle(0, y, r_w, 1, rock_col);
                    DrawRectangle(r_w > 0 ? r_w : 0, y, (r_w > 0 ? g_w : left_barrier_x), 1, gravel_col);
                }

                // Right side
                if (right_barrier_x < VIRTUAL_W) {
                    DrawRectangle(right_barrier_x, y, 8, 1, gravel_col);
                    DrawRectangle(right_barrier_x + 8, y, VIRTUAL_W - (right_barrier_x + 8), 1, rock_col);
                }
            } else {
                // ZONE 3: City & Sunset Boulevard (>= 11000m)
                Color city_ground = (Color){ 28, 25, 38, 255 };
                Color sidewalk_col = (((int)(line_dist * 0.12f)) % 10 == 0) ? (Color){ 52, 54, 60, 255 } : (Color){ 76, 78, 86, 255 };

                // Left side
                if (left_barrier_x > 0) {
                    int sw_w = 14;
                    int g_w = left_barrier_x - sw_w;
                    if (g_w > 0) DrawRectangle(0, y, g_w, 1, city_ground);
                    DrawRectangle(g_w > 0 ? g_w : 0, y, (g_w > 0 ? sw_w : left_barrier_x), 1, sidewalk_col);
                }

                // Right side
                if (right_barrier_x < VIRTUAL_W) {
                    DrawRectangle(right_barrier_x, y, 14, 1, sidewalk_col);
                    DrawRectangle(right_barrier_x + 14, y, VIRTUAL_W - (right_barrier_x + 14), 1, city_ground);
                }
            }

            // ==========================================
            // Road Surface & Curbs
            // ==========================================
            // Road surface (Asphalt)
            DrawRectangle((int)(center_x - half_w), y, (int)ROAD_WIDTH, 1, (Color){ 48, 48, 52, 255 });

            // Red & White Curbs
            int curb_segment = (int)(line_dist * 0.08f) % 2;
            Color curb_color = (curb_segment == 0) ? (Color){ 220, 30, 30, 255 } : WHITE;

            DrawRectangle((int)(center_x - half_w - ROAD_CURB_WIDTH), y, (int)ROAD_CURB_WIDTH, 1, curb_color);
            DrawRectangle((int)(center_x + half_w), y, (int)ROAD_CURB_WIDTH, 1, curb_color);

            // Barrier guard rails beyond curb
            DrawRectangle((int)(center_x - half_w - ROAD_CURB_WIDTH - 3), y, 3, 1, (Color){ 120, 125, 130, 255 });
            DrawRectangle((int)(center_x + half_w + ROAD_CURB_WIDTH), y, 3, 1, (Color){ 120, 125, 130, 255 });

            // Checkpoint line across road
            for (int cp = 0; cp < NUM_CHECKPOINTS; cp++) {
                if (fabsf(line_dist - r->checkpoints[cp]) < 6.0f) {
                    int checker = ((int)(center_x - half_w) + (int)line_dist) % 12;
                    DrawRectangle((int)(center_x - half_w), y, (int)ROAD_WIDTH, 1, (checker < 6) ? WHITE : BLACK);
                }
            }
        }

        // Dashed Lane Dividers
        int dash_spacing = 38;
        int dash_len = 18;
        for (int y = -dash_spacing; y < VIRTUAL_H + dash_spacing; y += 4) {
            float line_dist = p->distance + (p->y - y) * 1.05f;
            int offset = ((int)line_dist) % dash_spacing;
            if (offset < dash_len) {
                float center_x = Road_GetCenter(r, line_dist);
                // 3 lane dividing lines dividing the 4 lanes
                DrawRectangle((int)(center_x - ROAD_WIDTH * 0.25f - 1), y, 2, 4, (Color){ 240, 240, 240, 220 });
                DrawRectangle((int)(center_x - 1),                     y, 2, 4, (Color){ 240, 240, 240, 220 });
                DrawRectangle((int)(center_x + ROAD_WIDTH * 0.25f - 1), y, 2, 4, (Color){ 240, 240, 240, 220 });
            }
        }

        // Draw Scenery Objects on Sides (Beach Palms/Umbrellas, Forest Pines/Oaks, Mountain Rocks, City Buildings/Lamps)
        Scenery_Draw(&ctx->scenery, r, p->distance, p->y);

        // Draw Items & Hazards
        for (int i = 0; i < MAX_ITEMS; i++) {
            const Item *item = &ctx->items.items[i];
            if (!item->active) continue;

            float dist_diff = item->track_dist - p->distance;
            float screen_y = p->y - dist_diff * 0.95f;
            if (screen_y < -20 || screen_y > VIRTUAL_H + 20) continue;

            float road_center = Road_GetCenter(r, item->track_dist);
            float screen_x = road_center + item->lateral_offset;

            if (item->type == ITEM_OIL_SLICK) {
                // Black glistening oil puddle
                DrawEllipse((int)screen_x, (int)screen_y, item->width / 2.0f, item->height / 2.0f, (Color){ 20, 20, 25, 230 });
                DrawEllipse((int)screen_x - 2, (int)screen_y - 1, (item->width / 2.0f) * 0.6f, (item->height / 2.0f) * 0.5f, (Color){ 45, 45, 60, 255 });
            } else if (item->type == ITEM_FUEL_CAN) {
                // Fuel canister
                DrawRectangle((int)(screen_x - 6), (int)(screen_y - 6), 12, 12, (Color){ 220, 30, 30, 255 });
                DrawRectangle((int)(screen_x - 3), (int)(screen_y - 8), 6, 3, (Color){ 180, 180, 180, 255 });
                DrawText("F", (int)screen_x - 3, (int)screen_y - 5, 10, WHITE);
            } else if (item->type == ITEM_ROADWORK) {
                // Orange and white construction barrier
                DrawRectangle((int)(screen_x - 11), (int)(screen_y - 6), 22, 12, ORANGE);
                DrawRectangle((int)(screen_x - 7), (int)(screen_y - 6), 4, 12, WHITE);
                DrawRectangle((int)(screen_x + 3), (int)(screen_y - 6), 4, 12, WHITE);
                DrawRectangle((int)(screen_x - 11), (int)(screen_y + 4), 22, 2, BLACK);
            }
        }

        // Draw Traffic Vehicles
        for (int i = 0; i < MAX_TRAFFIC; i++) {
            const TrafficCar *c = &ctx->traffic.cars[i];
            if (!c->active) continue;

            float dist_diff = c->track_dist - p->distance;
            float screen_y = p->y - dist_diff * 0.95f;
            if (screen_y < -40 || screen_y > VIRTUAL_H + 40) continue;

            float road_center = Road_GetCenter(r, c->track_dist);
            float screen_x = road_center + c->lateral_offset;

            bool is_truck = (c->type == TRAFFIC_FUEL_TRUCK);
            bool blink = c->indicator_on && (((int)(c->blink_timer * 8.0f)) % 2 == 0);

            DrawCarSprite(screen_x, screen_y, c->width, c->height, c->body_color, 0.0f, is_truck, blink);
        }

        // Draw Player Car
        if (p->is_crashed) {
            DrawExplosion(p->x, p->y, p->crash_timer);
        } else {
            // Skid marks if sliding
            if (p->spin_dir != 0) {
                DrawRectangle((int)p->x - 6, (int)p->y + 8, 3, 6, (Color){ 25, 25, 25, 180 });
                DrawRectangle((int)p->x + 3, (int)p->y + 8, 3, 6, (Color){ 25, 25, 25, 180 });
            }

            float draw_angle = p->is_spinning_out ? p->spin_out_angle : p->spin_angle;
            Color player_color = (p->counter_steer_flash > 0.0f && ((int)(p->counter_steer_flash * 20.0f) % 2 == 0))
                                 ? WHITE : (Color){ 235, 35, 35, 255 };

            DrawCarSprite(p->x, p->y, 14.0f, 24.0f, player_color, draw_angle, false, false);
        }

        // HUD Overlay
        // Top Banner Bar
        DrawRectangle(0, 0, VIRTUAL_W, 20, (Color){ 0, 0, 0, 190 });

        // Score
        DrawText(TextFormat("SCORE: %06d", p->score), 6, 6, 10, WHITE);
        DrawText(TextFormat("HI: %06d", ctx->high_score), 115, 6, 10, (Color){ 210, 210, 210, 255 });

        // Speedometer & Gear (Bottom HUD)
        DrawRectangle(0, VIRTUAL_H - 24, VIRTUAL_W, 24, (Color){ 0, 0, 0, 195 });

        // Speed
        DrawText(TextFormat("SPEED: %03d", (int)p->speed), 8, VIRTUAL_H - 18, 10, YELLOW);
        DrawText("KM/H", 78, VIRTUAL_H - 17, 8, (Color){ 180, 180, 180, 255 });

        // Gear display
        const char *gear_text = (p->gear == 1) ? "GEAR: HIGH" : "GEAR: LOW";
        Color gear_color = (p->gear == 1) ? (Color){ 255, 90, 90, 255 } : (Color){ 80, 240, 80, 255 };
        DrawText(gear_text, 115, VIRTUAL_H - 18, 10, gear_color);

        // Fuel Gauge
        DrawText("FUEL:", 205, VIRTUAL_H - 18, 10, WHITE);
        DrawRectangle(238, VIRTUAL_H - 17, 72, 8, (Color){ 30, 30, 30, 255 });
        int fuel_bar_w = (int)((p->fuel / 100.0f) * 70.0f);
        Color fuel_col = (p->fuel < 20.0f) ? (((int)(GetTime() * 6.0f) % 2 == 0) ? RED : ORANGE) : GREEN;
        DrawRectangle(239, VIRTUAL_H - 16, fuel_bar_w, 6, fuel_col);
        DrawRectangleLines(238, VIRTUAL_H - 17, 72, 8, (Color){ 160, 160, 160, 255 });

        // Distance Remaining Indicator
        float remaining_m = fmaxf(TOTAL_TRACK_LENGTH - p->distance, 0.0f);
        DrawText(TextFormat("REMAIN: %04dm", (int)remaining_m), 215, 6, 10, (Color){ 100, 220, 255, 255 });

        // Emergency Warnings: Counter-Steer Alert!
        if (p->spin_dir != 0 && !p->is_spinning_out && !p->is_crashed) {
            const char *cue = (p->spin_dir == -1) ? ">>> COUNTER-STEER RIGHT! >>>" : "<<< COUNTER-STEER LEFT! <<<";
            int tw = MeasureText(cue, 10);
            int tx = (VIRTUAL_W - tw) / 2;
            DrawRectangle(tx - 6, 50, tw + 12, 16, (Color){ 220, 20, 20, 210 });
            DrawText(cue, tx, 54, 10, WHITE);
        }

        // Checkpoint Announcement with Zone Names
        if (r->checkpoint_alert_timer > 0.0f) {
            const char *cp_msg = "CHECKPOINT REACHED! +2500";
            if (r->next_checkpoint == 1) {
                cp_msg = "CP 1: FOREST ZONE REACHED! +2500";
            } else if (r->next_checkpoint == 2) {
                cp_msg = "CP 2: MOUNTAIN PASS REACHED! +2500";
            } else if (r->next_checkpoint == 3) {
                cp_msg = "CP 3: CITY BOULEVARD! +2500";
            } else if (r->next_checkpoint >= 4) {
                cp_msg = "FINAL CHECKPOINT! +2500";
            }

            int tw = MeasureText(cp_msg, 10);
            int tx = (VIRTUAL_W - tw) / 2;
            DrawRectangle(tx - 6, 80, tw + 12, 16, (Color){ 0, 160, 40, 220 });
            DrawText(cp_msg, tx, 84, 10, YELLOW);
        }

        // Game Over Overlay
        if (ctx->state == STATE_GAME_OVER) {
            DrawRectangle(0, 70, VIRTUAL_W, 60, (Color){ 0, 0, 0, 215 });
            const char *go_msg = (p->fuel <= 0.0f) ? "OUT OF FUEL!" : "WRECKED!";
            int tw1 = MeasureText(go_msg, 20);
            DrawText(go_msg, (VIRTUAL_W - tw1) / 2, 78, 20, RED);

            const char *restart_msg = "PRESS ENTER OR SPACE TO RETRY";
            int tw2 = MeasureText(restart_msg, 10);
            DrawText(restart_msg, (VIRTUAL_W - tw2) / 2, 108, 10, WHITE);
        }

        // Stage Clear Overlay
        if (ctx->state == STATE_STAGE_CLEAR) {
            DrawRectangle(0, 60, VIRTUAL_W, 80, (Color){ 0, 0, 0, 220 });
            const char *clear_msg = "STAGE CLEAR!";
            int tw1 = MeasureText(clear_msg, 20);
            DrawText(clear_msg, (VIRTUAL_W - tw1) / 2, 72, 20, GOLD);

            const char *bonus_msg = TextFormat("BONUS FUEL PTS: +%d", (int)(p->fuel * 100));
            int tw2 = MeasureText(bonus_msg, 10);
            DrawText(bonus_msg, (VIRTUAL_W - tw2) / 2, 100, 10, WHITE);

            const char *cont_msg = "PRESS ENTER TO RACE AGAIN";
            int tw3 = MeasureText(cont_msg, 10);
            DrawText(cont_msg, (VIRTUAL_W - tw3) / 2, 118, 10, GREEN);
        }

        // Title Screen Overlay
        if (ctx->state == STATE_TITLE) {
            DrawRectangle(0, 0, VIRTUAL_W, VIRTUAL_H, (Color){ 10, 12, 18, 235 });

            const char *t1 = "HIGHWAY FIGHTER";
            int w1 = MeasureText(t1, 20);
            DrawText(t1, (VIRTUAL_W - w1) / 2 + 1, 41, 20, BLACK);
            DrawText(t1, (VIRTUAL_W - w1) / 2, 40, 20, RED);

            const char *sub = "KONAMI ROAD FIGHTER TRIBUTE - 80s SYNTH";
            int wsub = MeasureText(sub, 10);
            DrawText(sub, (VIRTUAL_W - wsub) / 2, 63, 10, GOLD);

            const char *zones = "[ BEACH -> FOREST -> MOUNTAINS -> CITY ]";
            int wz = MeasureText(zones, 8);
            DrawText(zones, (VIRTUAL_W - wz) / 2, 77, 8, (Color){ 80, 220, 255, 255 });

            DrawText("CONTROLS:", 50, 96, 10, YELLOW);
            DrawText("- ACCEL: UP / W", 60, 110, 10, WHITE);
            DrawText("- BRAKE: DOWN / S", 60, 122, 10, WHITE);
            DrawText("- STEER: LEFT / RIGHT (A / D)", 60, 134, 10, WHITE);
            DrawText("- GEAR SHIFT: SHIFT / SPACE / Z", 60, 146, 10, SKYBLUE);
            DrawText("- RECOVER SKID: COUNTER-STEER!", 60, 158, 10, ORANGE);

            if (((int)(GetTime() * 2.5f)) % 2 == 0) {
                const char *press = "PRESS ENTER TO START";
                int wp = MeasureText(press, 10);
                DrawText(press, (VIRTUAL_W - wp) / 2, 186, 10, GREEN);
            }

        }

    EndTextureMode();
}

void Render_ToScreen(const RenderSystem *rs, int window_w, int window_h) {
    BeginDrawing();
        ClearBackground(BLACK);

        // Aspect ratio preserved scaling
        float scale = fminf((float)window_w / (float)VIRTUAL_W, (float)window_h / (float)VIRTUAL_H);
        float dest_w = VIRTUAL_W * scale;
        float dest_h = VIRTUAL_H * scale;
        float dest_x = (window_w - dest_w) / 2.0f;
        float dest_y = (window_h - dest_h) / 2.0f;

        Rectangle src = { 0.0f, 0.0f, (float)rs->target.texture.width, -(float)rs->target.texture.height };
        Rectangle dst = { dest_x, dest_y, dest_w, dest_h };

        DrawTexturePro(rs->target.texture, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // Retro CRT Scanlines & Tube Overlay
        if (rs->crt_enabled) {
            Color scan_color = (Color){ 0, 0, 0, (unsigned char)(rs->scanline_alpha * 255.0f) };
            for (float y = dest_y; y < dest_y + dest_h; y += scale * 2.0f) {
                DrawRectangle((int)dest_x, (int)y, (int)dest_w, (int)(scale * 0.7f), scan_color);
            }
            // CRT bezel border
            DrawRectangleLinesEx(dst, 2.0f, (Color){ 20, 20, 20, 200 });
        }
    EndDrawing();
}

void Render_Cleanup(RenderSystem *rs) {
    UnloadRenderTexture(rs->target);
}
