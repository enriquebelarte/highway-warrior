#include "scenery.h"
#include "game.h"
#include <stdlib.h>
#include <math.h>

void Scenery_Init(ScenerySystem *ss) {
    ss->count = 0;

    // Helper macro to add object safely
    #define ADD_SCENERY(t, dist, s, offset, var, sc) \
        if (ss->count < MAX_SCENERY) { \
            ss->objects[ss->count++] = (SceneryObject){ \
                .type = (t), \
                .track_dist = (dist), \
                .side = (s), \
                .lateral_offset = (offset), \
                .variant = (var), \
                .scale = (sc) \
            }; \
        }

    // ========================================================
    // ZONE 0: Beach & Coast (0 to 3500m)
    // Left side: Palm trees, tropical grass, occasional umbrella
    // Right side: Sand beach & ocean with umbrellas, towels, palms
    // ========================================================
    for (float d = 60.0f; d < 3420.0f; d += 36.0f) {
        // Left side: Palm trees and tropical foliage
        int r_left = (int)(d * 13.0f) % 100;
        float jitter_left = (float)(((int)(d * 7.0f) % 15) - 7);
        if (r_left < 65) {
            ADD_SCENERY(SCENERY_PALM_TREE, d + jitter_left, -1, 14.0f + (r_left % 16), r_left % 3, 0.9f + (r_left % 4) * 0.08f);
        } else if (r_left < 85) {
            ADD_SCENERY(SCENERY_BUSH, d + jitter_left, -1, 12.0f + (r_left % 10), r_left % 2, 0.85f);
        } else {
            ADD_SCENERY(SCENERY_SUN_UMBRELLA, d + jitter_left, -1, 16.0f, r_left % 3, 1.0f);
        }

        // Right side: Beach & Ocean
        int r_right = (int)(d * 17.0f) % 100;
        float jitter_right = (float)(((int)(d * 11.0f) % 15) - 7);
        if (r_right < 40) {
            // Sun umbrellas on the sand
            ADD_SCENERY(SCENERY_SUN_UMBRELLA, d + jitter_right, 1, 12.0f + (r_right % 14), r_right % 3, 0.95f + (r_right % 3) * 0.1f);
        } else if (r_right < 65) {
            // Beach towels & sunbathers
            ADD_SCENERY(SCENERY_BEACH_TOWEL, d + jitter_right, 1, 10.0f + (r_right % 16), r_right % 3, 1.0f);
        } else if (r_right < 85) {
            // Palm trees on beach dunes
            ADD_SCENERY(SCENERY_PALM_TREE, d + jitter_right, 1, 15.0f + (r_right % 12), r_right % 3, 0.95f + (r_right % 4) * 0.07f);
        } else {
            // Surfboards
            ADD_SCENERY(SCENERY_SURFBOARD, d + jitter_right, 1, 14.0f + (r_right % 8), r_right % 3, 1.0f);
        }
    }

    // ========================================================
    // ZONE 1: Forest & Woods (3500m to 7200m)
    // Deep green woodlands, lush pines, leafy oaks, bushes, logs
    // ========================================================
    for (float d = 3540.0f; d < 7120.0f; d += 34.0f) {
        // Left side
        int r_left = (int)(d * 19.0f) % 100;
        float jitter_l = (float)(((int)(d * 5.0f) % 14) - 7);
        if (r_left < 45) {
            ADD_SCENERY(SCENERY_PINE_TREE, d + jitter_l, -1, 12.0f + (r_left % 20), r_left % 3, 0.9f + (r_left % 4) * 0.08f);
        } else if (r_left < 78) {
            ADD_SCENERY(SCENERY_OAK_TREE, d + jitter_l, -1, 14.0f + (r_left % 18), r_left % 2, 0.9f + (r_left % 3) * 0.1f);
        } else if (r_left < 90) {
            ADD_SCENERY(SCENERY_BUSH, d + jitter_l, -1, 10.0f + (r_left % 8), r_left % 2, 1.0f);
        } else {
            ADD_SCENERY(SCENERY_WOOD_LOG, d + jitter_l, -1, 12.0f, 0, 1.0f);
        }

        // Right side
        int r_right = (int)(d * 23.0f) % 100;
        float jitter_r = (float)(((int)(d * 9.0f) % 14) - 7);
        if (r_right < 45) {
            ADD_SCENERY(SCENERY_OAK_TREE, d + jitter_r, 1, 13.0f + (r_right % 18), r_right % 2, 0.9f + (r_right % 4) * 0.08f);
        } else if (r_right < 80) {
            ADD_SCENERY(SCENERY_PINE_TREE, d + jitter_r, 1, 12.0f + (r_right % 20), r_right % 3, 0.95f + (r_right % 3) * 0.1f);
        } else if (r_right < 92) {
            ADD_SCENERY(SCENERY_BUSH, d + jitter_r, 1, 10.0f + (r_right % 8), r_right % 2, 1.0f);
        } else {
            ADD_SCENERY(SCENERY_WOOD_LOG, d + jitter_r, 1, 11.0f, 0, 1.0f);
        }
    }

    // ========================================================
    // ZONE 2: Mountains & Canyon Pass (7200m to 11000m)
    // Rocky canyon, mountain peaks, boulders, alpine pines
    // ========================================================
    for (float d = 7240.0f; d < 10920.0f; d += 38.0f) {
        // Left side
        int r_left = (int)(d * 29.0f) % 100;
        float jitter_l = (float)(((int)(d * 11.0f) % 16) - 8);
        if (r_left < 40) {
            ADD_SCENERY(SCENERY_MOUNTAIN_PEAK, d + jitter_l, -1, 15.0f + (r_left % 15), r_left % 3, 0.95f + (r_left % 4) * 0.1f);
        } else if (r_left < 70) {
            ADD_SCENERY(SCENERY_BOULDER, d + jitter_l, -1, 10.0f + (r_left % 12), r_left % 3, 0.9f + (r_left % 3) * 0.12f);
        } else if (r_left < 88) {
            ADD_SCENERY(SCENERY_ALPINE_PINE, d + jitter_l, -1, 12.0f + (r_left % 14), r_left % 2, 0.9f);
        } else {
            ADD_SCENERY(SCENERY_ROCK_PILLAR, d + jitter_l, -1, 14.0f + (r_left % 10), 0, 1.0f);
        }

        // Right side
        int r_right = (int)(d * 31.0f) % 100;
        float jitter_r = (float)(((int)(d * 13.0f) % 16) - 8);
        if (r_right < 42) {
            ADD_SCENERY(SCENERY_MOUNTAIN_PEAK, d + jitter_r, 1, 16.0f + (r_right % 16), r_right % 3, 0.95f + (r_right % 4) * 0.1f);
        } else if (r_right < 72) {
            ADD_SCENERY(SCENERY_BOULDER, d + jitter_r, 1, 10.0f + (r_right % 12), r_right % 3, 0.9f + (r_right % 3) * 0.12f);
        } else if (r_right < 90) {
            ADD_SCENERY(SCENERY_ALPINE_PINE, d + jitter_r, 1, 12.0f + (r_right % 14), r_right % 2, 0.9f);
        } else {
            ADD_SCENERY(SCENERY_ROCK_PILLAR, d + jitter_r, 1, 13.0f + (r_right % 10), 0, 1.0f);
        }
    }

    // ========================================================
    // ZONE 3: City & Sunset Boulevard (11000m to 15000m)
    // Modern city skyline, streetlamps, neon billboards, finish line
    // ========================================================
    for (float d = 11040.0f; d < 14900.0f; d += 35.0f) {
        // Street lamps lining both sides regularly every ~70m
        if (((int)d) % 70 < 35) {
            ADD_SCENERY(SCENERY_STREET_LAMP, d, -1, 6.0f, 0, 1.0f);
            ADD_SCENERY(SCENERY_STREET_LAMP, d,  1, 6.0f, 0, 1.0f);
        }

        // Left side buildings & billboards
        int r_left = (int)(d * 37.0f) % 100;
        if (r_left < 60) {
            ADD_SCENERY(SCENERY_SKYSCRAPER, d, -1, 18.0f + (r_left % 14), r_left % 4, 0.9f + (r_left % 3) * 0.15f);
        } else if (r_left < 85) {
            ADD_SCENERY(SCENERY_NEON_BILLBOARD, d, -1, 16.0f, r_left % 5, 1.0f);
        }

        // Right side buildings & billboards
        int r_right = (int)(d * 41.0f) % 100;
        if (r_right < 60) {
            ADD_SCENERY(SCENERY_SKYSCRAPER, d, 1, 18.0f + (r_right % 14), r_right % 4, 0.9f + (r_right % 3) * 0.15f);
        } else if (r_right < 85) {
            ADD_SCENERY(SCENERY_NEON_BILLBOARD, d, 1, 16.0f, (r_right + 2) % 5, 1.0f);
        }
    }

    // Checkered Finish Line Overhead Gantry at 14980m
    ADD_SCENERY(SCENERY_FINISH_ARCH, 14980.0f, 0, 0.0f, 0, 1.0f);

    #undef ADD_SCENERY
}

// ========================================================
// Pixel Art Scenery Draw Helpers
// ========================================================

static void DrawPalmTree(float x, float y, float scale, int side) {
    int ix = (int)x;
    int iy = (int)y;

    // Shadow on sand
    DrawEllipse(ix, iy + 2, (int)(14 * scale), (int)(6 * scale), (Color){ 160, 140, 90, 120 });

    // Curved Trunk leaning away from road
    int lean = (side == -1) ? -5 : 5;
    Color trunk_dark = (Color){ 105, 68, 30, 255 };
    Color trunk_light = (Color){ 150, 102, 52, 255 };

    for (int seg = 0; seg < 6; seg++) {
        float t = (float)seg / 5.0f;
        int sx = ix + (int)(lean * t * t * scale);
        int sy = iy - (int)(seg * 5 * scale);
        int w = (int)((4 - seg * 0.4f) * scale);
        if (w < 2) w = 2;
        DrawRectangle(sx - w/2, sy - (int)(4 * scale), w, (int)(4 * scale), (seg % 2 == 0) ? trunk_light : trunk_dark);
    }

    // Top Fronds (Leaves)
    int cx = ix + (int)(lean * scale);
    int cy = iy - (int)(25 * scale);

    // Coconuts
    DrawCircle(cx - 2, cy + 2, (int)(2 * scale), (Color){ 95, 60, 25, 255 });
    DrawCircle(cx + 2, cy + 2, (int)(2 * scale), (Color){ 110, 72, 32, 255 });
    DrawCircle(cx,     cy + 4, (int)(2 * scale), (Color){ 85, 52, 20, 255 });

    // Arching green palm fronds
    Color frond_main = (Color){ 35, 165, 45, 255 };
    Color frond_bright = (Color){ 65, 215, 60, 255 };
    Color frond_shadow = (Color){ 20, 115, 30, 255 };

    // 5 drooping leaf clusters
    // Center-up
    DrawTriangle((Vector2){ cx, cy - 14 * scale }, (Vector2){ cx - 5 * scale, cy }, (Vector2){ cx + 5 * scale, cy }, frond_bright);
    // Left-up & Left-down
    DrawTriangle((Vector2){ cx - 16 * scale, cy - 8 * scale }, (Vector2){ cx, cy - 4 * scale }, (Vector2){ cx, cy + 2 * scale }, frond_main);
    DrawTriangle((Vector2){ cx - 18 * scale, cy + 4 * scale }, (Vector2){ cx, cy }, (Vector2){ cx - 2 * scale, cy + 5 * scale }, frond_shadow);
    // Right-up & Right-down
    DrawTriangle((Vector2){ cx + 16 * scale, cy - 8 * scale }, (Vector2){ cx, cy - 4 * scale }, (Vector2){ cx, cy + 2 * scale }, frond_main);
    DrawTriangle((Vector2){ cx + 18 * scale, cy + 4 * scale }, (Vector2){ cx, cy }, (Vector2){ cx + 2 * scale, cy + 5 * scale }, frond_shadow);
}

static void DrawSunUmbrella(float x, float y, float scale, int variant) {
    int ix = (int)x;
    int iy = (int)y;
    int r = (int)(11 * scale);

    // Cast shadow on sand
    DrawEllipse(ix + 2, iy + 4, r + 2, (int)(r * 0.55f), (Color){ 150, 130, 85, 130 });

    Color col_a, col_b;
    if (variant == 0) {
        col_a = (Color){ 235, 45, 45, 255 }; // Red
        col_b = (Color){ 245, 245, 245, 255 }; // White
    } else if (variant == 1) {
        col_a = (Color){ 40, 175, 230, 255 }; // Cyan
        col_b = (Color){ 250, 220, 40, 255 }; // Yellow
    } else {
        col_a = (Color){ 245, 125, 30, 255 }; // Orange
        col_b = (Color){ 245, 245, 245, 255 }; // White
    }

    // Pie slices (8 segments)
    for (int i = 0; i < 8; i++) {
        float a1 = i * (2.0f * PI / 8.0f);
        float a2 = (i + 1) * (2.0f * PI / 8.0f);
        Vector2 p0 = { (float)ix, (float)iy };
        Vector2 p1 = { ix + cosf(a1) * r, iy + sinf(a1) * r * 0.85f };
        Vector2 p2 = { ix + cosf(a2) * r, iy + sinf(a2) * r * 0.85f };
        DrawTriangle(p0, p1, p2, (i % 2 == 0) ? col_a : col_b);
    }
    // Center pole button
    DrawCircle(ix, iy, (int)(2 * scale), WHITE);
    DrawCircle(ix, iy, 1, DARKGRAY);
}

static void DrawBeachTowel(float x, float y, float scale, int variant) {
    int ix = (int)x;
    int iy = (int)y;
    int w = (int)(10 * scale);
    int h = (int)(18 * scale);

    Color towel_col = (variant == 0) ? (Color){ 45, 150, 235, 255 } :
                      ((variant == 1) ? (Color){ 240, 70, 140, 255 } : (Color){ 50, 210, 160, 255 });

    // Towel rectangle
    DrawRectangle(ix - w/2, iy - h/2, w, h, towel_col);
    // White fringe ends
    DrawRectangle(ix - w/2, iy - h/2, w, 2, WHITE);
    DrawRectangle(ix - w/2, iy + h/2 - 2, w, 2, WHITE);
    // Stripe
    DrawRectangle(ix - w/2, iy - 2, w, 3, WHITE);

    // Cooler box or beach bag next to towel
    DrawRectangle(ix + w/2 + 2, iy - 3, (int)(6 * scale), (int)(6 * scale), (Color){ 230, 50, 50, 255 });
    DrawRectangle(ix + w/2 + 2, iy - 3, (int)(6 * scale), 2, WHITE);
}

static void DrawSurfboard(float x, float y, float scale, int variant) {
    int ix = (int)x;
    int iy = (int)y;
    int h = (int)(16 * scale);
    int w = (int)(5 * scale);

    // Shadow
    DrawEllipse(ix + 2, iy + h/2, (int)(4 * scale), (int)(2 * scale), (Color){ 150, 130, 85, 120 });

    Color board_col = (variant == 0) ? (Color){ 255, 190, 30, 255 } :
                      ((variant == 1) ? (Color){ 245, 50, 100, 255 } : (Color){ 40, 220, 240, 255 });

    // Pointed surfboard
    DrawTriangle((Vector2){ ix, iy - h/2 }, (Vector2){ ix - w/2, iy }, (Vector2){ ix + w/2, iy }, board_col);
    DrawRectangle(ix - w/2, iy, w, h/2, board_col);
    // Center stringer stripe
    DrawLine(ix, iy - h/2 + 2, ix, iy + h/2, WHITE);
}

static void DrawPineTree(float x, float y, float scale) {
    int ix = (int)x;
    int iy = (int)y;

    // Shadow
    DrawEllipse(ix, iy + 2, (int)(12 * scale), (int)(5 * scale), (Color){ 10, 45, 15, 120 });

    // Trunk
    DrawRectangle(ix - 2, iy - (int)(6 * scale), 4, (int)(7 * scale), (Color){ 95, 60, 30, 255 });

    Color pine_dark = (Color){ 16, 75, 30, 255 };
    Color pine_mid = (Color){ 25, 115, 42, 255 };
    Color pine_light = (Color){ 45, 155, 60, 255 };

    // 3 tiered triangular needle layers
    // Bottom tier
    DrawTriangle((Vector2){ ix, iy - 22 * scale }, (Vector2){ ix - 14 * scale, iy - 4 * scale }, (Vector2){ ix + 14 * scale, iy - 4 * scale }, pine_dark);
    DrawTriangle((Vector2){ ix, iy - 22 * scale }, (Vector2){ ix - 7 * scale, iy - 4 * scale }, (Vector2){ ix + 14 * scale, iy - 4 * scale }, pine_mid);

    // Middle tier
    DrawTriangle((Vector2){ ix, iy - 29 * scale }, (Vector2){ ix - 11 * scale, iy - 13 * scale }, (Vector2){ ix + 11 * scale, iy - 13 * scale }, pine_dark);
    DrawTriangle((Vector2){ ix, iy - 29 * scale }, (Vector2){ ix - 5 * scale, iy - 13 * scale }, (Vector2){ ix + 11 * scale, iy - 13 * scale }, pine_mid);

    // Top tier
    DrawTriangle((Vector2){ ix, iy - 36 * scale }, (Vector2){ ix - 8 * scale, iy - 22 * scale }, (Vector2){ ix + 8 * scale, iy - 22 * scale }, pine_mid);
    DrawTriangle((Vector2){ ix, iy - 36 * scale }, (Vector2){ ix, iy - 22 * scale }, (Vector2){ ix + 8 * scale, iy - 22 * scale }, pine_light);
}

static void DrawOakTree(float x, float y, float scale) {
    int ix = (int)x;
    int iy = (int)y;

    // Shadow
    DrawEllipse(ix, iy + 2, (int)(16 * scale), (int)(6 * scale), (Color){ 10, 45, 15, 120 });

    // Trunk & Roots
    DrawRectangle(ix - 3, iy - (int)(10 * scale), 6, (int)(11 * scale), (Color){ 110, 70, 35, 255 });
    DrawRectangle(ix - 5, iy - 2, 10, 3, (Color){ 90, 55, 25, 255 });

    // Multi-blob canopy
    int cy = iy - (int)(22 * scale);
    Color c_dark = (Color){ 20, 90, 30, 255 };
    Color c_mid = (Color){ 38, 145, 48, 255 };
    Color c_bright = (Color){ 70, 195, 65, 255 };

    // Background darker clusters
    DrawCircle(ix - (int)(8 * scale), cy + (int)(3 * scale), (int)(10 * scale), c_dark);
    DrawCircle(ix + (int)(8 * scale), cy + (int)(3 * scale), (int)(10 * scale), c_dark);
    DrawCircle(ix, cy - (int)(5 * scale), (int)(11 * scale), c_dark);

    // Mid green clusters
    DrawCircle(ix - (int)(6 * scale), cy + (int)(1 * scale), (int)(8 * scale), c_mid);
    DrawCircle(ix + (int)(6 * scale), cy + (int)(1 * scale), (int)(8 * scale), c_mid);
    DrawCircle(ix, cy - (int)(6 * scale), (int)(9 * scale), c_mid);

    // Bright sun highlights on top-right
    DrawCircle(ix + (int)(3 * scale), cy - (int)(8 * scale), (int)(6 * scale), c_bright);
    DrawCircle(ix + (int)(8 * scale), cy - (int)(1 * scale), (int)(5 * scale), c_bright);
}

static void DrawBush(float x, float y, float scale, int variant) {
    int ix = (int)x;
    int iy = (int)y;
    int r = (int)(7 * scale);

    DrawEllipse(ix, iy + 1, r + 2, (int)(r * 0.5f), (Color){ 10, 45, 15, 100 });
    DrawCircle(ix, iy - 2, r, (Color){ 30, 120, 40, 255 });
    DrawCircle(ix + 1, iy - 3, (int)(r * 0.7f), (Color){ 55, 175, 55, 255 });

    // Berries / flowers
    if (variant == 0) {
        DrawCircle(ix - 3, iy - 4, 1, RED);
        DrawCircle(ix + 2, iy - 5, 1, RED);
        DrawCircle(ix - 1, iy - 1, 1, RED);
        DrawCircle(ix + 3, iy - 2, 1, RED);
    } else {
        DrawCircle(ix - 2, iy - 3, 1, YELLOW);
        DrawCircle(ix + 2, iy - 4, 1, YELLOW);
    }
}

static void DrawWoodLog(float x, float y, float scale) {
    int ix = (int)x;
    int iy = (int)y;
    int w = (int)(14 * scale);
    int h = (int)(6 * scale);

    DrawRectangle(ix - w/2, iy - h, w, h, (Color){ 120, 80, 45, 255 });
    DrawRectangle(ix - w/2, iy - h + 2, w, 2, (Color){ 95, 60, 30, 255 });
    // Cut tree ring end
    DrawEllipse(ix - w/2, iy - h/2, (int)(2 * scale), (int)(h/2), (Color){ 180, 140, 90, 255 });
    DrawCircle(ix - w/2, iy - h/2, 1, (Color){ 110, 75, 40, 255 });
}

static void DrawMountainPeak(float x, float y, float scale) {
    int ix = (int)x;
    int iy = (int)y;
    int w = (int)(28 * scale);
    int h = (int)(38 * scale);

    // Mountain silhouette
    Vector2 top = { (float)ix, (float)(iy - h) };
    Vector2 bl  = { (float)(ix - w), (float)iy };
    Vector2 br  = { (float)(ix + w), (float)iy };

    // Shaded side (dark slate)
    DrawTriangle(top, bl, br, (Color){ 75, 72, 82, 255 });
    // Lighted side (lighter granite facet)
    DrawTriangle(top, bl, (Vector2){ (float)ix, (float)iy }, (Color){ 115, 112, 122, 255 });

    // Snow Cap on peak!
    Vector2 snow_l = { (float)(ix - w * 0.32f), (float)(iy - h * 0.68f) };
    Vector2 snow_r = { (float)(ix + w * 0.32f), (float)(iy - h * 0.68f) };
    DrawTriangle(top, snow_l, snow_r, (Color){ 242, 246, 255, 255 });
    DrawTriangle(top, snow_l, (Vector2){ (float)ix, (float)(iy - h * 0.68f) }, WHITE);
}

static void DrawBoulder(float x, float y, float scale, int variant) {
    (void)variant;
    int ix = (int)x;
    int iy = (int)y;
    int r = (int)(9 * scale);

    // Ground shadow
    DrawEllipse(ix + 1, iy + 2, r + 2, (int)(r * 0.5f), (Color){ 40, 35, 30, 130 });

    // Polygonal rock body
    DrawRectangle(ix - r, iy - (int)(r * 1.2f), r * 2, (int)(r * 1.2f), (Color){ 80, 78, 85, 255 });
    // Facet highlights
    DrawTriangle((Vector2){ ix - r, iy }, (Vector2){ ix, iy - r * 1.3f }, (Vector2){ ix, iy }, (Color){ 125, 122, 132, 255 });
    DrawTriangle((Vector2){ ix, iy - r * 1.3f }, (Vector2){ ix + r, iy }, (Vector2){ ix, iy }, (Color){ 95, 92, 102, 255 });
    // Specular top edge
    DrawLine(ix - r/2, iy - (int)(r * 1.2f), ix + r/2, iy - (int)(r * 1.1f), (Color){ 165, 162, 175, 255 });
}

static void DrawAlpinePine(float x, float y, float scale) {
    int ix = (int)x;
    int iy = (int)y;

    // Rock base
    DrawBoulder(x, y + 2, scale * 0.8f, 0);

    // Weathered high mountain spruce
    int h = (int)(26 * scale);
    DrawLine(ix, iy, ix, iy - h, (Color){ 80, 55, 35, 255 });
    DrawTriangle((Vector2){ ix, iy - h }, (Vector2){ ix - 8 * scale, iy - 6 * scale }, (Vector2){ ix + 8 * scale, iy - 6 * scale }, (Color){ 30, 70, 45, 255 });
    DrawTriangle((Vector2){ ix, iy - h }, (Vector2){ ix, iy - 6 * scale }, (Vector2){ ix + 8 * scale, iy - 6 * scale }, (Color){ 50, 110, 65, 255 });
}

static void DrawRockPillar(float x, float y, float scale) {
    int ix = (int)x;
    int iy = (int)y;
    int w = (int)(12 * scale);
    int h = (int)(28 * scale);

    // Sandstone tower
    DrawRectangle(ix - w/2, iy - h, w, h, (Color){ 135, 105, 80, 255 });
    DrawRectangle(ix - w/2, iy - h, w/2, h, (Color){ 165, 130, 100, 255 });
    // Horizontal strata layers
    for (int ly = iy - h + 4; ly < iy; ly += 6) {
        DrawLine(ix - w/2, ly, ix + w/2, ly, (Color){ 105, 78, 55, 255 });
    }
}

static void DrawStreetLamp(float x, float y, float scale, int side) {
    int ix = (int)x;
    int iy = (int)y;
    int h = (int)(26 * scale);
    int arm_dir = (side == -1) ? 1 : -1; // Arch towards the road

    // Steel pole
    Color steel = (Color){ 160, 168, 178, 255 };
    DrawRectangle(ix - 1, iy - h, 2, h, steel);
    // Base bracket
    DrawRectangle(ix - 2, iy - 2, 4, 3, (Color){ 90, 95, 105, 255 });

    // Curved overhead arm
    int arm_w = (int)(10 * scale);
    DrawLine(ix, iy - h, ix + arm_dir * arm_w, iy - h - 2, steel);

    // Lamp fixture head
    int hx = ix + arm_dir * arm_w;
    int hy = iy - h - 2;
    DrawRectangle(hx - 2, hy, 4, 2, (Color){ 110, 115, 125, 255 });

    // Halogen glow bulb
    DrawCircle(hx, hy + 2, 2, YELLOW);
    DrawCircle(hx, hy + 2, 1, WHITE);

    // Soft light cone pool on ground
    DrawEllipse(hx, iy, (int)(12 * scale), (int)(5 * scale), (Color){ 255, 240, 150, 45 });
}

static void DrawSkyscraper(float x, float y, float scale, int variant) {
    int ix = (int)x;
    int iy = (int)y;
    int w = (int)((20 + (variant % 3) * 6) * scale);
    int h = (int)((34 + (variant * 8)) * scale);

    Color bldg_body = (variant % 2 == 0) ? (Color){ 36, 40, 54, 255 } : (Color){ 48, 44, 62, 255 };
    Color bldg_edge = (Color){ 65, 72, 92, 255 };

    // Building facade
    DrawRectangle(ix - w/2, iy - h, w, h, bldg_body);
    DrawRectangleLines(ix - w/2, iy - h, w, h, bldg_edge);

    // Window grid
    Color win_lit_a = (Color){ 255, 225, 90, 255 }; // Warm lit office
    Color win_lit_b = (Color){ 80, 220, 255, 255 };  // Cool cyan screen glow
    Color win_dark  = (Color){ 22, 25, 34, 255 };

    for (int r = iy - h + 4; r < iy - 4; r += 5) {
        for (int c = ix - w/2 + 3; c < ix + w/2 - 3; c += 4) {
            int seed = (r * 13 + c * 7);
            Color win_col = (seed % 3 == 0) ? win_lit_a :
                            ((seed % 5 == 0) ? win_lit_b : win_dark);
            DrawRectangle(c, r, 2, 3, win_col);
        }
    }

    // Rooftop antenna with blinking warning beacon
    int ax = ix;
    int ay = iy - h;
    DrawLine(ax, ay, ax, ay - 6, (Color){ 160, 160, 170, 255 });
    bool blink = (((int)(GetTime() * 4.0f) + variant) % 2 == 0);
    DrawCircle(ax, ay - 7, 1, blink ? RED : (Color){ 80, 0, 0, 255 });
}

static void DrawNeonBillboard(float x, float y, float scale, int variant) {
    int ix = (int)x;
    int iy = (int)y;
    int w = (int)(26 * scale);
    int h = (int)(16 * scale);

    // Twin steel support beams
    DrawLine(ix - w/3, iy, ix - w/3, iy - h, (Color){ 130, 135, 145, 255 });
    DrawLine(ix + w/3, iy, ix + w/3, iy - h, (Color){ 130, 135, 145, 255 });

    // Billboard dark backing
    DrawRectangle(ix - w/2, iy - h - 8, w, 12, (Color){ 15, 12, 22, 255 });

    // Glowing Neon Border
    Color neon_border = (variant % 2 == 0) ? (Color){ 255, 40, 150, 255 } : (Color){ 40, 225, 255, 255 };
    DrawRectangleLines(ix - w/2, iy - h - 8, w, 12, neon_border);

    // Neon text label
    const char *text = (variant == 0) ? "ROAD" :
                       ((variant == 1) ? "80s" :
                       ((variant == 2) ? "NEON" :
                       ((variant == 3) ? "HOTEL" : "SPEED")));
    DrawText(text, ix - w/2 + 3, iy - h - 6, 8, neon_border);
}

static void DrawFinishArch(float x, float y, float scale) {
    (void)scale;
    int ix = (int)x;
    int iy = (int)y;

    // Overhead truss spanning the highway
    int arch_w = (int)(ROAD_WIDTH + 24.0f);
    int arch_h = 24;

    // Side metal pillars
    DrawRectangle(ix - arch_w/2 - 4, iy - arch_h, 5, arch_h, (Color){ 180, 185, 195, 255 });
    DrawRectangle(ix + arch_w/2,     iy - arch_h, 5, arch_h, (Color){ 180, 185, 195, 255 });

    // Overhead Banner
    DrawRectangle(ix - arch_w/2, iy - arch_h, arch_w, 14, BLACK);
    DrawRectangleLines(ix - arch_w/2, iy - arch_h, arch_w, 14, GOLD);

    // Checkered pattern on banner top
    for (int bx = ix - arch_w/2 + 2; bx < ix + arch_w/2 - 4; bx += 8) {
        DrawRectangle(bx, iy - arch_h + 1, 4, 3, WHITE);
        DrawRectangle(bx + 4, iy - arch_h + 4, 4, 3, WHITE);
    }

    // Text "FINISH / GOAL"
    const char *msg = "== GOAL - FINISH ==";
    int tw = MeasureText(msg, 10);
    DrawText(msg, ix - tw/2, iy - arch_h + 4, 10, YELLOW);
}

// ========================================================
// Main Draw Routine
// ========================================================

void Scenery_Draw(const ScenerySystem *ss, const struct Road *r, float player_dist, float player_y) {
    for (int i = 0; i < ss->count; i++) {
        const SceneryObject *obj = &ss->objects[i];

        float dist_diff = obj->track_dist - player_dist;
        // Cull objects outside screen vertical bounds
        if (dist_diff < -80.0f || dist_diff > 260.0f) continue;

        float screen_y = player_y - dist_diff * 0.95f;
        if (screen_y < -35.0f || screen_y > VIRTUAL_H + 35.0f) continue;

        float road_center = Road_GetCenter(r, obj->track_dist);
        float screen_x;

        if (obj->side == 0) {
            // Centered overhead (e.g. Finish Arch)
            screen_x = road_center;
        } else if (obj->side == -1) {
            // Left of the road curb
            screen_x = road_center - (ROAD_WIDTH / 2.0f) - ROAD_CURB_WIDTH - 4.0f - obj->lateral_offset;
        } else {
            // Right of the road curb
            screen_x = road_center + (ROAD_WIDTH / 2.0f) + ROAD_CURB_WIDTH + 4.0f + obj->lateral_offset;
        }

        switch (obj->type) {
            case SCENERY_PALM_TREE:
                DrawPalmTree(screen_x, screen_y, obj->scale, obj->side);
                break;
            case SCENERY_SUN_UMBRELLA:
                DrawSunUmbrella(screen_x, screen_y, obj->scale, obj->variant);
                break;
            case SCENERY_BEACH_TOWEL:
                DrawBeachTowel(screen_x, screen_y, obj->scale, obj->variant);
                break;
            case SCENERY_SURFBOARD:
                DrawSurfboard(screen_x, screen_y, obj->scale, obj->variant);
                break;
            case SCENERY_PINE_TREE:
                DrawPineTree(screen_x, screen_y, obj->scale);
                break;
            case SCENERY_OAK_TREE:
                DrawOakTree(screen_x, screen_y, obj->scale);
                break;
            case SCENERY_BUSH:
                DrawBush(screen_x, screen_y, obj->scale, obj->variant);
                break;
            case SCENERY_WOOD_LOG:
                DrawWoodLog(screen_x, screen_y, obj->scale);
                break;
            case SCENERY_MOUNTAIN_PEAK:
                DrawMountainPeak(screen_x, screen_y, obj->scale);
                break;
            case SCENERY_BOULDER:
                DrawBoulder(screen_x, screen_y, obj->scale, obj->variant);
                break;
            case SCENERY_ALPINE_PINE:
                DrawAlpinePine(screen_x, screen_y, obj->scale);
                break;
            case SCENERY_ROCK_PILLAR:
                DrawRockPillar(screen_x, screen_y, obj->scale);
                break;
            case SCENERY_STREET_LAMP:
                DrawStreetLamp(screen_x, screen_y, obj->scale, obj->side);
                break;
            case SCENERY_SKYSCRAPER:
                DrawSkyscraper(screen_x, screen_y, obj->scale, obj->variant);
                break;
            case SCENERY_NEON_BILLBOARD:
                DrawNeonBillboard(screen_x, screen_y, obj->scale, obj->variant);
                break;
            case SCENERY_FINISH_ARCH:
                DrawFinishArch(screen_x, screen_y, obj->scale);
                break;
        }
    }
}
