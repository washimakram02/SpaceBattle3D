#ifndef POWERUP_H
#define POWERUP_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/glut.h>
#include <cmath>

enum PowerUpType {
    POWERUP_SHIELD = 0,
    POWERUP_HEALTH = 1,
    POWERUP_TRIPLE_SHOT = 2
};

class PowerUp {
public:
    float x, y, z;
    float vz;
    int type;
    bool active;
    float rotation;
    float bobbing;
    float radius;
    float lifetime;

    PowerUp()
        : x(0), y(0), z(0),
          vz(15.0f),
          type(POWERUP_SHIELD),
          active(false),
          rotation(0.0f),
          bobbing(0.0f),
          radius(1.5f),
          lifetime(0.0f) {}

    PowerUp(float startX, float startY, float startZ, int pType)
        : x(startX), y(startY), z(startZ),
          vz(16.0f),
          type(pType),
          active(true),
          rotation(0.0f),
          bobbing(0.0f),
          radius(1.5f),
          lifetime(0.0f) {}

    void update(float dt) {
        if (!active) return;
        z += vz * dt;
        rotation += 90.0f * dt;
        if (rotation >= 360.0f) rotation -= 360.0f;
        bobbing += 3.0f * dt;

        lifetime += dt;
        if (z > 70.0f || lifetime > 18.0f) {
            active = false;
        }
    }

    void draw() const {
        if (!active) return;

        glPushMatrix();
        float currentY = y + sinf(bobbing) * 0.4f;
        glTranslatef(x, currentY, z);
        glRotatef(rotation, 0.4f, 1.0f, 0.2f);

        // Render based on power-up type
        if (type == POWERUP_SHIELD) {
            // Cyan Shield Orb
            glDisable(GL_LIGHTING);
            glColor4f(0.1f, 0.8f, 1.0f, 0.9f);
            glutWireSphere(radius * 1.1f, 10, 10);

            glEnable(GL_LIGHTING);
            GLfloat cyanMat[] = { 0.0f, 0.6f, 0.9f, 1.0f };
            GLfloat cyanSpec[] = { 0.8f, 1.0f, 1.0f, 1.0f };
            glMaterialfv(GL_FRONT, GL_DIFFUSE, cyanMat);
            glMaterialfv(GL_FRONT, GL_SPECULAR, cyanSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, 64.0f);
            glutSolidOctahedron();
        } else if (type == POWERUP_HEALTH) {
            // Green Health Cross / Cube
            glDisable(GL_LIGHTING);
            glColor4f(0.2f, 1.0f, 0.3f, 0.9f);
            glutWireCube(radius * 1.6f);

            glEnable(GL_LIGHTING);
            GLfloat greenMat[] = { 0.1f, 0.9f, 0.2f, 1.0f };
            GLfloat greenSpec[] = { 0.7f, 1.0f, 0.7f, 1.0f };
            glMaterialfv(GL_FRONT, GL_DIFFUSE, greenMat);
            glMaterialfv(GL_FRONT, GL_SPECULAR, greenSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, 48.0f);

            // 3D Cross geometry
            glPushMatrix();
            glScalef(0.4f, 1.3f, 0.4f);
            glutSolidCube(1.0f);
            glPopMatrix();

            glPushMatrix();
            glScalef(1.3f, 0.4f, 0.4f);
            glutSolidCube(1.0f);
            glPopMatrix();
        } else {
            // Golden Triple Shot Crystal
            glDisable(GL_LIGHTING);
            glColor4f(1.0f, 0.85f, 0.1f, 0.95f);
            glutWireTorus(0.25f, radius * 0.9f, 8, 16);

            glEnable(GL_LIGHTING);
            GLfloat goldMat[] = { 1.0f, 0.75f, 0.0f, 1.0f };
            GLfloat goldSpec[] = { 1.0f, 1.0f, 0.6f, 1.0f };
            glMaterialfv(GL_FRONT, GL_DIFFUSE, goldMat);
            glMaterialfv(GL_FRONT, GL_SPECULAR, goldSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, 80.0f);

            glPushMatrix();
            glScalef(0.7f, 1.4f, 0.7f);
            glutSolidOctahedron();
            glPopMatrix();
        }

        glPopMatrix();
    }
};

#endif // POWERUP_H
