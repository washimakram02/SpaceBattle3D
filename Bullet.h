#ifndef BULLET_H
#define BULLET_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/glut.h>
#include <cmath>

class Bullet {
public:
    float x, y, z;
    float vx, vy, vz;
    float speed;
    bool active;
    bool isEnemy;
    int damage;
    float lifetime;
    float maxLifetime;
    float r, g, b;
    float radius;

    Bullet()
        : x(0), y(0), z(0),
          vx(0), vy(0), vz(-1),
          speed(160.0f),
          active(false),
          isEnemy(false),
          damage(25),
          lifetime(0.0f),
          maxLifetime(3.5f),
          r(0.2f), g(0.9f), b(1.0f),
          radius(0.6f) {}

    Bullet(float startX, float startY, float startZ,
           float dirX, float dirY, float dirZ,
           bool enemy = false, int dmg = 25,
           float red = 0.2f, float green = 0.9f, float blue = 1.0f)
        : x(startX), y(startY), z(startZ),
          active(true),
          isEnemy(enemy),
          damage(dmg),
          lifetime(0.0f),
          maxLifetime(3.5f),
          r(red), g(green), b(blue),
          radius(enemy ? 0.7f : 0.5f) {
        
        speed = enemy ? 65.0f : 175.0f;
        float len = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
        if (len > 0.0001f) {
            vx = (dirX / len) * speed;
            vy = (dirY / len) * speed;
            vz = (dirZ / len) * speed;
        } else {
            vx = 0.0f;
            vy = 0.0f;
            vz = enemy ? speed : -speed;
        }
    }

    void update(float dt) {
        if (!active) return;
        x += vx * dt;
        y += vy * dt;
        z += vz * dt;
        lifetime += dt;
        if (lifetime >= maxLifetime) {
            active = false;
        }
    }

    void draw() const {
        if (!active) return;

        glPushMatrix();
        glTranslatef(x, y, z);

        // Emissive energy glow
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Outer glow
        glColor4f(r, g, b, 0.45f);
        glutSolidSphere(radius * 1.6f, 10, 10);

        // Bright core
        glColor4f(1.0f, 1.0f, 1.0f, 0.95f);
        glutSolidSphere(radius * 0.7f, 8, 8);

        // Directional laser streak
        glColor4f(r, g, b, 0.85f);
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(-vx * 0.035f, -vy * 0.035f, -vz * 0.035f);
        glEnd();
        glLineWidth(1.0f);

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
};

#endif // BULLET_H
