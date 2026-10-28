#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include "scenery.h"

// Internal virtual resolution (authentic retro 320x240)
#define VIRTUAL_W 320
#define VIRTUAL_H 240

// Default window resolution
#define SCREEN_W 960
#define SCREEN_H 720

// Road geometry
#define ROAD_WIDTH 180.0f
#define ROAD_CURB_WIDTH 8.0f
#define TOTAL_TRACK_LENGTH 15000.0f
#define NUM_CHECKPOINTS 4

// Limits
#define MAX_TRAFFIC 16
#define MAX_ITEMS 20

// Speed limits (km/h)
#define MAX_SPEED_LOW 110.0f
#define MAX_SPEED_HIGH 220.0f
#define ACCEL_LOW 110.0f
#define ACCEL_HIGH 86.4f
#define DRAG_DECEL 47.1f
#define BRAKE_DECEL 146.6f
#define OFFROAD_DECEL 125.7f
#define SCROLL_SPEED_MULT 3.341f

// Game States
typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_STAGE_CLEAR,
    STATE_GAME_OVER
} StateType;

// Sound Effects
typedef enum {
    SFX_NONE = 0,
    SFX_SKID,
    SFX_CRASH,
    SFX_FUEL,
    SFX_CHECKPOINT,
    SFX_GEAR
} SfxType;

// Player Vehicle
typedef struct {
    float x;                 // Screen X position
    float y;                 // Screen Y position (normally around 190)
    float speed;             // km/h (0 to 220)
    int gear;                // 0 = Low (max ~110), 1 = High (max ~220)
    float fuel;              // 0 to 100
    float distance;          // Distance travelled along the track (meters/units)
    int score;               // Score points

    // Side-swipe & skid physics
    int spin_dir;            // -1 = sliding left, 1 = sliding right, 0 = normal
    float spin_timer;        // Time spent in slide
    float spin_angle;        // Visual tilt angle in degrees
    bool is_spinning_out;    // Out-of-control spin into barrier
    float spin_out_angle;    // 360 degree rotation during spinout
    bool is_crashed;         // Crash explosion active
    float crash_timer;       // Crash animation timer
    float counter_steer_flash; // Visual flash cue when recovered
} Player;

// Road System
typedef struct Road {
    float length;
    float checkpoints[NUM_CHECKPOINTS];
    int next_checkpoint;
    float checkpoint_alert_timer;
} Road;

// Traffic Cars
typedef enum {
    TRAFFIC_YELLOW,      // Travels straight at constant moderate speed
    TRAFFIC_BLUE,        // Lane-changer, abruptly swerves when player nears
    TRAFFIC_FUEL_TRUCK   // Special bonus truck that grants fuel/points
} TrafficType;

typedef struct {
    TrafficType type;
    float track_dist;    // World distance along track
    float lateral_offset;// Offset from road center (-ROAD_WIDTH/2 to +ROAD_WIDTH/2)
    float target_offset; // Target lateral offset for lane changes
    float speed;         // km/h
    float width, height; // Dimensions
    Color body_color;
    bool active;
    float lane_timer;    // Timer for lane changes
    bool indicator_on;   // Turn signal blinker
    float blink_timer;
} TrafficCar;

typedef struct {
    TrafficCar cars[MAX_TRAFFIC];
    float spawn_timer;
} TrafficSystem;

// Hazards and Collectible Items
typedef enum {
    ITEM_OIL_SLICK,
    ITEM_ROADWORK,
    ITEM_FUEL_CAN
} ItemType;

typedef struct {
    ItemType type;
    float track_dist;
    float lateral_offset;
    float width, height;
    bool active;
} Item;

typedef struct {
    Item items[MAX_ITEMS];
    float spawn_timer;
} ItemSystem;

// Renderer System
typedef struct {
    RenderTexture2D target;
    bool crt_enabled;
    float scanline_alpha;
} RenderSystem;

// Audio System
typedef struct {
    AudioStream engine_stream;
    AudioStream music_stream;
    Sound snd_skid;
    Sound snd_crash;
    Sound snd_fuel;
    Sound snd_checkpoint;
    Sound snd_gear;
    bool initialized;
} AudioSystem;

// Combined Game Context
typedef struct {
    StateType state;
    Player player;
    Road road;
    TrafficSystem traffic;
    ItemSystem items;
    ScenerySystem scenery;
    RenderSystem render;
    AudioSystem audio;
    int high_score;
    float state_timer;
} GameContext;

// ==========================================
// Function Prototypes
// ==========================================

// Player
void Player_Init(Player *p);
void Player_Update(Player *p, const Road *r, float dt);
void Player_TriggerSlide(Player *p, int dir);
void Player_Crash(Player *p);
void Player_RecoverSlide(Player *p);
Rectangle Player_GetBounds(const Player *p);

// Road
void Road_Init(Road *r);
float Road_GetCenter(const Road *r, float track_dist);
bool Road_IsOffroad(const Road *r, float x, float track_dist);
void Road_CheckPoints(Road *r, Player *p);

// Traffic
void Traffic_Init(TrafficSystem *ts);
void Traffic_Update(TrafficSystem *ts, Player *p, const Road *r, float dt);
void Traffic_Spawn(TrafficSystem *ts, float player_dist, const Road *r);

// Items
void Items_Init(ItemSystem *is);
void Items_Update(ItemSystem *is, Player *p, const Road *r, float dt);
void Items_Spawn(ItemSystem *is, float player_dist, const Road *r);

// Audio
void Audio_Init(AudioSystem *as);
void Audio_Update(AudioSystem *as, const Player *p, bool is_offroad);
void Audio_PlaySfx(AudioSystem *as, SfxType sfx);
void Audio_Cleanup(AudioSystem *as);

// Render
void Render_Init(RenderSystem *rs);
void Render_Game(const RenderSystem *rs, const GameContext *ctx);
void Render_ToScreen(const RenderSystem *rs, int window_w, int window_h);
void Render_Cleanup(RenderSystem *rs);

#endif // GAME_H
