#ifndef PARTICLES_H
#define PARTICLES_H

struct Particle
{
    v3 position;
    v3 speed;
    v4 color;
    r32 size;
    r32 angle;
    r32 weight;
    r32 life;
};

struct Particles
{
    Particle *data;
    u32 count;
    u32 max;
    Mesh mesh; // particle mesh
    
    u32 opengl_buffer;
    u32 opengl_life_buffer;
};

Particles particles = {};

#endif //PARTICLES_H