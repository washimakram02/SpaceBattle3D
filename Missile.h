#ifndef MISSILE_H
#define MISSILE_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/glut.h>
#include <cmath>
#include "Particle.h"

class Missile {
public:
    float x, y, z;
    float vx, vy, vz;
    float speed;
    float maxSpeed;
    float acceleration;
    float turnRate;

    float targetX, targetY, targetZ;
    bool active;
    int damage;
    float blastRadius;
    float lifetime;
    float maxLifetime;
    float radius;

    // Visual orientation
    float pitch;
    float yaw;
    float roll;
    float enginePulse;

    Missile()
        : x(0), y(0), z(0),
          vx(0), vy(0), vz(-80.0f),
          speed(80.0f), maxSpeed(210.0f),
          acceleration(65.0f), turnRate(4.8f),
          targetX(0), targetY(0), targetZ(-100.0f),
          active(false), damage(220), blastRadius(22.0f),
          lifetime(0.0f), maxLifetime(4.5f), radius(1.1f),
          pitch(0), yaw(0), roll(0), enginePulse(0) {}

    Missile(float startX, float startY, float startZ,
            float aimX, float aimY, float aimZ,
            float initialSpeed = 75.0f)
        : x(startX), y(startY), z(startZ),
          speed(initialSpeed), maxSpeed(210.0f),
          acceleration(70.0f), turnRate(4.8f),
          targetX(aimX), targetY(aimY), targetZ(aimZ),
          active(true), damage(220), blastRadius(22.0f),
          lifetime(0.0f), maxLifetime(4.5f), radius(1.1f),
          pitch(0), yaw(0), roll(0), enginePulse(0) {

        // Initial launch direction: forward with slight convergence towards aim
        float dx = targetX - x;
        float dy = targetY - y;
        float dz = targetZ - z;
        float len = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (len > 0.001f) {
            vx = (dx / len) * speed;
            vy = (dy / len) * speed;
            vz = (dz / len) * speed;
        } else {
            vx = 0.0f;
            vy = 0.0f;
            vz = -speed;
        }
        updateAngles();
    }

    void setTarget(float tx, float ty, float tz) {
        targetX = tx;
        targetY = ty;
        targetZ = tz;
    }

    void updateAngles() {
        // Compute yaw and pitch from current velocity
        float horizDist = std::sqrt(vx * vx + vz * vz);
        yaw = std::atan2(-vx, -vz) * 57.2957795f; // Rad to deg
        pitch = std::atan2(vy, horizDist) * 57.2957795f;
    }

    void update(float dt, ParticleSystem& particles) {
        if (!active) return;

        enginePulse += dt * 30.0f;
        lifetime += dt;
        if (lifetime >= maxLifetime) {
            active = false;
            return;
        }

        // Accelerate up to max speed
        if (speed < maxSpeed) {
            speed += acceleration * dt;
            if (speed > maxSpeed) speed = maxSpeed;
        }

        // Homing Guidance: steer velocity vector toward target
        float tdx = targetX - x;
        float tdy = targetY - y;
        float tdz = targetZ - z;
        float targetDist = std::sqrt(tdx * tdx + tdy * tdy + tdz * tdz);

        if (targetDist > 1.0f) {
            float desiredVx = (tdx / targetDist) * speed;
            float desiredVy = (tdy / targetDist) * speed;
            float desiredVz = (tdz / targetDist) * speed;

            // Smoothly steer velocity
            float steer = turnRate * dt;
            if (steer > 0.35f) steer = 0.35f;
            vx += (desiredVx - vx) * steer;
            vy += (desiredVy - vy) * steer;
            vz += (desiredVz - vz) * steer;

            // Re-normalize velocity to current speed
            float curLen = std::sqrt(vx * vx + vy * vy + vz * vz);
            if (curLen > 0.001f) {
                vx = (vx / curLen) * speed;
                vy = (vy / curLen) * speed;
                vz = (vz / curLen) * speed;
            }
        }

        // Position integration
        x += vx * dt;
        y += vy * dt;
        z += vz * dt;

        // Roll spinning effect for aerodynamic stability
        roll += 360.0f * dt;
        if (roll >= 360.0f) roll -= 360.0f;

        updateAngles();

        // Dense Rocket Thruster Exhaust Particles
        // Fiery exhaust sparks
        particles.addTrail(x - (vx / speed) * 0.8f,
                           y - (vy / speed) * 0.8f,
                           z - (vz / speed) * 0.8f,
                           1.0f, 0.55f, 0.1f, 0.45f);

        // Billowing white/grey smoke puffs
        if (rand() % 2 == 0) {
            particles.addTrail(x - (vx / speed) * 1.3f + ((float)rand()/RAND_MAX * 0.2f - 0.1f),
                               y - (vy / speed) * 1.3f + ((float)rand()/RAND_MAX * 0.2f - 0.1f),
                               z - (vz / speed) * 1.3f,
                               0.85f, 0.85f, 0.9f, 0.6f);
        }
    }

    void draw() const {
        if (!active) return;

        glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(roll, 0.0f, 0.0f, 1.0f);

        // --- Materials for High-Tech Heavy Missile ---
        GLfloat bodyDiffuse[]    = { 0.25f, 0.28f, 0.32f, 1.0f }; // Gunmetal grey
        GLfloat warheadDiffuse[] = { 0.95f, 0.20f, 0.15f, 1.0f }; // Crimson red explosive warhead
        GLfloat finDiffuse[]     = { 0.15f, 0.16f, 0.18f, 1.0f }; // Charcoal fins
        GLfloat goldBandDiffuse[]= { 0.95f, 0.80f, 0.15f, 1.0f }; // Hazard warning gold ring
        GLfloat specReflect[]    = { 0.8f,  0.8f,  0.8f,  1.0f };

        glMaterialfv(GL_FRONT, GL_SPECULAR, specReflect);
        glMaterialf(GL_FRONT, GL_SHININESS, 64.0f);

        // 1. Central Cylindrical Rocket Fuselage
        glMaterialfv(GL_FRONT, GL_DIFFUSE, bodyDiffuse);
        glPushMatrix();
        glScalef(0.32f, 0.32f, 1.6f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Hazard warning band near warhead
        glMaterialfv(GL_FRONT, GL_DIFFUSE, goldBandDiffuse);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.75f);
        glScalef(0.34f, 0.34f, 0.15f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // 2. Warhead Nosecone (Pointed front, faces -Z)
        glMaterialfv(GL_FRONT, GL_DIFFUSE, warheadDiffuse);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.85f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        glutSolidCone(0.33f, 0.95f, 16, 12);
        glPopMatrix();

        // 3. Four Stabilizer Delta Fins (Cross / Cruciform shape at rear)
        glMaterialfv(GL_FRONT, GL_DIFFUSE, finDiffuse);

        // Horizontal Fins (Left & Right)
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.55f);
        glScalef(1.15f, 0.06f, 0.55f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Vertical Fins (Top & Bottom)
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.55f);
        glScalef(0.06f, 1.15f, 0.55f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // 4. Glowing Rocket Thruster Exhaust Nozzle
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        float pulse = 0.85f + 0.35f * sinf(enginePulse);

        // Outer Orange-Red Fire Cone
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.8f);
        glColor4f(1.0f, 0.45f, 0.05f, 0.95f);
        glutSolidCone(0.26f, 0.85f * pulse, 12, 12);

        // Inner White-Hot Core
        glColor4f(1.0f, 0.95f, 0.7f, 1.0f);
        glutSolidCone(0.14f, 0.45f * pulse, 10, 10);
        glPopMatrix();

        // Glowing Core Halo Sphere at nozzle
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.82f);
        glColor4f(1.0f, 0.6f, 0.1f, 0.65f);
        glutSolidSphere(0.30f, 8, 8);
        glPopMatrix();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);

        glPopMatrix();
    }
};

#endif // MISSILE_H
