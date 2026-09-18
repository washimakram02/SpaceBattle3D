#ifndef ENEMY_H
#define ENEMY_H

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
#include "Bullet.h"
#include "Particle.h"

enum EnemyType {
    ENEMY_SCOUT = 0,
    ENEMY_FIGHTER = 1,
    ENEMY_BOSS = 2
};

class Enemy {
public:
    float x, y, z;
    float vx, vy, vz;
    int type;
    int health;
    int maxHealth;
    bool alive;
    float fireTimer;
    float fireInterval;
    float aiTimer;
    float radius;
    float roll, pitch, yaw;
    float flashTimer;
    float turretAngle;

    Enemy()
        : x(0), y(0), z(-150.0f),
          vx(0), vy(0), vz(18.0f),
          type(ENEMY_SCOUT),
          health(25), maxHealth(25),
          alive(true),
          fireTimer(1.0f), fireInterval(1.8f),
          aiTimer(0.0f),
          radius(2.0f),
          roll(0), pitch(0), yaw(0),
          flashTimer(0), turretAngle(0) {}

    Enemy(float startX, float startY, float startZ, int enemyType)
        : x(startX), y(startY), z(startZ),
          type(enemyType),
          alive(true),
          aiTimer((float)(rand() % 100) / 10.0f),
          roll(0), pitch(0), yaw(0),
          flashTimer(0), turretAngle(0) {
        
        if (type == ENEMY_SCOUT) {
            maxHealth = 25;
            health = maxHealth;
            radius = 1.8f;
            vz = 22.0f + (float)rand() / RAND_MAX * 8.0f;
            vx = ((float)rand() / RAND_MAX * 10.0f - 5.0f);
            vy = ((float)rand() / RAND_MAX * 6.0f - 3.0f);
            fireInterval = 1.4f + (float)rand() / RAND_MAX * 1.0f;
            fireTimer = 0.5f + (float)rand() / RAND_MAX * 1.0f;
        } else if (type == ENEMY_FIGHTER) {
            maxHealth = 65;
            health = maxHealth;
            radius = 2.8f;
            vz = 16.0f + (float)rand() / RAND_MAX * 6.0f;
            vx = ((float)rand() / RAND_MAX * 8.0f - 4.0f);
            vy = 0.0f;
            fireInterval = 1.6f + (float)rand() / RAND_MAX * 0.8f;
            fireTimer = 0.8f + (float)rand() / RAND_MAX * 1.0f;
        } else {
            // Boss
            maxHealth = 950;
            health = maxHealth;
            radius = 8.5f;
            vz = 10.0f; // Approaches to combat position
            vx = 0.0f;
            vy = 0.0f;
            fireInterval = 0.75f;
            fireTimer = 1.5f;
        }
    }

    void takeDamage(int dmg) {
        health -= dmg;
        flashTimer = 0.15f;
        if (health <= 0) {
            health = 0;
            alive = false;
        }
    }

    void update(float dt, float playerX, float playerY, float playerZ,
                std::vector<Bullet>& enemyBullets, ParticleSystem& particles) {
        if (!alive) return;

        aiTimer += dt;
        turretAngle += 45.0f * dt;
        if (flashTimer > 0.0f) flashTimer -= dt;

        if (type == ENEMY_SCOUT) {
            // Agile weave pattern
            vx = sinf(aiTimer * 3.0f) * 16.0f;
            vy = cosf(aiTimer * 2.0f) * 8.0f;

            x += vx * dt;
            y += vy * dt;
            z += vz * dt;

            roll = -vx * 1.8f;

            // Fire forward towards player
            fireTimer -= dt;
            if (fireTimer <= 0.0f && z < playerZ - 5.0f && z > -90.0f) {
                fireTimer = fireInterval;
                float dx = playerX - x;
                float dy = playerY - y;
                float dz = playerZ - z;
                enemyBullets.push_back(Bullet(x, y, z + 1.2f, dx, dy, dz, true, 15, 1.0f, 0.25f, 0.1f));
            }

            // Despawn if passed behind camera
            if (z > 70.0f) alive = false;

        } else if (type == ENEMY_FIGHTER) {
            // Sweeping strafing motion
            vx = sinf(aiTimer * 1.5f) * 18.0f;
            vy = sinf(aiTimer * 0.8f) * 6.0f;

            x += vx * dt;
            y += vy * dt;
            z += vz * dt;

            roll = -vx * 1.2f;

            // Twin plasma blasters
            fireTimer -= dt;
            if (fireTimer <= 0.0f && z < playerZ - 8.0f && z > -110.0f) {
                fireTimer = fireInterval;
                enemyBullets.push_back(Bullet(x - 1.6f, y, z + 1.5f, 0.0f, 0.0f, 1.0f, true, 18, 1.0f, 0.4f, 0.05f));
                enemyBullets.push_back(Bullet(x + 1.6f, y, z + 1.5f, 0.0f, 0.0f, 1.0f, true, 18, 1.0f, 0.4f, 0.05f));
            }

            if (z > 70.0f) alive = false;

        } else if (type == ENEMY_BOSS) {
            // Boss AI: Move to z = -35.0f then hover, track player, and unleash multi-barrage attacks
            if (z < -35.0f) {
                z += vz * dt;
            } else {
                // Hover and strafe
                float targetX = playerX * 0.65f + sinf(aiTimer * 0.9f) * 18.0f;
                float targetY = sinf(aiTimer * 1.4f) * 8.0f;
                x += (targetX - x) * 2.0f * dt;
                y += (targetY - y) * 2.0f * dt;
                roll = -(targetX - x) * 0.8f;
            }

            // Boss Multi-Phase Attacks
            fireTimer -= dt;
            if (fireTimer <= 0.0f && z >= -45.0f) {
                bool rage = (health < maxHealth * 0.45f);
                fireTimer = rage ? 0.45f : 0.75f;

                int attackPattern = rand() % 3;
                if (attackPattern == 0) {
                    // Spread shot: 5 fiery plasma bolts
                    for (int angle = -2; angle <= 2; ++angle) {
                        float spreadX = angle * 0.18f;
                        float dx = (playerX - x) * 0.02f + spreadX;
                        enemyBullets.push_back(Bullet(x + angle * 2.5f, y - 0.5f, z + 3.0f,
                                                      dx, (playerY - y) * 0.02f, 1.0f,
                                                      true, 22, 1.0f, 0.1f, 0.2f));
                    }
                } else if (attackPattern == 1) {
                    // Twin heavy cannon blast
                    enemyBullets.push_back(Bullet(x - 5.5f, y - 0.8f, z + 2.5f, 0.0f, 0.0f, 1.0f, true, 26, 1.0f, 0.5f, 0.0f));
                    enemyBullets.push_back(Bullet(x + 5.5f, y - 0.8f, z + 2.5f, 0.0f, 0.0f, 1.0f, true, 26, 1.0f, 0.5f, 0.0f));
                } else {
                    // Targeted salvo straight at player
                    float dx = playerX - x;
                    float dy = playerY - y;
                    float dz = playerZ - z;
                    enemyBullets.push_back(Bullet(x, y - 1.2f, z + 4.0f, dx, dy, dz, true, 24, 0.9f, 0.1f, 0.9f));
                }

                // Boss thruster particles
                particles.addTrail(x - 4.0f, y, z - 7.0f, 1.0f, 0.3f, 0.1f, 0.7f);
                particles.addTrail(x + 4.0f, y, z - 7.0f, 1.0f, 0.3f, 0.1f, 0.7f);
            }
        }
    }

    void draw() const {
        if (!alive) return;

        glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(roll, 0.0f, 0.0f, 1.0f);

        // Flash white when hit
        bool flashing = (flashTimer > 0.0f);

        if (type == ENEMY_SCOUT) {
            // Sleek Aggressive Crimson Scout Drone
            GLfloat hullColor[] = { flashing ? 1.0f : 0.85f, flashing ? 1.0f : 0.12f, flashing ? 1.0f : 0.15f, 1.0f };
            GLfloat eyeGlow[] = { 1.0f, 0.7f, 0.0f, 1.0f };

            glMaterialfv(GL_FRONT, GL_DIFFUSE, hullColor);

            // Forward-pointing needle nose / cockpit
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 1.2f);
            glutSolidCone(0.55f, 1.6f, 10, 10);
            glPopMatrix();

            // Main Scout Body
            glPushMatrix();
            glScalef(0.7f, 0.35f, 2.0f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // Swept-Forward Angular Wings
            glPushMatrix();
            glTranslatef(-1.2f, 0.0f, 0.2f);
            glRotatef(25.0f, 0.0f, 1.0f, 0.0f);
            glScalef(1.6f, 0.08f, 0.9f);
            glutSolidCube(1.0f);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(1.2f, 0.0f, 0.2f);
            glRotatef(-25.0f, 0.0f, 1.0f, 0.0f);
            glScalef(1.6f, 0.08f, 0.9f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // Center Sensor Eye
            glMaterialfv(GL_FRONT, GL_DIFFUSE, eyeGlow);
            glPushMatrix();
            glTranslatef(0.0f, 0.15f, 0.4f);
            glutSolidSphere(0.22f, 8, 8);
            glPopMatrix();

            // Engine Flame
            glDisable(GL_LIGHTING);
            glColor4f(1.0f, 0.3f, 0.0f, 0.9f);
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, -1.0f);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            glutSolidCone(0.25f, 0.7f, 8, 8);
            glPopMatrix();
            glEnable(GL_LIGHTING);

        } else if (type == ENEMY_FIGHTER) {
            // Dual-Hull Heavy Strike Fighter
            GLfloat hullColor[] = { flashing ? 1.0f : 0.75f, flashing ? 1.0f : 0.35f, flashing ? 1.0f : 0.05f, 1.0f }; // Burnished bronze/orange
            GLfloat darkColor[] = { 0.25f, 0.25f, 0.3f, 1.0f };
            GLfloat cockpitColor[] = { 0.9f, 0.1f, 0.1f, 1.0f };

            // Left Pod
            glMaterialfv(GL_FRONT, GL_DIFFUSE, hullColor);
            glPushMatrix();
            glTranslatef(-1.5f, 0.0f, 0.0f);
            glScalef(0.65f, 0.5f, 3.0f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // Right Pod
            glPushMatrix();
            glTranslatef(1.5f, 0.0f, 0.0f);
            glScalef(0.65f, 0.5f, 3.0f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // Cross Bridge / Cockpit
            glMaterialfv(GL_FRONT, GL_DIFFUSE, darkColor);
            glPushMatrix();
            glScalef(2.6f, 0.35f, 1.2f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // Cockpit Dome
            glMaterialfv(GL_FRONT, GL_DIFFUSE, cockpitColor);
            glPushMatrix();
            glTranslatef(0.0f, 0.25f, 0.0f);
            glutSolidSphere(0.45f, 10, 10);
            glPopMatrix();

            // Twin Forward Heavy Blasters
            glMaterialfv(GL_FRONT, GL_DIFFUSE, darkColor);
            glPushMatrix();
            glTranslatef(-1.5f, -0.1f, 1.8f);
            glScalef(0.18f, 0.18f, 1.0f);
            glutSolidCube(1.0f);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(1.5f, -0.1f, 1.8f);
            glScalef(0.18f, 0.18f, 1.0f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // Twin Exhaust Flames
            glDisable(GL_LIGHTING);
            glColor4f(1.0f, 0.45f, 0.0f, 0.95f);
            glPushMatrix();
            glTranslatef(-1.5f, 0.0f, -1.5f);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            glutSolidCone(0.35f, 0.9f, 8, 8);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(1.5f, 0.0f, -1.5f);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            glutSolidCone(0.35f, 0.9f, 8, 8);
            glPopMatrix();
            glEnable(GL_LIGHTING);

        } else {
            // --- ENEMY BOSS: GRAND DREADNOUGHT WARSHIP ---
            GLfloat bossHull[] = { flashing ? 1.0f : 0.38f, flashing ? 1.0f : 0.08f, flashing ? 1.0f : 0.14f, 1.0f }; // Dark Crimson Armor
            GLfloat armorPlates[] = { 0.22f, 0.22f, 0.26f, 1.0f }; // Titanium grey
            GLfloat coreGlow[] = { 1.0f, 0.2f, 0.0f, 1.0f }; // Molten Reactor Core
            GLfloat bridgeGlass[] = { 0.1f, 0.9f, 0.3f, 1.0f }; // Toxic green command deck

            // 1. Massive Main Hull
            glMaterialfv(GL_FRONT, GL_DIFFUSE, bossHull);
            glPushMatrix();
            glScalef(4.5f, 2.2f, 12.0f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // 2. Heavy Outrigger Armor Wings
            glMaterialfv(GL_FRONT, GL_DIFFUSE, armorPlates);
            glPushMatrix();
            glTranslatef(-5.2f, -0.3f, 0.0f);
            glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);
            glScalef(6.0f, 1.0f, 9.0f);
            glutSolidCube(1.0f);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(5.2f, -0.3f, 0.0f);
            glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
            glScalef(6.0f, 1.0f, 9.0f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // 3. Command Bridge Tower
            glMaterialfv(GL_FRONT, GL_DIFFUSE, bossHull);
            glPushMatrix();
            glTranslatef(0.0f, 1.8f, -2.5f);
            glScalef(2.2f, 1.4f, 4.0f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // Command Deck Visor
            glMaterialfv(GL_FRONT, GL_DIFFUSE, bridgeGlass);
            glPushMatrix();
            glTranslatef(0.0f, 2.1f, -0.4f);
            glScalef(1.8f, 0.4f, 0.6f);
            glutSolidCube(1.0f);
            glPopMatrix();

            // 4. Exposed Glowing Core on Top
            glMaterialfv(GL_FRONT, GL_DIFFUSE, coreGlow);
            glPushMatrix();
            glTranslatef(0.0f, 1.2f, 2.0f);
            glutSolidSphere(1.2f, 14, 14);
            glPopMatrix();

            // 5. Rotating Heavy Turrets on Wings
            glMaterialfv(GL_FRONT, GL_DIFFUSE, armorPlates);

            // Left Turret
            glPushMatrix();
            glTranslatef(-5.5f, 0.5f, 2.0f);
            glRotatef(turretAngle, 0.0f, 1.0f, 0.0f);
            glutSolidSphere(0.9f, 10, 10);
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.8f);
            glScalef(0.3f, 0.3f, 1.5f);
            glutSolidCube(1.0f);
            glPopMatrix();
            glPopMatrix();

            // Right Turret
            glPushMatrix();
            glTranslatef(5.5f, 0.5f, 2.0f);
            glRotatef(-turretAngle, 0.0f, 1.0f, 0.0f);
            glutSolidSphere(0.9f, 10, 10);
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.8f);
            glScalef(0.3f, 0.3f, 1.5f);
            glutSolidCube(1.0f);
            glPopMatrix();
            glPopMatrix();

            // 6. Forward Heavy Prow / Ram
            glMaterialfv(GL_FRONT, GL_DIFFUSE, bossHull);
            glPushMatrix();
            glTranslatef(0.0f, -0.2f, 6.5f);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            glutSolidCone(2.2f, 3.5f, 12, 12);
            glPopMatrix();

            // 7. Quad Heavy Thruster Plumes
            glDisable(GL_LIGHTING);
            glColor4f(1.0f, 0.2f, 0.0f, 0.95f);
            for (int e = -1; e <= 1; e += 2) {
                glPushMatrix();
                glTranslatef(e * 1.6f, -0.2f, -6.2f);
                glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
                glutSolidCone(0.85f, 2.5f, 12, 12);
                glPopMatrix();

                glPushMatrix();
                glTranslatef(e * 4.8f, -0.3f, -4.8f);
                glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
                glutSolidCone(0.7f, 2.0f, 10, 10);
                glPopMatrix();
            }
            glEnable(GL_LIGHTING);
        }

        glPopMatrix();
    }
};

#endif // ENEMY_H
