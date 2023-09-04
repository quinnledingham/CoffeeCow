function void
update_particles_model_buffer(Particles *particles)
{
    m4x4 *models = ARRAY_MALLOC(m4x4, particles->count);
    r32 *lifes = ARRAY_MALLOC(r32, particles->count);
    u32 models_index = 0;
    for (u32 i = 0; i < particles->max; i++)
    {
        if (particles->data[i].life <= 0) continue;
        v3 dim = { particles->data[i].weight, particles->data[i].weight, 1 };
        m4x4 model = create_transform_m4x4(particles->data[i].position, get_rotation(0.0f, { 0, 0, 1 }), dim);
        lifes[models_index] = particles->data[i].life;
        models[models_index++] = model;
    }

    glBindBuffer(GL_ARRAY_BUFFER, particles->opengl_buffer);
    glBufferData(GL_ARRAY_BUFFER, particles->max * sizeof(m4x4), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particles->count * sizeof(m4x4), &models[0]);

    glBindBuffer(GL_ARRAY_BUFFER, particles->opengl_life_buffer);
    glBufferData(GL_ARRAY_BUFFER, particles->max * sizeof(r32), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particles->count * sizeof(r32), &lifes[0]);

    free(models);
    free(lifes);
}

function void
init_particles(Particles *particles, u32 particles_max)
{
    particles->data = ARRAY_MALLOC(Particle, particles_max);
    SDL_memset(particles->data, 0, sizeof(Particle) * particles_max);
    particles->max = particles_max;
    init_rect_mesh(&particles->mesh);

    glGenBuffers(1, &particles->opengl_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles->opengl_buffer);
    glBufferData(GL_ARRAY_BUFFER, particles->max * sizeof(m4x4), NULL, GL_STATIC_DRAW);
    
    // create a 4x4 matrix in the buffer using 4 v4s
    glBindVertexArray(particles->mesh.vao);

    glEnableVertexAttribArray(3); 
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(v4), (void*)0);
    glEnableVertexAttribArray(4); 
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(v4), (void*)(1 * sizeof(v4)));
    glEnableVertexAttribArray(5); 
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(v4), (void*)(2 * sizeof(v4)));
    glEnableVertexAttribArray(6); 
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(v4), (void*)(3 * sizeof(v4)));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glGenBuffers(1, &particles->opengl_life_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles->opengl_life_buffer);
    glBufferData(GL_ARRAY_BUFFER, particles->max * sizeof(r32), NULL, GL_STATIC_DRAW);

    glEnableVertexAttribArray(7); 
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(r32), (void*)0);
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0);
}

function void
add_particle(Particles *particles, v3 position, r32 angle, r32 weight)
{
    Particle *part = 0;
    for (u32 i = 0; i < particles->max; i++)
    {
        if (particles->data[i].life <= 0)
        {
            part = &particles->data[i];
            break;
        }
    }
    if (part == 0) return;

    part->life = 0.5f;
    part->position = position;
    part->angle = angle;
    part->weight = weight;
    particles->count++;
}

function v3
get_unit(r32 degrees)
{
    if      (degrees == 90.0f)  return { 0.0f,  1.0f, 0.0f };
    else if (degrees == 270.0f) return { 0.0f, -1.0f, 0.0f };

    r32 radians = DEG2RAD * degrees;
    return { cosf(radians), sinf(radians), 0.0f };
}

function void
update_particles(Particles *particles, r32 frame_time_s)
{
    r32 delta_life = frame_time_s;

    for (u32 i = 0; i < particles->max; i++)
    {
        Particle *part = &particles->data[i];
        if (part->life > 0.0f) 
        {
            part->life -= delta_life;

            if (part->life <= 0.0f) particles->count--;

            r32 radians = DEG2RAD * part->angle;

            part->position += get_unit(part->angle) * 0.01f;

        }
    }

    update_particles_model_buffer(particles);

}

function void
draw_particles(Particles *particles)
{
    local_persist Shader shader = {};
    if (!shader.compiled)
    {
        shader.vs_filename = "../assets/shaders/particle.vs";
        shader.fs_filename = "../assets/shaders/particle.fs";
        load_shader(&shader);
        compile_shader(&shader);
    }
    
    u32 handle = use_shader(&shader);
    glBindVertexArray(particles->mesh.vao);
    v4 color = { 0, 255, 0, 1 };
    glUniform4fv(glGetUniformLocation(handle, "user_color"), (GLsizei)1, (float*)&color);
    glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), (GLsizei)1, false, (float*)&orthographic_matrix);
    glUniformMatrix4fv(glGetUniformLocation(handle, "view"), (GLsizei)1, false, (float*)&identity_m4x4());
    glDrawElementsInstanced(GL_TRIANGLES, particles->mesh.indices_count, GL_UNSIGNED_INT, 0, particles->count);
}