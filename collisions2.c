// Optimized particle simulation using raylib
// Key improvements:
// - Spatial partitioning (uniform grid) to reduce O(N^2) collision checks
// - Fewer sqrt/pow calls (use squared distance where possible)
// - Reduced copies of Particle structs
// - Minor math and loop optimizations

#include "raylib.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>

#define WIDTH 1920
#define HEIGHT 1000

#define NUM_PARTICLES 5000
#define GRAVITY 0.1
#define DAMPING 0.98
#define SPEED 5.0f

#define CELL_SIZE 20        // Size of spatial grid cell
#define MAX_IN_CELL 64     // Max particles per cell

// Grid dimensions
#define GRID_W (WIDTH / CELL_SIZE + 1)
#define GRID_H (HEIGHT / CELL_SIZE + 1)

typedef struct {
    float x, y, r, vx, vy;
} Particle;

// Spatial grid
static int grid[GRID_W][GRID_H][MAX_IN_CELL];
static int gridCount[GRID_W][GRID_H];

static Particle particles[NUM_PARTICLES];

// ------------------ Utility ------------------

static inline int ClampInt(int v, int min, int max)
{
    return (v < min) ? min : (v > max) ? max : v;
}

static inline float DistSq(float dx, float dy)
{
    return dx * dx + dy * dy;
}

// ------------------ Particles ------------------

void UpdateParticle(Particle *p)
{
    p->vy += GRAVITY;
    p->x += p->vx;
    p->y += p->vy;

    bool collision = false;

    if (p->x - p->r < 0) {
        p->x = p->r;
        p->vx = -p->vx;
        collision = true;
    }
    if (p->x + p->r > WIDTH) {
        p->x = WIDTH - p->r;
        p->vx = -p->vx;
        collision = true;
    }
    if (p->y + p->r > HEIGHT) {
        p->y = HEIGHT - p->r;
        p->vy = -p->vy;
        collision = true;
    }
    if (p->y - p->r < 0) {
        p->y = p->r;
        p->vy = -p->vy;
        collision = true;
    }

    if (collision) {
        p->vx *= DAMPING;
        p->vy *= DAMPING;
    }
}

void DrawParticles(void)
{
    for (int i = 0; i < NUM_PARTICLES; i++) {
        DrawCircleV((Vector2){ particles[i].x, particles[i].y }, particles[i].r, WHITE);
    }
}

// ------------------ Spatial Grid ------------------

void ClearGrid(void)
{
    for (int x = 0; x < GRID_W; x++)
        for (int y = 0; y < GRID_H; y++)
            gridCount[x][y] = 0;
}

void InsertToGrid(int index)
{
    int gx = ClampInt((int)(particles[index].x / CELL_SIZE), 0, GRID_W - 1);
    int gy = ClampInt((int)(particles[index].y / CELL_SIZE), 0, GRID_H - 1);

    int count = gridCount[gx][gy];
    if (count < MAX_IN_CELL) {
        grid[gx][gy][count] = index;
        gridCount[gx][gy]++;
    }
}

// ------------------ Collisions ------------------

void ResolveCollision(int i, int j)
{
    Particle *p1 = &particles[i];
    Particle *p2 = &particles[j];

    float dx = p1->x - p2->x;
    float dy = p1->y - p2->y;

    float r = p1->r + p2->r;
    float distSq = DistSq(dx, dy);

    if (distSq >= r * r || distSq == 0.0f)
        return;

    float dist = sqrtf(distSq);
    float nx = dx / dist;
    float ny = dy / dist;

    float overlap = r - dist;

    // Separate
    p1->x += nx * overlap * 0.5f;
    p1->y += ny * overlap * 0.5f;
    p2->x -= nx * overlap * 0.5f;
    p2->y -= ny * overlap * 0.5f;

    // Tangent
    float tx = -ny;
    float ty = nx;

    float v1t = p1->vx * tx + p1->vy * ty;
    float v2t = p2->vx * tx + p2->vy * ty;

    float v1n = p1->vx * nx + p1->vy * ny;
    float v2n = p2->vx * nx + p2->vy * ny;

    // Swap normal velocities (elastic collision, equal mass)
    float tmp = v1n;
    v1n = v2n;
    v2n = tmp;

    p1->vx = v1t * tx + v1n * nx;
    p1->vy = v1t * ty + v1n * ny;

    p2->vx = v2t * tx + v2n * nx;
    p2->vy = v2t * ty + v2n * ny;

    p1->vx *= DAMPING;
    p1->vy *= DAMPING;
    p2->vx *= DAMPING;
    p2->vy *= DAMPING;
}

void CollideAllParticles(void)
{
    ClearGrid();

    // Insert particles into grid
    for (int i = 0; i < NUM_PARTICLES; i++)
        InsertToGrid(i);

    // Check collisions in neighboring cells
    for (int gx = 0; gx < GRID_W; gx++) {
        for (int gy = 0; gy < GRID_H; gy++) {
            for (int i = 0; i < gridCount[gx][gy]; i++) {
                int pIndex = grid[gx][gy][i];

                for (int ox = -1; ox <= 1; ox++) {
                    for (int oy = -1; oy <= 1; oy++) {
                        int nx = gx + ox;
                        int ny = gy + oy;

                        if (nx < 0 || ny < 0 || nx >= GRID_W || ny >= GRID_H)
                            continue;

                        for (int j = 0; j < gridCount[nx][ny]; j++) {
                            int other = grid[nx][ny][j];
                            if (other <= pIndex) continue;

                            ResolveCollision(pIndex, other);
                        }
                    }
                }
            }
        }
    }
}

// ------------------ Init ------------------

void InitParticles(void)
{
    SetRandomSeed(time(NULL));

    for (int i = 0; i < NUM_PARTICLES; i++) {
        float radius = 5.0f;

        particles[i].r = radius;
        particles[i].x = (float)GetRandomValue((int)radius, WIDTH - (int)radius);
        particles[i].y = (float)GetRandomValue((int)radius, HEIGHT - (int)radius);
        particles[i].vx = (float)GetRandomValue(-(int)SPEED, (int)SPEED);
        particles[i].vy = (float)GetRandomValue(-(int)SPEED, (int)SPEED);
    }
}

// ------------------ Main ------------------

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Optimized Particle Simulation");

    InitParticles();
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < NUM_PARTICLES; i++)
            UpdateParticle(&particles[i]);

        CollideAllParticles();
        DrawParticles();
        DrawFPS(5, 5);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
