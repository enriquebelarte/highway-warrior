#include "game.h"
#include <math.h>

void Player_Init(Player *p) {
    p->x = VIRTUAL_W / 2.0f;
    p->y = 190.0f;
    p->speed = 0.0f;
    p->gear = 0; // Low gear
    p->fuel = 100.0f;
    p->distance = 0.0f;
    p->score = 0;

    p->spin_dir = 0;
    p->spin_timer = 0.0f;
    p->spin_angle = 0.0f;
    p->is_spinning_out = false;
    p->spin_out_angle = 0.0f;
    p->is_crashed = false;
    p->crash_timer = 0.0f;
    p->counter_steer_flash = 0.0f;
}

void Player_TriggerSlide(Player *p, int dir) {
    if (p->is_crashed || p->is_spinning_out) return;
    
    // If not already sliding, initiate slide
    if (p->spin_dir == 0) {
        p->spin_dir = (dir < 0) ? -1 : 1;
        p->spin_timer = 0.0f;
        p->spin_angle = (p->spin_dir == -1) ? -22.0f : 22.0f;
    } else {
        // If hit again while already sliding, intensify or provoke full spin
        if (dir == p->spin_dir) {
            p->is_spinning_out = true;
        }
    }
}

void Player_RecoverSlide(Player *p) {
    p->spin_dir = 0;
    p->spin_timer = 0.0f;
    p->spin_angle = 0.0f;
    p->counter_steer_flash = 0.25f;
}

void Player_Crash(Player *p) {
    if (p->is_crashed) return;
    p->is_crashed = true;
    p->crash_timer = 1.4f;
    p->speed = 0.0f;
    p->spin_dir = 0;
    p->is_spinning_out = false;
    p->fuel = fmaxf(p->fuel - 10.0f, 0.0f); // Penalty for crashing
}

Rectangle Player_GetBounds(const Player *p) {
    return (Rectangle){
        p->x - 7.0f,
        p->y - 12.0f,
        14.0f,
        24.0f
    };
}

void Player_Update(Player *p, const Road *r, float dt) {
    if (p->counter_steer_flash > 0.0f) {
        p->counter_steer_flash -= dt;
    }

    // Handle Crash State
    if (p->is_crashed) {
        p->crash_timer -= dt;
        if (p->crash_timer <= 0.0f) {
            // Respawn after crash
            p->is_crashed = false;
            p->speed = 0.0f;
            p->gear = 0; // Reset to low gear
            p->x = Road_GetCenter(r, p->distance);
            p->spin_dir = 0;
            p->is_spinning_out = false;
            p->spin_angle = 0.0f;
        }
        return;
    }

    // Handle Uncontrolled Spinout State
    if (p->is_spinning_out) {
        p->spin_out_angle += 720.0f * dt;
        p->x += (p->spin_dir != 0 ? p->spin_dir : 1) * 160.0f * dt;
        p->speed = fmaxf(p->speed - 115.0f * dt, 21.0f);
        p->distance += (p->speed * 0.28f) * dt * SCROLL_SPEED_MULT;

        float road_center = Road_GetCenter(r, p->distance);
        float road_left = road_center - (ROAD_WIDTH / 2.0f);
        float road_right = road_center + (ROAD_WIDTH / 2.0f);

        // Crashing into barriers
        if (p->x <= road_left - 12.0f || p->x >= road_right + 12.0f) {
            Player_Crash(p);
        }
        return;
    }

    // Input Controls: Gear Shift
    if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT) ||
        IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_SPACE)) {
        p->gear = !p->gear;
    }
    if (IsKeyPressed(KEY_ONE)) p->gear = 0;
    if (IsKeyPressed(KEY_TWO)) p->gear = 1;

    // Check if car is currently off-road
    bool offroad = Road_IsOffroad(r, p->x, p->distance);

    // Maximum speed for current gear
    float max_speed = (p->gear == 0) ? MAX_SPEED_LOW : MAX_SPEED_HIGH;
    if (offroad) {
        max_speed = fminf(max_speed, 50.0f);
    }

    // Acceleration & Braking
    bool is_accelerating = (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && (p->fuel > 0.0f);
    bool is_braking = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);

    if (is_accelerating) {
        float accel_rate = (p->gear == 0) ? ACCEL_LOW : ACCEL_HIGH;
        // In high gear at low speed, acceleration is more sluggish
        if (p->gear == 1 && p->speed < 63.0f) {
            accel_rate *= 0.45f;
        }
        p->speed = fminf(p->speed + accel_rate * dt, max_speed);

        // Fuel consumption: higher at high speed, even worse offroad
        float fuel_burn_rate = 0.5f + (p->speed / MAX_SPEED_HIGH) * 2.2f;
        if (offroad) fuel_burn_rate *= 2.8f;
        p->fuel = fmaxf(p->fuel - fuel_burn_rate * dt, 0.0f);
    } else if (is_braking) {
        p->speed = fmaxf(p->speed - BRAKE_DECEL * dt, 0.0f);
    } else {
        // Natural rolling resistance / drag
        float drag = offroad ? OFFROAD_DECEL : DRAG_DECEL;
        p->speed = fmaxf(p->speed - drag * dt, 0.0f);
    }

    // Advance distance along track
    p->distance += (p->speed * 0.28f) * dt * SCROLL_SPEED_MULT;

    // Score accumulation based on speed
    if (p->speed > 0.0f) {
        p->score += (int)((p->speed / 10.0f) * dt * 57.3f);
    }

    // Side-swipe Slide & Counter-Steering Mechanics
    if (p->spin_dir != 0) {
        p->spin_timer += dt;

        // Slide pushing the car laterally
        float slide_speed = 135.0f * (0.8f + (p->speed / MAX_SPEED_HIGH) * 0.6f);
        p->x += p->spin_dir * slide_speed * dt;

        // Visual tilt oscillates slightly as car struggles
        p->spin_angle = (p->spin_dir == -1) ? (-22.0f + sinf(p->spin_timer * 20.0f) * 4.0f)
                                             : ( 22.0f + sinf(p->spin_timer * 20.0f) * 4.0f);

        // Check for Counter-Steering
        // Sliding LEFT requires counter-steering RIGHT
        // Sliding RIGHT requires counter-steering LEFT
        bool counter_steered = false;
        if (p->spin_dir == -1 && (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))) {
            counter_steered = true;
        } else if (p->spin_dir == 1 && (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))) {
            counter_steered = true;
        }

        if (counter_steered) {
            // Player successfully applied counter-steer
            Player_RecoverSlide(p);
        } else {
            // If steer in same direction as slide, triggers immediate spinout!
            if ((p->spin_dir == -1 && (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))) ||
                (p->spin_dir == 1 && (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)))) {
                p->is_spinning_out = true;
                p->spin_out_angle = p->spin_angle;
            } else if (p->spin_timer > 0.52f) {
                // Window expired: failed to counter-steer in time!
                p->is_spinning_out = true;
                p->spin_out_angle = p->spin_angle;
            }
        }
    } else {
        // Normal Steering
        float steer_rate = 125.0f;
        if (p->speed > 0.0f) {
            float speed_factor = fminf(p->speed / 52.0f, 1.25f);
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  p->x -= steer_rate * speed_factor * dt;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) p->x += steer_rate * speed_factor * dt;
        }
        p->spin_angle = 0.0f;
    }

    // Boundary check against road edges & barrier collisions
    float road_center = Road_GetCenter(r, p->distance);
    float road_left = road_center - (ROAD_WIDTH / 2.0f);
    float road_right = road_center + (ROAD_WIDTH / 2.0f);

    // Hard crash into outer guardrails / walls
    if (p->x <= road_left - 18.0f || p->x >= road_right + 18.0f) {
        Player_Crash(p);
    }
}
