#include "raylib.h"
#include <time.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 1920
#define HEIGHT 1000

#define NUM_PARTICLES 2000
#define GRAVITY 0.1
#define DAMPING 0.98
#define SPEED 15

typedef struct {
    float x, y, r, vx, vy;
} Particle;

Particle particles[NUM_PARTICLES];

void UpdateParticle(Particle *particle)
{
    particle->vy += GRAVITY;
    particle->x += particle->vx;
    particle->y += particle->vy;

    float x = particle->x;
    float y = particle->y;
    float r = particle->r;

    bool collision = false;

    if(x-r < 0){
        particle->x = r;
        particle->vx = -particle->vx;
        collision = true;
    }
    if(x+r > WIDTH){
        particle->x = WIDTH-r;
        particle->vx = -particle->vx;
        collision = true;
    }
    if(y+r > HEIGHT){
        particle->y = HEIGHT-r;
        particle->vy = -particle->vy;
        collision = true;
    }
    if(y-r < 0){
        particle->y = r;
        particle->vy = -particle->vy;
        collision = true;
    }

    if(collision == true){
        particle->vx *= DAMPING;
        particle->vy *= DAMPING;
    }
}



void DrawParticle(Particle *particle)
{
    DrawCircle(particle->x, particle->y, particle->r, WHITE);
}

void DrawParticles(){
    for(int i=0; i<NUM_PARTICLES; i++){
        DrawParticle(particles+i);
    }
}

void UpdateParticles(){
    for(int i=0; i<NUM_PARTICLES; i++){
        UpdateParticle(particles+i);
    }
}

void CollideAllParticles(){
    Particle p1;
    Particle p2;
    for(int i=0; i<NUM_PARTICLES; i++){
        for(int j=i+1; j<NUM_PARTICLES; j++){
            if(i == j){
                break;
            }
            p1 = particles[i];
            p2 = particles[j];

            Vector2 c1 = {p1.x, p1.y};
            Vector2 c2 = {p2.x, p2.y};
            bool collide = CheckCollisionCircles(c1, p1.r, c2, p2.r);

            if(collide == true){
                // oh boy.....
                float dx = (p1.x - p2.x);
                float dy = (p1.y - p2.y);

                float abs_d = sqrt(pow(dx,2) + pow(dy,2));
                float nx = dx/abs_d;
                float ny = dy/abs_d;

                float overlap = ((p2.r+p1.r)-abs_d);
                particles[i].x += nx*overlap/2;
                particles[i].y += ny*overlap/2;
                particles[j].x += -nx*overlap/2;
                particles[j].y += -ny*overlap/2;

                float tx = -ny;
                float ty = nx;

                float v1t = p1.vx*tx + p1.vy*ty;
                float v2t = p2.vx*tx + p2.vy*ty;
                float v1n = p1.vx*nx + p1.vy*ny; 
                float v2n = p2.vx*nx + p2.vy*ny;

                float vflip = v2n;
                v2n = v1n;
                v1n = vflip;

                particles[i].vx = v1t;
                particles[i].vy = v1n;
                particles[j].vx = v2t;
                particles[j].vy = v2n;

                if(collide == true){
                    particles[i].vx *= DAMPING;
                    particles[i].vy *= DAMPING;
                    particles[j].vx *= DAMPING;
                    particles[j].vy *= DAMPING;
                }
            }
        }
    }
}

void InitParticles(){
    SetRandomSeed(time(NULL));

    float radius;
    for(int i=0; i<NUM_PARTICLES; i++){
        radius = GetRandomValue(5, 5);
        particles[i].r = radius;
        particles[i].x = GetRandomValue(radius, WIDTH-radius);
        particles[i].y = GetRandomValue(radius, HEIGHT-radius);
        particles[i].vx = GetRandomValue(-SPEED, SPEED);
        particles[i].vy = GetRandomValue(-SPEED, SPEED);
    }
}

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Particle Simulation");

    InitParticles();
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        UpdateParticles();
        CollideAllParticles();
        DrawParticles();
        DrawFPS(5, 5);
        EndDrawing();
    }

    CloseWindow(); 

    return 0;
}