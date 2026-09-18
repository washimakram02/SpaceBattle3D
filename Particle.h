#ifndef PARTICLE_H
#define PARTICLE_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>

enum ParticleType {
    PARTICLE_SPARK,
    PARTICLE_DEBRIS,
    PARTICLE_SMOKE,
    PARTICLE_SHOCKWAVE
};

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float r, g, b, a;
    float size;
    float life;
    float maxLife;
    ParticleType type;
    float shockRadius;
    float maxShockRadius;
};

class ParticleSystem {
public:
    std::vector<Particle> particles;

    void update(float dt) {
        for (size_t i = 0; i < particles.size(); ) {
            Particle& p = particles[i];
            p.life += dt;
            if (p.life >= p.maxLife) {
                particles.erase(particles.begin() + i);
                continue;
            }

            float t = p.life / p.maxLife;

            if (p.type == PARTICLE_SHOCKWAVE) {
                p.shockRadius = p.maxShockRadius * t;
                p.a = 1.0f - t;
            } else {
                p.x += p.vx * dt;
                p.y += p.vy * dt;
                p.z += p.vz * dt;

                // Drag / dampening
                p.vx *= (1.0f - 1.2f * dt);
                p.vy *= (1.0f - 1.2f * dt);
                p.vz *= (1.0f - 1.2f * dt);

                p.a = 1.0f - (t * t); // Smooth fade
            }
            ++i;
        }
    }

    void draw() const {
        if (particles.empty()) return;

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow
        glDepthMask(GL_FALSE); // Don't write to depth buffer for transparent particles

        for (const auto& p : particles) {
            if (p.type == PARTICLE_SHOCKWAVE) {
                glPushMatrix();
                glTranslatef(p.x, p.y, p.z);
                glColor4f(p.r, p.g, p.b, p.a * 0.8f);
                glLineWidth(2.5f);
                
                // Draw expanding ring
                glBegin(GL_LINE_LOOP);
                const int segments = 24;
                for (int s = 0; s < segments; ++s) {
                    float theta = 2.0f * 3.14159265f * float(s) / float(segments);
                    float rx = p.shockRadius * cosf(theta);
                    float ry = p.shockRadius * sinf(theta);
                    glVertex3f(rx, ry, 0.0f);
                }
                glEnd();
                glLineWidth(1.0f);
                glPopMatrix();
            } else {
                glPushMatrix();
                glTranslatef(p.x, p.y, p.z);
                glColor4f(p.r, p.g, p.b, p.a);

                if (p.type == PARTICLE_SPARK) {
                    glutSolidSphere(p.size * (1.0f - 0.5f * (p.life / p.maxLife)), 6, 6);
                } else if (p.type == PARTICLE_DEBRIS) {
                    glutSolidCube(p.size);
                } else {
                    glutSolidSphere(p.size * (1.0f + 1.2f * (p.life / p.maxLife)), 6, 6);
                }
                glPopMatrix();
            }
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    void addExplosion(float x, float y, float z, int count = 35,
                      float r = 1.0f, float g = 0.5f, float b = 0.1f) {
        // Shockwave
        addShockwave(x, y, z, 7.5f, r, g, b);

        for (int i = 0; i < count; ++i) {
            Particle p;
            p.x = x;
            p.y = y;
            p.z = z;

            // Random sphere vector
            float u = (float)rand() / RAND_MAX * 2.0f - 1.0f;
            float phi = (float)rand() / RAND_MAX * 2.0f * 3.14159265f;
            float speed = 12.0f + (float)rand() / RAND_MAX * 28.0f;

            p.vx = sqrtf(1.0f - u * u) * cosf(phi) * speed;
            p.vy = sqrtf(1.0f - u * u) * sinf(phi) * speed;
            p.vz = u * speed;

            p.r = r + ((float)rand() / RAND_MAX * 0.3f - 0.15f);
            p.g = g + ((float)rand() / RAND_MAX * 0.3f - 0.15f);
            p.b = b + ((float)rand() / RAND_MAX * 0.2f);
            if (p.r > 1.0f) p.r = 1.0f;
            if (p.g > 1.0f) p.g = 1.0f;
            if (p.b > 1.0f) p.b = 1.0f;

            p.a = 1.0f;
            p.size = 0.25f + ((float)rand() / RAND_MAX * 0.35f);
            p.life = 0.0f;
            p.maxLife = 0.6f + ((float)rand() / RAND_MAX * 0.9f);
            p.type = (rand() % 3 == 0) ? PARTICLE_DEBRIS : PARTICLE_SPARK;
            p.shockRadius = 0.0f;
            p.maxShockRadius = 0.0f;

            particles.push_back(p);
        }
    }

    void addShockwave(float x, float y, float z, float maxRadius = 8.0f,
                      float r = 0.4f, float g = 0.8f, float b = 1.0f) {
        Particle p;
        p.x = x; p.y = y; p.z = z;
        p.vx = p.vy = p.vz = 0.0f;
        p.r = r; p.g = g; p.b = b; p.a = 1.0f;
        p.size = 0.0f;
        p.life = 0.0f;
        p.maxLife = 0.45f;
        p.type = PARTICLE_SHOCKWAVE;
        p.shockRadius = 0.1f;
        p.maxShockRadius = maxRadius;
        particles.push_back(p);
    }

    void addTrail(float x, float y, float z, float r = 0.2f, float g = 0.7f, float b = 1.0f, float size = 0.3f) {
        Particle p;
        p.x = x + ((float)rand() / RAND_MAX * 0.2f - 0.1f);
        p.y = y + ((float)rand() / RAND_MAX * 0.2f - 0.1f);
        p.z = z;
        p.vx = ((float)rand() / RAND_MAX * 2.0f - 1.0f);
        p.vy = ((float)rand() / RAND_MAX * 2.0f - 1.0f);
        p.vz = 4.0f + ((float)rand() / RAND_MAX * 6.0f); // Emitted backward
        p.r = r; p.g = g; p.b = b; p.a = 0.7f;
        p.size = size;
        p.life = 0.0f;
        p.maxLife = 0.25f + ((float)rand() / RAND_MAX * 0.15f);
        p.type = PARTICLE_SPARK;
        p.shockRadius = 0.0f;
        p.maxShockRadius = 0.0f;
        particles.push_back(p);
    }

    void clear() {
        particles.clear();
    }
};

#endif // PARTICLE_H
