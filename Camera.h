#ifndef CAMERA_H
#define CAMERA_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdlib>

enum CameraMode {
    CAM_THIRD_PERSON = 0,
    CAM_FIRST_PERSON = 1,
    CAM_FREE = 2
};

class Camera {
public:
    float eyeX, eyeY, eyeZ;
    float lookX, lookY, lookZ;
    float upX, upY, upZ;
    int mode;

    float shakeTimer;
    float shakeIntensity;
    float freeAngle;

    Camera()
        : eyeX(0), eyeY(4.5f), eyeZ(42.0f),
          lookX(0), lookY(0), lookZ(-50.0f),
          upX(0), upY(1.0f), upZ(0),
          mode(CAM_THIRD_PERSON),
          shakeTimer(0.0f), shakeIntensity(0.0f),
          freeAngle(0.0f) {}

    void toggleMode() {
        mode = (mode + 1) % 3;
    }

    void addShake(float duration = 0.25f, float intensity = 0.6f) {
        shakeTimer = duration;
        shakeIntensity = intensity;
    }

    void update(float dt, float playerX, float playerY, float playerZ, float roll, float pitch) {
        float targetEyeX = 0.0f;
        float targetEyeY = 0.0f;
        float targetEyeZ = 0.0f;
        float targetLookX = 0.0f;
        float targetLookY = 0.0f;
        float targetLookZ = 0.0f;

        if (mode == CAM_THIRD_PERSON) {
            // Chase camera trailing behind and slightly above the ship
            targetEyeX = playerX * 0.75f;
            targetEyeY = playerY + 3.8f;
            targetEyeZ = playerZ + 11.0f;

            targetLookX = playerX;
            targetLookY = playerY + 0.5f;
            targetLookZ = playerZ - 60.0f;

            upX = -sinf(roll * 0.0174533f) * 0.35f;
            upY = 1.0f;
            upZ = 0.0f;

            // Smooth interpolation
            eyeX += (targetEyeX - eyeX) * 9.0f * dt;
            eyeY += (targetEyeY - eyeY) * 9.0f * dt;
            eyeZ += (targetEyeZ - eyeZ) * 9.0f * dt;

            lookX += (targetLookX - lookX) * 12.0f * dt;
            lookY += (targetLookY - lookY) * 12.0f * dt;
            lookZ += (targetLookZ - lookZ) * 12.0f * dt;

        } else if (mode == CAM_FIRST_PERSON) {
            // Inside / at the nose of cockpit
            eyeX = playerX;
            eyeY = playerY + 0.4f;
            eyeZ = playerZ - 1.2f;

            lookX = playerX;
            lookY = playerY;
            lookZ = playerZ - 80.0f;

            upX = 0.0f;
            upY = 1.0f;
            upZ = 0.0f;

        } else {
            // Free / Cinematic orbital view around the action
            freeAngle += dt * 0.5f;
            eyeX = playerX + cosf(freeAngle) * 22.0f;
            eyeY = playerY + 8.0f;
            eyeZ = playerZ + sinf(freeAngle) * 22.0f;

            lookX = playerX;
            lookY = playerY;
            lookZ = playerZ - 15.0f;

            upX = 0.0f;
            upY = 1.0f;
            upZ = 0.0f;
        }

        // Camera Shake effect
        if (shakeTimer > 0.0f) {
            shakeTimer -= dt;
            float offX = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * shakeIntensity;
            float offY = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * shakeIntensity;
            eyeX += offX;
            eyeY += offY;
            lookX += offX * 0.5f;
            lookY += offY * 0.5f;
        }
    }

    void apply() const {
        gluLookAt(eyeX, eyeY, eyeZ,
                  lookX, lookY, lookZ,
                  upX, upY, upZ);
    }
};

#endif // CAMERA_H
