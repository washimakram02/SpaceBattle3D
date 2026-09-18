#ifndef PLAYER_H
#define PLAYER_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/glut.h>
#include <vector>
#include <cmath>
#include "Bullet.h"
#include "Particle.h"

class Player {
public:
    float x, y, z;
    float vx, vy, vz;
    float roll;
    float pitch;
    float yaw;

    int health;
    int maxHealth;
    float shield;
    float maxShield;
    float shieldRegenTimer;
    float hitFlashTimer;

    int weaponType; // 0 = Standard Twin Blaster, 1 = Triple Shot
    float tripleShotTimer;
    float fireCooldown;
    float fireRate;

    float radius;
    float enginePulse;

    Player() {
        reset();
    }

    void reset() {
        x = 0.0f;
        y = 0.0f;
        z = 30.0f;
        vx = vy = vz = 0.0f;
        roll = pitch = yaw = 0.0f;

        maxHealth = 100;
        health = maxHealth;
        maxShield = 100.0f;
        shield = maxShield;
        shieldRegenTimer = 0.0f;
        hitFlashTimer = 0.0f;

        weaponType = 0;
        tripleShotTimer = 0.0f;
        fireCooldown = 0.0f;
        fireRate = 0.16f;

        radius = 2.2f;
        enginePulse = 0.0f;
    }

    void handleMovement(bool moveLeft, bool moveRight,
                        bool moveUp, bool moveDown,
                        bool moveForward, bool moveBack,
                        float dt) {
        float speed = 40.0f;
        float targetVx = 0.0f;
        float targetVy = 0.0f;
        float targetVz = 0.0f;

        if (moveLeft)    targetVx -= speed;
        if (moveRight)   targetVx += speed;
        if (moveUp)      targetVy += speed * 0.85f;
        if (moveDown)    targetVy -= speed * 0.85f;
        if (moveForward) targetVz -= speed * 0.9f;
        if (moveBack)    targetVz += speed * 0.7f;

        // Smooth velocity interpolation
        vx += (targetVx - vx) * 12.0f * dt;
        vy += (targetVy - vy) * 12.0f * dt;
        vz += (targetVz - vz) * 12.0f * dt;

        x += vx * dt;
        y += vy * dt;
        z += vz * dt;

        // Boundaries
        const float minX = -42.0f, maxX = 42.0f;
        const float minY = -24.0f, maxY = 24.0f;
        const float minZ = -5.0f,  maxZ = 45.0f;

        if (x < minX) { x = minX; vx = 0; }
        if (x > maxX) { x = maxX; vx = 0; }
        if (y < minY) { y = minY; vy = 0; }
        if (y > maxY) { y = maxY; vy = 0; }
        if (z < minZ) { z = minZ; vz = 0; }
        if (z > maxZ) { z = maxZ; vz = 0; }

        // Dynamic roll banking when strafing
        float targetRoll = -(vx / speed) * 35.0f;
        roll += (targetRoll - roll) * 10.0f * dt;

        // Dynamic pitch when climbing/diving (W = Up, S = Down)
        float targetPitch = (vy / speed) * 22.0f;
        pitch += (targetPitch - pitch) * 10.0f * dt;
    }

    void update(float dt, ParticleSystem& particles) {
        enginePulse += dt * 18.0f;

        // Fire cooldown
        if (fireCooldown > 0.0f) {
            fireCooldown -= dt;
        }

        // Weapon timer
        if (tripleShotTimer > 0.0f) {
            tripleShotTimer -= dt;
            if (tripleShotTimer <= 0.0f) {
                weaponType = 0;
            }
        }

        // Shield regeneration (after 3.5 seconds of not taking damage)
        if (shieldRegenTimer > 0.0f) {
            shieldRegenTimer -= dt;
        } else if (shield < maxShield) {
            shield += 12.0f * dt;
            if (shield > maxShield) shield = maxShield;
        }

        if (hitFlashTimer > 0.0f) {
            hitFlashTimer -= dt;
        }

        // Engine trail particles
        if (rand() % 2 == 0) {
            particles.addTrail(x - 0.9f, y - 0.15f, z + 2.2f, 0.1f, 0.7f, 1.0f, 0.35f);
            particles.addTrail(x + 0.9f, y - 0.15f, z + 2.2f, 0.1f, 0.7f, 1.0f, 0.35f);
        }
    }

    bool canFire() const {
        return fireCooldown <= 0.0f;
    }

    void updateAimYaw(float targetX, float dt) {
        float diffX = targetX - x;
        float targetYaw = diffX * 0.35f;
        if (targetYaw > 22.0f) targetYaw = 22.0f;
        if (targetYaw < -22.0f) targetYaw = -22.0f;
        yaw += (targetYaw - yaw) * 8.0f * dt;
    }

    bool fire(std::vector<Bullet>& bullets, float targetX = 0.0f, float targetY = 0.0f, float targetZ = -100.0f) {
        if (!canFire()) return false;
        fireCooldown = fireRate;

        // Left cannon pos
        float leftX = x - 1.5f;
        float leftY = y - 0.1f;
        float leftZ = z - 1.2f;

        // Right cannon pos
        float rightX = x + 1.5f;
        float rightY = y - 0.1f;
        float rightZ = z - 1.2f;

        // Vector towards target
        float ldx = targetX - leftX;
        float ldy = targetY - leftY;
        float ldz = targetZ - leftZ;

        float rdx = targetX - rightX;
        float rdy = targetY - rightY;
        float rdz = targetZ - rightZ;

        if (weaponType == 0) {
            // Twin blasters converging towards mouse aim target
            bullets.push_back(Bullet(leftX, leftY, leftZ, ldx, ldy, ldz, false, 25, 0.2f, 0.85f, 1.0f));
            bullets.push_back(Bullet(rightX, rightY, rightZ, rdx, rdy, rdz, false, 25, 0.2f, 0.85f, 1.0f));
        } else {
            // Triple Shot: Center cannon directly at target + side cannons with subtle flanking angle
            float midX = x;
            float midY = y + 0.2f;
            float midZ = z - 1.5f;
            bullets.push_back(Bullet(midX, midY, midZ, targetX - midX, targetY - midY, targetZ - midZ, false, 32, 1.0f, 0.85f, 0.1f));
            bullets.push_back(Bullet(leftX - 0.3f, leftY, leftZ, ldx - 2.0f, ldy, ldz, false, 25, 1.0f, 0.85f, 0.1f));
            bullets.push_back(Bullet(rightX + 0.3f, rightY, rightZ, rdx + 2.0f, rdy, rdz, false, 25, 1.0f, 0.85f, 0.1f));
        }
        return true;
    }

    void takeDamage(int dmg, ParticleSystem& particles) {
        hitFlashTimer = 0.35f;
        shieldRegenTimer = 3.5f;

        if (shield > 0.0f) {
            shield -= (float)dmg;
            if (shield < 0.0f) {
                health += (int)shield; // Overflow to health
                shield = 0.0f;
            }
            particles.addShockwave(x, y, z, 3.5f, 0.2f, 0.8f, 1.0f);
        } else {
            health -= dmg;
            particles.addExplosion(x, y, z, 12, 1.0f, 0.3f, 0.1f);
        }

        if (health < 0) health = 0;
    }

    void heal(int amt) {
        health += amt;
        if (health > maxHealth) health = maxHealth;
    }

    void addShield(int amt) {
        shield += amt;
        if (shield > maxShield) shield = maxShield;
    }

    void activateTripleShot(float duration = 12.0f) {
        weaponType = 1;
        tripleShotTimer = duration;
    }

    void draw() const {
        glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(roll, 0.0f, 0.0f, 1.0f);

        // --- Materials for Spaceship Hull ---
        GLfloat hullDiffuse[] = { 0.22f, 0.35f, 0.65f, 1.0f }; // High-tech navy/cobalt blue
        GLfloat hullSpec[] = { 0.7f, 0.8f, 1.0f, 1.0f };
        GLfloat cockpitDiffuse[] = { 0.1f, 0.9f, 0.95f, 0.85f }; // Cyan glass
        GLfloat trimDiffuse[] = { 0.85f, 0.85f, 0.9f, 1.0f }; // Platinum white trim
        GLfloat gunDiffuse[] = { 0.25f, 0.25f, 0.28f, 1.0f }; // Dark titanium

        glMaterialfv(GL_FRONT, GL_SPECULAR, hullSpec);
        glMaterialf(GL_FRONT, GL_SHININESS, 64.0f);

        // 1. Central Fuselage Body
        glMaterialfv(GL_FRONT, GL_DIFFUSE, hullDiffuse);
        glPushMatrix();
        glScalef(0.75f, 0.45f, 3.2f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Nose Cone (Pointed Front)
        glPushMatrix();
        glTranslatef(0.0f, -0.05f, -1.6f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        glutSolidCone(0.55f, 1.8f, 16, 16);
        glPopMatrix();

        // 2. Cockpit Canopy (Glass)
        glMaterialfv(GL_FRONT, GL_DIFFUSE, cockpitDiffuse);
        glPushMatrix();
        glTranslatef(0.0f, 0.28f, -0.3f);
        glScalef(0.42f, 0.32f, 1.1f);
        glutSolidSphere(1.0f, 12, 12);
        glPopMatrix();

        // 3. Delta Wings
        glMaterialfv(GL_FRONT, GL_DIFFUSE, hullDiffuse);

        // Left Wing
        glPushMatrix();
        glTranslatef(-1.4f, -0.05f, 0.2f);
        glRotatef(-8.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(12.0f, 0.0f, 0.0f, 1.0f);
        glScalef(2.2f, 0.08f, 1.6f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Right Wing
        glPushMatrix();
        glTranslatef(1.4f, -0.05f, 0.2f);
        glRotatef(8.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-12.0f, 0.0f, 0.0f, 1.0f);
        glScalef(2.2f, 0.08f, 1.6f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // 4. Wing Accents & Wingtip Blasters
        glMaterialfv(GL_FRONT, GL_DIFFUSE, gunDiffuse);

        // Left Cannon
        glPushMatrix();
        glTranslatef(-2.4f, -0.08f, -0.3f);
        glScalef(0.12f, 0.12f, 1.6f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Right Cannon
        glPushMatrix();
        glTranslatef(2.4f, -0.08f, -0.3f);
        glScalef(0.12f, 0.12f, 1.6f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // 5. Vertical Stabilizer Fins
        glMaterialfv(GL_FRONT, GL_DIFFUSE, trimDiffuse);
        glPushMatrix();
        glTranslatef(-0.6f, 0.55f, 1.0f);
        glRotatef(15.0f, 0.0f, 0.0f, 1.0f);
        glScalef(0.08f, 0.8f, 0.9f);
        glutSolidCube(1.0f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.6f, 0.55f, 1.0f);
        glRotatef(-15.0f, 0.0f, 0.0f, 1.0f);
        glScalef(0.08f, 0.8f, 0.9f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // 6. Dual Engine Thruster Exhausts
        glDisable(GL_LIGHTING);
        float pulse = 0.85f + 0.2f * sinf(enginePulse);

        // Left Thruster Flame
        glPushMatrix();
        glTranslatef(-0.45f, -0.05f, 1.6f);
        glColor4f(0.1f, 0.7f, 1.0f, 0.95f);
        glutSolidCone(0.28f, 0.9f * pulse, 12, 12);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glutSolidCone(0.14f, 0.5f * pulse, 10, 10);
        glPopMatrix();

        // Right Thruster Flame
        glPushMatrix();
        glTranslatef(0.45f, -0.05f, 1.6f);
        glColor4f(0.1f, 0.7f, 1.0f, 0.95f);
        glutSolidCone(0.28f, 0.9f * pulse, 12, 12);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glutSolidCone(0.14f, 0.5f * pulse, 10, 10);
        glPopMatrix();

        // 7. Shield Bubble Effect when hit or shielded
        if (hitFlashTimer > 0.0f && shield > 0.0f) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glColor4f(0.2f, 0.8f, 1.0f, 0.45f * (hitFlashTimer / 0.35f));
            glutSolidSphere(radius * 1.35f, 16, 16);
            glColor4f(0.5f, 0.9f, 1.0f, 0.75f * (hitFlashTimer / 0.35f));
            glutWireSphere(radius * 1.38f, 12, 12);
            glDisable(GL_BLEND);
        }

        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
};

#endif // PLAYER_H
