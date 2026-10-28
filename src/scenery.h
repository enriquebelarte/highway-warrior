#ifndef SCENERY_H
#define SCENERY_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_SCENERY 750

typedef enum {
    // Zone 0: Beach & Coast (0 - 3500m)
    SCENERY_PALM_TREE,
    SCENERY_SUN_UMBRELLA,
    SCENERY_BEACH_TOWEL,
    SCENERY_SURFBOARD,

    // Zone 1: Forest & Woods (3500 - 7200m)
    SCENERY_PINE_TREE,
    SCENERY_OAK_TREE,
    SCENERY_BUSH,
    SCENERY_WOOD_LOG,

    // Zone 2: Mountains & Canyon (7200 - 11000m)
    SCENERY_MOUNTAIN_PEAK,
    SCENERY_BOULDER,
    SCENERY_ALPINE_PINE,
    SCENERY_ROCK_PILLAR,

    // Zone 3: City & Sunset Boulevard (11000 - 15000m)
    SCENERY_STREET_LAMP,
    SCENERY_SKYSCRAPER,
    SCENERY_NEON_BILLBOARD,
    SCENERY_FINISH_ARCH
} SceneryType;

typedef struct {
    SceneryType type;
    float track_dist;
    int side;             // -1 = Left, +1 = Right
    float lateral_offset; // Pixels outward from the curb
    int variant;          // Color / text / visual variation
    float scale;
} SceneryObject;

typedef struct {
    SceneryObject objects[MAX_SCENERY];
    int count;
} ScenerySystem;

typedef struct Road Road;

void Scenery_Init(ScenerySystem *ss);
void Scenery_Draw(const ScenerySystem *ss, const Road *r, float player_dist, float player_y);

#endif // SCENERY_H
