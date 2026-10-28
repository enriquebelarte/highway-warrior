#include "game.h"
#include <stdio.h>
#include <math.h>

static void ResetGame(GameContext *ctx, bool reset_score) {
    Player_Init(&ctx->player);
    Road_Init(&ctx->road);
    Traffic_Init(&ctx->traffic);
    Items_Init(&ctx->items);
    Scenery_Init(&ctx->scenery);

    if (reset_score) {
        ctx->player.score = 0;
    }
    ctx->state_timer = 0.0f;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_W, SCREEN_H, "Highway Fighter - Konami Road Fighter Tribute");
    SetWindowMinSize(VIRTUAL_W, VIRTUAL_H);
    SetTargetFPS(60);

    GameContext ctx = { 0 };
    ctx.state = STATE_TITLE;
    ctx.high_score = 10000;

    Render_Init(&ctx.render);
    Audio_Init(&ctx.audio);
    ResetGame(&ctx, true);

    int prev_gear = ctx.player.gear;
    int prev_spin_dir = ctx.player.spin_dir;
    bool prev_crashed = ctx.player.is_crashed;
    float prev_fuel = ctx.player.fuel;
    int prev_checkpoint = ctx.road.next_checkpoint;

    while (!WindowShouldClose()) {
        float dt = fminf(GetFrameTime(), 0.05f); // Delta time clamp to prevent tunneling

        // Toggle CRT Scanlines
        if (IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_C)) {
            ctx.render.crt_enabled = !ctx.render.crt_enabled;
        }

        // State Machine Handling
        switch (ctx.state) {
            case STATE_TITLE:
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_Z)) {
                    ResetGame(&ctx, true);
                    ctx.state = STATE_PLAYING;
                    Audio_PlaySfx(&ctx.audio, SFX_GEAR);
                }
                break;

            case STATE_PLAYING: {
                // Update Road Checkpoints
                Road_CheckPoints(&ctx.road, &ctx.player);

                // Update Player
                Player_Update(&ctx.player, &ctx.road, dt);

                // Update Traffic
                Traffic_Update(&ctx.traffic, &ctx.player, &ctx.road, dt);

                // Update Hazards & Collectibles
                Items_Update(&ctx.items, &ctx.player, &ctx.road, dt);

                // Offroad check for audio & physics
                bool offroad = Road_IsOffroad(&ctx.road, ctx.player.x, ctx.player.distance);

                // Audio Events Detection
                if (ctx.player.gear != prev_gear) {
                    Audio_PlaySfx(&ctx.audio, SFX_GEAR);
                }
                if (prev_spin_dir == 0 && ctx.player.spin_dir != 0) {
                    Audio_PlaySfx(&ctx.audio, SFX_SKID);
                }
                if (ctx.player.counter_steer_flash > 0.20f && prev_spin_dir != 0 && ctx.player.spin_dir == 0) {
                    Audio_PlaySfx(&ctx.audio, SFX_SKID);
                }
                if (!prev_crashed && ctx.player.is_crashed) {
                    Audio_PlaySfx(&ctx.audio, SFX_CRASH);
                }
                if (ctx.road.next_checkpoint != prev_checkpoint) {
                    Audio_PlaySfx(&ctx.audio, SFX_CHECKPOINT);
                }
                if (ctx.player.fuel > prev_fuel + 5.0f) {
                    Audio_PlaySfx(&ctx.audio, SFX_FUEL);
                }

                prev_gear = ctx.player.gear;
                prev_spin_dir = ctx.player.spin_dir;
                prev_crashed = ctx.player.is_crashed;
                prev_fuel = ctx.player.fuel;
                prev_checkpoint = ctx.road.next_checkpoint;

                // Update High Score
                if (ctx.player.score > ctx.high_score) {
                    ctx.high_score = ctx.player.score;
                }

                // Check Goal / Stage Clear
                if (ctx.player.distance >= ctx.road.length) {
                    ctx.state = STATE_STAGE_CLEAR;
                    ctx.player.score += (int)(ctx.player.fuel * 100);
                    if (ctx.player.score > ctx.high_score) ctx.high_score = ctx.player.score;
                    Audio_PlaySfx(&ctx.audio, SFX_CHECKPOINT);
                }

                // Check Out of Fuel / Game Over
                if (ctx.player.fuel <= 0.0f && ctx.player.speed <= 0.0f && !ctx.player.is_crashed) {
                    ctx.state = STATE_GAME_OVER;
                }

                // Update Procedural Engine Hum & Pitch
                Audio_Update(&ctx.audio, &ctx.player, offroad);
                break;
            }

            case STATE_STAGE_CLEAR:
                Audio_Update(&ctx.audio, &ctx.player, false);
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    ResetGame(&ctx, false);
                    ctx.state = STATE_PLAYING;
                }
                break;

            case STATE_GAME_OVER:
                Audio_Update(&ctx.audio, &ctx.player, false);
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_R)) {
                    ResetGame(&ctx, true);
                    ctx.state = STATE_PLAYING;
                }
                break;
        }

        // Render pass to virtual 320x240 canvas
        Render_Game(&ctx.render, &ctx);

        // Blit and upscale to actual window with optional CRT effect
        Render_ToScreen(&ctx.render, GetScreenWidth(), GetScreenHeight());
    }

    // Teardown & Resource Cleanup
    Audio_Cleanup(&ctx.audio);
    Render_Cleanup(&ctx.render);
    CloseWindow();

    return 0;
}
