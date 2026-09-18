#include "Game.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <algorithm>

Game::Game()
    : windowWidth(1280), windowHeight(720),
      state(STATE_MENU),
      score(0), highScore(0),
      level(1), kills(0),
      killsThisLevel(0), killsNeeded(12),
      enemySpawnTimer(0.0f), enemySpawnInterval(1.8f),
      asteroidSpawnTimer(0.0f), levelBannerTimer(0.0f),
      mouseX(640), mouseY(360), spacePressed(false),
      mouseLeftDown(false), mouseRightDown(false),
      aimTargetX(0.0f), aimTargetY(0.0f), aimTargetZ(-70.0f),
      targetLocked(false), lockedTargetName(""),
      lockedTargetHealth(0), lockedTargetMaxHealth(0),
      bossAlertPlayed(false), gameOverSoundPlayed(false),
      victorySoundPlayed(false), prevEnemyBulletCount(0) {
    
    for (int i = 0; i < 256; ++i) {
        keys[i] = false;
        specialKeys[i] = false;
    }
}

void Game::init(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    soundManager.init();
    initStars();
    reset();
}

void Game::reset() {
    player.reset();
    camera = Camera();
    particles.clear();
    enemies.clear();
    playerBullets.clear();
    enemyBullets.clear();
    powerups.clear();
    asteroids.clear();

    mouseLeftDown = false;
    mouseRightDown = false;
    targetLocked = false;
    aimTargetX = 0.0f;
    aimTargetY = 0.0f;
    aimTargetZ = -70.0f;

    score = 0;
    level = 1;
    kills = 0;
    killsThisLevel = 0;
    killsNeeded = 12;

    enemySpawnTimer = 0.5f;
    enemySpawnInterval = 1.8f;
    asteroidSpawnTimer = 0.8f;
    levelBannerTimer = 3.0f;

    bossAlertPlayed = false;
    gameOverSoundPlayed = false;
    victorySoundPlayed = false;
    prevEnemyBulletCount = 0;
}

void Game::startNextLevel() {
    level++;
    killsThisLevel = 0;
    levelBannerTimer = 3.5f;

    soundManager.play(SND_LEVEL_CLEAR, 0.95f);

    // Clear remaining minions
    enemies.clear();
    enemyBullets.clear();

    if (level == 2) {
        killsNeeded = 22;
        enemySpawnInterval = 1.4f;
    } else if (level == 3) {
        // Boss Level!
        killsNeeded = 1;
        enemySpawnInterval = 4.0f; // Occasional escort scouts
        // Spawn the Boss Dreadnought
        enemies.push_back(Enemy(0.0f, 2.0f, -140.0f, ENEMY_BOSS));
        soundManager.play(SND_BOSS_WARNING, 1.0f);
        bossAlertPlayed = true;
    }
}

void Game::initStars() {
    stars.clear();
    const int numStars = 650;
    for (int i = 0; i < numStars; ++i) {
        Star s;
        s.x = ((float)rand() / RAND_MAX * 260.0f - 130.0f);
        s.y = ((float)rand() / RAND_MAX * 180.0f - 90.0f);
        s.z = ((float)rand() / RAND_MAX * 420.0f - 320.0f);

        int colorType = rand() % 5;
        if (colorType == 0) {
            // Cyan star
            s.r = 0.4f; s.g = 0.85f; s.b = 1.0f;
        } else if (colorType == 1) {
            // Golden star
            s.r = 1.0f; s.g = 0.9f; s.b = 0.5f;
        } else if (colorType == 2) {
            // Violet star
            s.r = 0.85f; s.g = 0.5f; s.b = 1.0f;
        } else {
            // Pure white star
            s.r = 0.95f; s.g = 0.95f; s.b = 1.0f;
        }

        s.size = 1.0f + ((float)rand() / RAND_MAX * 2.2f);
        s.speed = 35.0f + ((float)rand() / RAND_MAX * 45.0f);
        stars.push_back(s);
    }
}

void Game::spawnAsteroid() {
    Asteroid a;
    a.x = ((float)rand() / RAND_MAX * 70.0f - 35.0f);
    a.y = ((float)rand() / RAND_MAX * 40.0f - 20.0f);
    a.z = -180.0f;

    a.vx = ((float)rand() / RAND_MAX * 6.0f - 3.0f);
    a.vy = ((float)rand() / RAND_MAX * 4.0f - 2.0f);
    a.vz = 14.0f + ((float)rand() / RAND_MAX * 10.0f);

    a.rotX = (float)(rand() % 360);
    a.rotY = (float)(rand() % 360);
    a.rotZ = (float)(rand() % 360);
    a.rotSpeedX = ((float)rand() / RAND_MAX * 45.0f - 22.5f);
    a.rotSpeedY = ((float)rand() / RAND_MAX * 45.0f - 22.5f);

    a.radius = 2.2f + ((float)rand() / RAND_MAX * 2.2f);
    a.maxHealth = (int)(a.radius * 18);
    a.health = a.maxHealth;
    a.active = true;

    asteroids.push_back(a);
}

void Game::spawnEnemies(float dt) {
    if (level < 3) {
        enemySpawnTimer -= dt;
        if (enemySpawnTimer <= 0.0f) {
            enemySpawnTimer = enemySpawnInterval;

            float startX = ((float)rand() / RAND_MAX * 60.0f - 30.0f);
            float startY = ((float)rand() / RAND_MAX * 30.0f - 15.0f);
            float startZ = -140.0f;

            int type = ENEMY_SCOUT;
            if (level == 2 && (rand() % 100 < 45)) {
                type = ENEMY_FIGHTER;
            }

            enemies.push_back(Enemy(startX, startY, startZ, type));
        }
    } else {
        // Level 3 Boss level: occasionally spawn escort scouts
        enemySpawnTimer -= dt;
        if (enemySpawnTimer <= 0.0f) {
            enemySpawnTimer = enemySpawnInterval;
            if (enemies.size() < 4) { // Don't crowd the boss fight
                float startX = ((float)rand() / RAND_MAX * 50.0f - 25.0f);
                float startY = ((float)rand() / RAND_MAX * 24.0f - 12.0f);
                enemies.push_back(Enemy(startX, startY, -130.0f, ENEMY_SCOUT));
            }
        }
    }

    // Asteroid spawning
    asteroidSpawnTimer -= dt;
    if (asteroidSpawnTimer <= 0.0f) {
        asteroidSpawnTimer = 2.4f + ((float)rand() / RAND_MAX * 1.6f);
        if (asteroids.size() < 8) {
            spawnAsteroid();
        }
    }
}

void Game::handleKeyDown(unsigned char key, int x, int y) {
    keys[key] = true;

    if (key == 27) { // ESC
        exit(0);
    }

    if (state == STATE_MENU) {
        if (key == 13 || key == 32) { // ENTER or SPACE
            soundManager.play(SND_UI_CLICK, 0.8f);
            state = STATE_PLAYING;
        }
        return;
    }

    if (state == STATE_GAMEOVER || state == STATE_VICTORY) {
        if (key == 'r' || key == 'R') {
            soundManager.play(SND_UI_CLICK, 0.8f);
            reset();
            state = STATE_PLAYING;
        }
        return;
    }

    if (key == 'p' || key == 'P') {
        soundManager.play(SND_UI_CLICK, 0.7f);
        if (state == STATE_PLAYING) state = STATE_PAUSED;
        else if (state == STATE_PAUSED) state = STATE_PLAYING;
    }

    if (key == 'm' || key == 'M') {
        soundManager.toggleMute();
    }

    if (key == '+' || key == '=') {
        soundManager.increaseVolume(0.1f);
    }

    if (key == '-' || key == '_') {
        soundManager.decreaseVolume(0.1f);
    }

    if (key == 'c' || key == 'C') {
        soundManager.play(SND_UI_CLICK, 0.5f);
        camera.toggleMode();
    }
}

void Game::handleKeyUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

void Game::handleSpecialKeyDown(int key, int x, int y) {
    specialKeys[key] = true;
}

void Game::handleSpecialKeyUp(int key, int x, int y) {
    specialKeys[key] = false;
}

void Game::handlePassiveMouse(int x, int y) {
    mouseX = x;
    mouseY = y;
}

void Game::handleMouseClick(int button, int btnState, int x, int y) {
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON) {
        mouseLeftDown = (btnState == GLUT_DOWN);

        if (btnState == GLUT_DOWN) {
            if (state == STATE_MENU) {
                soundManager.play(SND_UI_CLICK, 0.8f);
                state = STATE_PLAYING;
            } else if (state == STATE_GAMEOVER || state == STATE_VICTORY) {
                soundManager.play(SND_UI_CLICK, 0.8f);
                reset();
                state = STATE_PLAYING;
            }
        }
    } else if (button == GLUT_RIGHT_BUTTON) {
        mouseRightDown = (btnState == GLUT_DOWN);
        if (btnState == GLUT_DOWN && state == STATE_PLAYING) {
            soundManager.play(SND_UI_CLICK, 0.5f);
            camera.toggleMode();
        }
    }
}

void Game::update(float dt) {
    // Starfield warp movement even on menus for dynamic backdrop
    for (auto& s : stars) {
        s.z += s.speed * dt * (state == STATE_PLAYING ? 1.0f : 0.4f);
        if (s.z > 80.0f) {
            s.z = -320.0f;
            s.x = ((float)rand() / RAND_MAX * 260.0f - 130.0f);
            s.y = ((float)rand() / RAND_MAX * 180.0f - 90.0f);
        }
    }

    if (state != STATE_PLAYING) return;

    if (levelBannerTimer > 0.0f) {
        levelBannerTimer -= dt;
    }

    // Player controls (W = UP, S = DOWN, A = LEFT, D = RIGHT, E = FORWARD, Q = BACK)
    bool moveLeft    = keys['a'] || keys['A'] || specialKeys[GLUT_KEY_LEFT];
    bool moveRight   = keys['d'] || keys['D'] || specialKeys[GLUT_KEY_RIGHT];
    bool moveUp      = keys['w'] || keys['W'] || specialKeys[GLUT_KEY_UP];
    bool moveDown    = keys['s'] || keys['S'] || specialKeys[GLUT_KEY_DOWN];
    bool moveForward = keys['e'] || keys['E'];
    bool moveBack    = keys['q'] || keys['Q'];

    player.handleMovement(moveLeft, moveRight, moveUp, moveDown, moveForward, moveBack, dt);
    player.update(dt, particles);

    // 3D Mouse Aim Calculation
    float normX = ((float)mouseX / (float)windowWidth) * 2.0f - 1.0f;
    float normY = (1.0f - ((float)mouseY / (float)windowHeight)) * 2.0f - 1.0f;
    float aspect = (float)windowWidth / (float)windowHeight;
    float tanHalfFov = tanf(30.0f * 0.0174532925f);

    float combatDepth = 85.0f;
    float defaultTargetZ = camera.eyeZ - combatDepth;
    aimTargetX = camera.eyeX + normX * combatDepth * tanHalfFov * aspect;
    aimTargetY = camera.eyeY + normY * combatDepth * tanHalfFov;
    aimTargetZ = defaultTargetZ;
    targetLocked = false;

    // Check Enemy Lock-On
    float mouseOrthoY = (float)(windowHeight - mouseY);
    for (const auto& enemy : enemies) {
        if (!enemy.alive) continue;
        float relZ = camera.eyeZ - enemy.z;
        if (relZ > 3.0f) {
            float projX = windowWidth * 0.5f + ((enemy.x - camera.eyeX) / (relZ * tanHalfFov * aspect)) * (windowWidth * 0.5f);
            float projY = windowHeight * 0.5f + ((enemy.y - camera.eyeY) / (relZ * tanHalfFov)) * (windowHeight * 0.5f);
            float dist = sqrtf((projX - mouseX)*(projX - mouseX) + (projY - mouseOrthoY)*(projY - mouseOrthoY));
            float threshold = (enemy.type == ENEMY_BOSS) ? 120.0f : 55.0f;
            if (dist < threshold) {
                targetLocked = true;
                aimTargetX = enemy.x;
                aimTargetY = enemy.y;
                aimTargetZ = enemy.z;
                lockedTargetHealth = enemy.health;
                lockedTargetMaxHealth = enemy.maxHealth;
                lockedTargetName = (enemy.type == ENEMY_BOSS) ? "BOSS DREADNOUGHT" :
                                   (enemy.type == ENEMY_FIGHTER) ? "STRIKE FIGHTER" : "SCOUT DRONE";
                break;
            }
        }
    }

    // Subtly turn ship yaw towards mouse aim
    player.updateAimYaw(aimTargetX, dt);

    // Firing (Spacebar OR Left Mouse Click)
    if (keys[32] || mouseLeftDown) {
        if (player.fire(playerBullets, aimTargetX, aimTargetY, aimTargetZ)) {
            if (player.weaponType == 1) {
                soundManager.play(SND_LASER_TRIPLE, 0.9f);
            } else {
                soundManager.play(SND_LASER, 0.8f);
            }
        }
    }

    // Update Camera
    camera.update(dt, player.x, player.y, player.z, player.roll, player.pitch);

    // Update Player Bullets
    for (size_t i = 0; i < playerBullets.size(); ) {
        playerBullets[i].update(dt);
        if (!playerBullets[i].active) {
            playerBullets.erase(playerBullets.begin() + i);
        } else {
            ++i;
        }
    }

    // Update Enemy Bullets
    for (size_t i = 0; i < enemyBullets.size(); ) {
        enemyBullets[i].update(dt);
        if (!enemyBullets[i].active) {
            enemyBullets.erase(enemyBullets.begin() + i);
        } else {
            ++i;
        }
    }

    // Update Enemies
    spawnEnemies(dt);
    size_t prevBullets = enemyBullets.size();
    for (size_t i = 0; i < enemies.size(); ) {
        enemies[i].update(dt, player.x, player.y, player.z, enemyBullets, particles);
        if (!enemies[i].alive) {
            enemies.erase(enemies.begin() + i);
        } else {
            ++i;
        }
    }
    if (enemyBullets.size() > prevBullets) {
        bool bossFired = false;
        float fireX = 0.0f, fireZ = 0.0f;
        for (size_t b = prevBullets; b < enemyBullets.size(); ++b) {
            if (enemyBullets[b].damage >= 22) {
                bossFired = true;
                fireX = enemyBullets[b].x;
                fireZ = enemyBullets[b].z;
                break;
            }
        }
        if (bossFired) {
            soundManager.play3D(SND_BOSS_LASER, fireX, fireZ, player.x, player.z, 0.95f);
        } else {
            soundManager.play3D(SND_ENEMY_LASER, enemyBullets[prevBullets].x, enemyBullets[prevBullets].z, player.x, player.z, 0.75f);
        }
    }

    // Update Asteroids
    for (size_t i = 0; i < asteroids.size(); ) {
        Asteroid& a = asteroids[i];
        a.x += a.vx * dt;
        a.y += a.vy * dt;
        a.z += a.vz * dt;
        a.rotX += a.rotSpeedX * dt;
        a.rotY += a.rotSpeedY * dt;

        if (a.z > 70.0f || !a.active) {
            asteroids.erase(asteroids.begin() + i);
        } else {
            ++i;
        }
    }

    // Update PowerUps
    for (size_t i = 0; i < powerups.size(); ) {
        powerups[i].update(dt);
        if (!powerups[i].active) {
            powerups.erase(powerups.begin() + i);
        } else {
            ++i;
        }
    }

    // Update Particles
    particles.update(dt);

    // Check all collisions
    checkCollisions();

    // Check player health
    if (player.health <= 0) {
        if (!gameOverSoundPlayed) {
            soundManager.play(SND_EXPLOSION_MED, 1.0f);
            soundManager.play(SND_GAME_OVER, 1.0f);
            gameOverSoundPlayed = true;
        }
        particles.addExplosion(player.x, player.y, player.z, 70, 1.0f, 0.4f, 0.1f);
        particles.addShockwave(player.x, player.y, player.z, 14.0f, 1.0f, 0.6f, 0.2f);
        camera.addShake(0.6f, 1.5f);
        state = STATE_GAMEOVER;
    }

    // Check level victory conditions
    if (level < 3 && killsThisLevel >= killsNeeded) {
        startNextLevel();
    }
}

void Game::checkCollisions() {
    // 1. Player Bullets vs Enemies
    for (auto& bullet : playerBullets) {
        if (!bullet.active) continue;

        for (auto& enemy : enemies) {
            if (!enemy.alive) continue;

            float dx = bullet.x - enemy.x;
            float dy = bullet.y - enemy.y;
            float dz = bullet.z - enemy.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            float radSum = bullet.radius + enemy.radius;

            if (distSq <= radSum * radSum) {
                bullet.active = false;
                enemy.takeDamage(bullet.damage);

                // Small impact spark and 3D hit sound
                particles.addExplosion(bullet.x, bullet.y, bullet.z, 8, 1.0f, 0.8f, 0.2f);
                soundManager.play3D(SND_HIT, enemy.x, enemy.z, player.x, player.z, 0.55f);

                if (!enemy.alive) {
                    // Enemy destroyed!
                    kills++;
                    killsThisLevel++;

                    if (enemy.type == ENEMY_BOSS) {
                        score += 5000;
                        particles.addExplosion(enemy.x, enemy.y, enemy.z, 120, 1.0f, 0.3f, 0.1f);
                        particles.addShockwave(enemy.x, enemy.y, enemy.z, 22.0f, 1.0f, 0.8f, 0.2f);
                        camera.addShake(0.8f, 2.0f);
                        soundManager.play(SND_EXPLOSION_BOSS, 1.0f);
                        if (!victorySoundPlayed) {
                            soundManager.play(SND_VICTORY, 1.0f);
                            victorySoundPlayed = true;
                        }
                        state = STATE_VICTORY;
                    } else if (enemy.type == ENEMY_FIGHTER) {
                        score += 350;
                        particles.addExplosion(enemy.x, enemy.y, enemy.z, 40, 1.0f, 0.5f, 0.1f);
                        camera.addShake(0.25f, 0.5f);
                        soundManager.play3D(SND_EXPLOSION_MED, enemy.x, enemy.z, player.x, player.z, 0.9f);
                    } else {
                        score += 150;
                        particles.addExplosion(enemy.x, enemy.y, enemy.z, 25, 1.0f, 0.2f, 0.2f);
                        soundManager.play3D(SND_EXPLOSION_SMALL, enemy.x, enemy.z, player.x, player.z, 0.85f);
                    }

                    if (score > highScore) highScore = score;

                    // Chance to drop power-up
                    if (rand() % 100 < 28 && enemy.type != ENEMY_BOSS) {
                        int pType = rand() % 3;
                        powerups.push_back(PowerUp(enemy.x, enemy.y, enemy.z, pType));
                    }
                }
                break;
            }
        }
    }

    // 2. Player Bullets vs Asteroids
    for (auto& bullet : playerBullets) {
        if (!bullet.active) continue;

        for (auto& a : asteroids) {
            if (!a.active) continue;

            float dx = bullet.x - a.x;
            float dy = bullet.y - a.y;
            float dz = bullet.z - a.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            float radSum = bullet.radius + a.radius;

            if (distSq <= radSum * radSum) {
                bullet.active = false;
                a.health -= bullet.damage;
                particles.addExplosion(bullet.x, bullet.y, bullet.z, 6, 0.7f, 0.6f, 0.5f);
                soundManager.play3D(SND_HIT, a.x, a.z, player.x, player.z, 0.45f);

                if (a.health <= 0) {
                    a.active = false;
                    score += 80;
                    if (score > highScore) highScore = score;
                    particles.addExplosion(a.x, a.y, a.z, 30, 0.65f, 0.6f, 0.55f);
                    particles.addShockwave(a.x, a.y, a.z, a.radius * 2.5f, 0.8f, 0.7f, 0.6f);
                    soundManager.play3D(SND_EXPLOSION_SMALL, a.x, a.z, player.x, player.z, 0.8f);

                    // Chance to drop powerup
                    if (rand() % 100 < 35) {
                        int pType = rand() % 3;
                        powerups.push_back(PowerUp(a.x, a.y, a.z, pType));
                    }
                }
                break;
            }
        }
    }

    // 3. Enemy Bullets vs Player
    for (auto& bullet : enemyBullets) {
        if (!bullet.active) continue;

        float dx = bullet.x - player.x;
        float dy = bullet.y - player.y;
        float dz = bullet.z - player.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float radSum = bullet.radius + player.radius;

        if (distSq <= radSum * radSum) {
            bullet.active = false;
            bool hadShield = (player.shield > 0.0f);
            player.takeDamage(bullet.damage, particles);
            camera.addShake(0.3f, 0.8f);
            if (hadShield) {
                soundManager.play(SND_SHIELD_HIT, 0.85f);
            } else {
                soundManager.play(SND_HULL_HIT, 0.95f);
            }
        }
    }

    // 4. Player vs Asteroids (Ramming)
    for (auto& a : asteroids) {
        if (!a.active) continue;

        float dx = player.x - a.x;
        float dy = player.y - a.y;
        float dz = player.z - a.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float radSum = player.radius + a.radius;

        if (distSq <= radSum * radSum) {
            a.active = false;
            bool hadShield = (player.shield > 0.0f);
            player.takeDamage(35, particles);
            particles.addExplosion(a.x, a.y, a.z, 35, 0.8f, 0.4f, 0.2f);
            camera.addShake(0.5f, 1.3f);
            soundManager.play3D(SND_EXPLOSION_MED, a.x, a.z, player.x, player.z, 0.9f);
            if (hadShield) {
                soundManager.play(SND_SHIELD_HIT, 0.8f);
            } else {
                soundManager.play(SND_HULL_HIT, 0.95f);
            }
        }
    }

    // 5. Player vs Enemies (Ramming)
    for (auto& enemy : enemies) {
        if (!enemy.alive || enemy.type == ENEMY_BOSS) continue;

        float dx = player.x - enemy.x;
        float dy = player.y - enemy.y;
        float dz = player.z - enemy.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float radSum = player.radius + enemy.radius;

        if (distSq <= radSum * radSum) {
            enemy.alive = false;
            bool hadShield = (player.shield > 0.0f);
            player.takeDamage(25, particles);
            particles.addExplosion(enemy.x, enemy.y, enemy.z, 35, 1.0f, 0.4f, 0.1f);
            camera.addShake(0.4f, 1.0f);
            soundManager.play3D(SND_EXPLOSION_SMALL, enemy.x, enemy.z, player.x, player.z, 0.85f);
            if (hadShield) {
                soundManager.play(SND_SHIELD_HIT, 0.8f);
            } else {
                soundManager.play(SND_HULL_HIT, 0.95f);
            }
        }
    }

    // 6. Player vs PowerUps
    for (auto& pup : powerups) {
        if (!pup.active) continue;

        float dx = player.x - pup.x;
        float dy = player.y - pup.y;
        float dz = player.z - pup.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float radSum = player.radius + pup.radius;

        if (distSq <= radSum * radSum) {
            pup.active = false;
            score += 200;
            if (score > highScore) highScore = score;

            if (pup.type == POWERUP_SHIELD) {
                player.addShield(45);
                particles.addShockwave(player.x, player.y, player.z, 5.0f, 0.2f, 0.8f, 1.0f);
                soundManager.play(SND_POWERUP_SHIELD, 0.9f);
            } else if (pup.type == POWERUP_HEALTH) {
                player.heal(35);
                particles.addShockwave(player.x, player.y, player.z, 5.0f, 0.2f, 1.0f, 0.3f);
                soundManager.play(SND_POWERUP_HEALTH, 0.9f);
            } else if (pup.type == POWERUP_TRIPLE_SHOT) {
                player.activateTripleShot(14.0f);
                particles.addShockwave(player.x, player.y, player.z, 6.0f, 1.0f, 0.85f, 0.1f);
                soundManager.play(SND_POWERUP_WEAPON, 0.9f);
            }
        }
    }
}

void Game::renderStars() {
    glDisable(GL_LIGHTING);
    glPointSize(1.8f);
    glBegin(GL_POINTS);
    for (const auto& s : stars) {
        glColor3f(s.r, s.g, s.b);
        glVertex3f(s.x, s.y, s.z);
    }
    glEnd();
    glPointSize(1.0f);
    glEnable(GL_LIGHTING);
}

void Game::renderAsteroids() {
    GLfloat rockDiffuse[] = { 0.45f, 0.42f, 0.40f, 1.0f };
    GLfloat rockSpec[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, rockDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, rockSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 16.0f);

    for (const auto& a : asteroids) {
        if (!a.active) continue;
        glPushMatrix();
        glTranslatef(a.x, a.y, a.z);
        glRotatef(a.rotX, 1.0f, 0.0f, 0.0f);
        glRotatef(a.rotY, 0.0f, 1.0f, 0.0f);
        glRotatef(a.rotZ, 0.0f, 0.0f, 1.0f);

        // Craggy faceted rock shape
        glScalef(a.radius, a.radius * 0.85f, a.radius * 1.15f);
        glutSolidDodecahedron();

        glPopMatrix();
    }
}

void Game::renderText(float x, float y, const std::string& text, void* font) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void Game::drawBar(float x, float y, float width, float height, float percent, float r, float g, float b) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 1.0f) percent = 1.0f;

    // Dark background
    glColor4f(0.1f, 0.1f, 0.15f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Filled progress
    glColor4f(r, g, b, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width * percent, y);
    glVertex2f(x + width * percent, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Sleek frame outline
    glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
    glLineWidth(1.0f);
}

void Game::renderHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    char buf[128];

    if (state == STATE_PLAYING || state == STATE_PAUSED) {
        // --- 1. Top-Left: Player Hull and Energy Shield Bars ---
        float hudX = 25.0f;
        float hudY = windowHeight - 45.0f;

        // Health Bar
        float hpPercent = (float)player.health / (float)player.maxHealth;
        float hpR = (1.0f - hpPercent) * 1.2f;
        float hpG = hpPercent * 1.0f;
        if (hpR > 1.0f) hpR = 1.0f;
        if (hpG > 1.0f) hpG = 1.0f;

        glColor3f(0.85f, 0.95f, 1.0f);
        snprintf(buf, sizeof(buf), "HULL: %d / %d", player.health, player.maxHealth);
        renderText(hudX, hudY + 12.0f, buf, GLUT_BITMAP_HELVETICA_12);
        drawBar(hudX, hudY - 5.0f, 180.0f, 14.0f, hpPercent, hpR, hpG, 0.15f);

        // Shield Bar
        float shieldPercent = player.shield / player.maxShield;
        glColor3f(0.4f, 0.9f, 1.0f);
        snprintf(buf, sizeof(buf), "SHIELD: %d%%", (int)player.shield);
        renderText(hudX, hudY - 22.0f, buf, GLUT_BITMAP_HELVETICA_12);
        drawBar(hudX, hudY - 39.0f, 180.0f, 14.0f, shieldPercent, 0.1f, 0.75f, 1.0f);

        // Weapon / Powerup Status
        if (player.tripleShotTimer > 0.0f) {
            glColor3f(1.0f, 0.85f, 0.1f);
            snprintf(buf, sizeof(buf), "TRIPLE SHOT ACTIVE [%.1fs]", player.tripleShotTimer);
            renderText(hudX, hudY - 58.0f, buf, GLUT_BITMAP_HELVETICA_12);
        } else {
            glColor3f(0.6f, 0.75f, 0.85f);
            renderText(hudX, hudY - 58.0f, "WEAPON: TWIN BLASTER", GLUT_BITMAP_HELVETICA_12);
        }

        // Audio Status Indicator
        if (soundManager.isMuted()) {
            glColor3f(0.95f, 0.35f, 0.35f);
            renderText(hudX, hudY - 76.0f, "AUDIO: MUTED [Press M]", GLUT_BITMAP_HELVETICA_12);
        } else {
            glColor3f(0.45f, 0.9f, 0.55f);
            snprintf(buf, sizeof(buf), "AUDIO: %d%% [M to mute, +/- vol]", (int)(soundManager.getMasterVolume() * 100.0f));
            renderText(hudX, hudY - 76.0f, buf, GLUT_BITMAP_HELVETICA_12);
        }

        // --- 2. Top-Right: Score, Level, Kills ---
        float rightX = windowWidth - 230.0f;
        glColor3f(1.0f, 0.95f, 0.3f);
        snprintf(buf, sizeof(buf), "SCORE: %06d", score);
        renderText(rightX, windowHeight - 35.0f, buf, GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.8f, 0.8f, 0.85f);
        snprintf(buf, sizeof(buf), "HIGH SCORE: %06d", highScore);
        renderText(rightX, windowHeight - 55.0f, buf, GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.4f, 0.9f, 1.0f);
        snprintf(buf, sizeof(buf), "LEVEL %d / 3", level);
        renderText(rightX, windowHeight - 75.0f, buf, GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.7f, 0.75f, 0.8f);
        if (level < 3) {
            snprintf(buf, sizeof(buf), "SECTOR TARGET: %d / %d", killsThisLevel, killsNeeded);
        } else {
            snprintf(buf, sizeof(buf), "TARGET: DESTROY BOSS");
        }
        renderText(rightX, windowHeight - 95.0f, buf, GLUT_BITMAP_HELVETICA_12);

        // --- 3. Level 3 Boss Health Bar (Top Center) ---
        if (level == 3) {
            for (const auto& enemy : enemies) {
                if (enemy.type == ENEMY_BOSS && enemy.alive) {
                    float bossPercent = (float)enemy.health / (float)enemy.maxHealth;
                    float bossBarW = 440.0f;
                    float bossBarX = (windowWidth - bossBarW) / 2.0f;
                    float bossBarY = windowHeight - 42.0f;

                    glColor3f(1.0f, 0.2f, 0.2f);
                    renderText(bossBarX + 110.0f, bossBarY + 16.0f, "WARNING: DREADNOUGHT MOTHERSHIP", GLUT_BITMAP_HELVETICA_18);
                    drawBar(bossBarX, bossBarY, bossBarW, 18.0f, bossPercent, 0.95f, 0.15f, 0.15f);

                    snprintf(buf, sizeof(buf), "%d / %d", enemy.health, enemy.maxHealth);
                    glColor3f(1.0f, 1.0f, 1.0f);
                    renderText(bossBarX + bossBarW / 2.0f - 25.0f, bossBarY + 3.0f, buf, GLUT_BITMAP_HELVETICA_12);
                    break;
                }
            }
        }

        // --- 4. Level Transition Banner ---
        if (levelBannerTimer > 0.0f) {
            float bannerAlpha = (levelBannerTimer > 1.0f) ? 1.0f : levelBannerTimer;
            glColor4f(0.2f, 0.8f, 1.0f, bannerAlpha);
            if (level == 1) {
                renderText(windowWidth / 2.0f - 140.0f, windowHeight / 2.0f + 70.0f, "SECTOR 1: ASTEROID BELT", GLUT_BITMAP_HELVETICA_18);
            } else if (level == 2) {
                renderText(windowWidth / 2.0f - 140.0f, windowHeight / 2.0f + 70.0f, "SECTOR 2: NEBULA OUTPOST", GLUT_BITMAP_HELVETICA_18);
            } else {
                glColor4f(1.0f, 0.2f, 0.2f, bannerAlpha);
                renderText(windowWidth / 2.0f - 160.0f, windowHeight / 2.0f + 70.0f, "SECTOR 3: BOSS CONFRONTATION", GLUT_BITMAP_HELVETICA_18);
            }
        }

        // --- 5. Mouse-Controlled Targeting Reticle (Crosshairs) ---
        float targetScreenX = (float)mouseX;
        float targetScreenY = (float)(windowHeight - mouseY);

        // Clamp inside window borders
        if (targetScreenX < 20.0f) targetScreenX = 20.0f;
        if (targetScreenX > windowWidth - 20.0f) targetScreenX = windowWidth - 20.0f;
        if (targetScreenY < 20.0f) targetScreenY = 20.0f;
        if (targetScreenY > windowHeight - 20.0f) targetScreenY = windowHeight - 20.0f;

        glPushMatrix();
        glTranslatef(targetScreenX, targetScreenY, 0.0f);

        if (targetLocked) {
            // High-Tech Red / Amber Lock-On Reticle
            glColor4f(1.0f, 0.2f, 0.1f, 0.95f);
            glLineWidth(2.0f);

            // Diamond bracket
            float dSize = 22.0f;
            glBegin(GL_LINE_LOOP);
            glVertex2f(0.0f, dSize);
            glVertex2f(dSize, 0.0f);
            glVertex2f(0.0f, -dSize);
            glVertex2f(-dSize, 0.0f);
            glEnd();

            // Corner targeting brackets
            float bSize = 15.0f;
            glBegin(GL_LINES);
            glVertex2f(-bSize, bSize); glVertex2f(-bSize + 6.0f, bSize);
            glVertex2f(-bSize, bSize); glVertex2f(-bSize, bSize - 6.0f);

            glVertex2f(bSize, bSize); glVertex2f(bSize - 6.0f, bSize);
            glVertex2f(bSize, bSize); glVertex2f(bSize, bSize - 6.0f);

            glVertex2f(-bSize, -bSize); glVertex2f(-bSize + 6.0f, -bSize);
            glVertex2f(-bSize, -bSize); glVertex2f(-bSize, -bSize + 6.0f);

            glVertex2f(bSize, -bSize); glVertex2f(bSize - 6.0f, -bSize);
            glVertex2f(bSize, -bSize); glVertex2f(bSize, -bSize + 6.0f);
            glEnd();

            // Center pip
            glPointSize(4.0f);
            glBegin(GL_POINTS);
            glVertex2f(0.0f, 0.0f);
            glEnd();
            glPointSize(1.0f);

            // Lock-on info label
            char lockBuf[64];
            snprintf(lockBuf, sizeof(lockBuf), "[LOCK: %d/%d]", lockedTargetHealth, lockedTargetMaxHealth);
            glColor3f(1.0f, 0.35f, 0.15f);
            renderText(28.0f, -6.0f, lockBuf, GLUT_BITMAP_HELVETICA_12);
            renderText(28.0f, 10.0f, lockedTargetName, GLUT_BITMAP_HELVETICA_12);

        } else {
            // Sleek Cyan Dynamic Mouse Aim Reticle
            glColor4f(0.15f, 0.85f, 1.0f, 0.85f);
            glLineWidth(1.6f);

            // Crosshair ticks
            glBegin(GL_LINES);
            glVertex2f(-20.0f, 0.0f); glVertex2f(-6.0f, 0.0f);
            glVertex2f(6.0f, 0.0f);  glVertex2f(20.0f, 0.0f);
            glVertex2f(0.0f, -20.0f); glVertex2f(0.0f, -6.0f);
            glVertex2f(0.0f, 6.0f);  glVertex2f(0.0f, 20.0f);
            glEnd();

            // Center dot
            glPointSize(3.0f);
            glBegin(GL_POINTS);
            glVertex2f(0.0f, 0.0f);
            glEnd();
            glPointSize(1.0f);

            // Outer circular bracket ring
            glBegin(GL_LINE_LOOP);
            const int segs = 24;
            for (int i = 0; i < segs; ++i) {
                float th = 2.0f * 3.14159265f * (float)i / (float)segs;
                glVertex2f(cosf(th) * 13.0f, sinf(th) * 13.0f);
            }
            glEnd();
        }
        glLineWidth(1.0f);
        glPopMatrix();

        // --- 6. Bottom Controls Bar ---
        glColor4f(0.6f, 0.7f, 0.8f, 0.7f);
        const char* camStr = (camera.mode == CAM_THIRD_PERSON) ? "3RD PERSON" :
                             (camera.mode == CAM_FIRST_PERSON) ? "COCKPIT" : "FREE ORBIT";
        snprintf(buf, sizeof(buf), "[MOUSE] Aim / Fire | [W/S] Pitch | [A/D] Roll | [Q/E] Thrust | [C] Cam (%s) | [P] Pause | [M] %s", camStr, soundManager.isMuted() ? "Unmute" : "Mute");
        renderText(25.0f, 20.0f, buf, GLUT_BITMAP_HELVETICA_12);

        // Hit flash screen border
        if (player.hitFlashTimer > 0.0f) {
            float alpha = (player.hitFlashTimer / 0.35f) * 0.35f;
            glColor4f(1.0f, 0.0f, 0.0f, alpha);
            glBegin(GL_QUADS);
            glVertex2f(0, 0);
            glVertex2f(windowWidth, 0);
            glVertex2f(windowWidth, windowHeight);
            glVertex2f(0, windowHeight);
            glEnd();
        }
    }

    // --- 7. Pause Overlay Screen ---
    if (state == STATE_PAUSED) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
        glEnd();

        glColor3f(1.0f, 0.9f, 0.2f);
        renderText(windowWidth / 2.0f - 75.0f, windowHeight / 2.0f + 20.0f, "GAME PAUSED", GLUT_BITMAP_HELVETICA_18);
        glColor3f(0.8f, 0.8f, 0.85f);
        renderText(windowWidth / 2.0f - 140.0f, windowHeight / 2.0f - 15.0f, "Press [P] to Resume | [M] Toggle Audio", GLUT_BITMAP_HELVETICA_12);
    }

    // --- 8. Start Menu Screen ---
    if (state == STATE_MENU) {
        // Dark translucent backing
        glColor4f(0.02f, 0.04f, 0.08f, 0.75f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
        glEnd();

        // Glowing Title
        glColor3f(0.1f, 0.85f, 1.0f);
        renderText(windowWidth / 2.0f - 180.0f, windowHeight / 2.0f + 140.0f, "3D SPACE DEFENDER", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.7f, 0.85f, 1.0f);
        renderText(windowWidth / 2.0f - 165.0f, windowHeight / 2.0f + 110.0f, "OpenGL & FreeGLUT Shooting Game", GLUT_BITMAP_HELVETICA_12);

        // Box Frame
        float boxX = windowWidth / 2.0f - 240.0f;
        float boxY = windowHeight / 2.0f - 120.0f;
        float boxW = 480.0f;
        float boxH = 200.0f;

        glColor4f(0.08f, 0.15f, 0.28f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(boxX, boxY);
        glVertex2f(boxX + boxW, boxY);
        glVertex2f(boxX + boxW, boxY + boxH);
        glVertex2f(boxX, boxY + boxH);
        glEnd();

        glColor3f(0.2f, 0.7f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(boxX, boxY);
        glVertex2f(boxX + boxW, boxY);
        glVertex2f(boxX + boxW, boxY + boxH);
        glVertex2f(boxX, boxY + boxH);
        glEnd();
        glLineWidth(1.0f);

        // Instructions
        glColor3f(1.0f, 0.9f, 0.3f);
        renderText(boxX + 180.0f, boxY + 165.0f, "MISSION BRIEFING", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.9f, 0.9f, 0.95f);
        renderText(boxX + 35.0f, boxY + 135.0f, "* MOUSE : Aim Target Crosshair", GLUT_BITMAP_HELVETICA_12);
        renderText(boxX + 35.0f, boxY + 110.0f, "* LEFT CLICK / SPACE : Fire Plasma Blasters", GLUT_BITMAP_HELVETICA_12);
        renderText(boxX + 35.0f, boxY + 85.0f,  "* W / S : Move Up / Down | A / D : Bank Left / Right", GLUT_BITMAP_HELVETICA_12);
        renderText(boxX + 35.0f, boxY + 60.0f,  "* Q / E : Reverse / Forward Thrust", GLUT_BITMAP_HELVETICA_12);
        renderText(boxX + 35.0f, boxY + 35.0f,  "* C : Camera | P : Pause | M : Mute Sound | +/- : Vol", GLUT_BITMAP_HELVETICA_12);

        // Press Enter or Click prompt
        glColor3f(0.2f, 1.0f, 0.4f);
        renderText(windowWidth / 2.0f - 160.0f, windowHeight / 2.0f - 160.0f, "CLICK OR PRESS ENTER TO LAUNCH", GLUT_BITMAP_HELVETICA_18);
    }

    // --- 9. Game Over Screen ---
    if (state == STATE_GAMEOVER) {
        glColor4f(0.12f, 0.02f, 0.02f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
        glEnd();

        glColor3f(1.0f, 0.15f, 0.15f);
        renderText(windowWidth / 2.0f - 95.0f, windowHeight / 2.0f + 80.0f, "MISSION FAILED", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.9f, 0.9f, 0.9f);
        snprintf(buf, sizeof(buf), "FINAL SCORE: %d", score);
        renderText(windowWidth / 2.0f - 75.0f, windowHeight / 2.0f + 30.0f, buf, GLUT_BITMAP_HELVETICA_18);

        snprintf(buf, sizeof(buf), "ENEMIES ELIMINATED: %d", kills);
        renderText(windowWidth / 2.0f - 90.0f, windowHeight / 2.0f, buf, GLUT_BITMAP_HELVETICA_12);

        snprintf(buf, sizeof(buf), "SECTOR REACHED: LEVEL %d", level);
        renderText(windowWidth / 2.0f - 85.0f, windowHeight / 2.0f - 25.0f, buf, GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.9f, 1.0f);
        renderText(windowWidth / 2.0f - 110.0f, windowHeight / 2.0f - 75.0f, "PRESS [R] TO RESTART MISSION", GLUT_BITMAP_HELVETICA_18);
    }

    // --- 10. Victory Screen ---
    if (state == STATE_VICTORY) {
        glColor4f(0.02f, 0.12f, 0.05f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
        glEnd();

        glColor3f(0.2f, 1.0f, 0.35f);
        renderText(windowWidth / 2.0f - 145.0f, windowHeight / 2.0f + 80.0f, "VICTORY - GALAXY DEFENDED!", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 0.9f, 0.2f);
        snprintf(buf, sizeof(buf), "BOSS DESTROYED - FINAL SCORE: %d", score);
        renderText(windowWidth / 2.0f - 150.0f, windowHeight / 2.0f + 30.0f, buf, GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.9f, 0.9f, 0.95f);
        snprintf(buf, sizeof(buf), "TOTAL ENEMIES NEUTRALIZED: %d", kills);
        renderText(windowWidth / 2.0f - 115.0f, windowHeight / 2.0f, buf, GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.85f, 1.0f);
        renderText(windowWidth / 2.0f - 110.0f, windowHeight / 2.0f - 60.0f, "PRESS [R] TO PLAY AGAIN", GLUT_BITMAP_HELVETICA_18);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Game::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 1. Setup Camera View Matrix
    camera.apply();

    // 2. Setup Lighting in World Space
    GLfloat sunPos[] = { 40.0f, 60.0f, -50.0f, 1.0f };
    GLfloat sunColor[] = { 1.0f, 0.95f, 0.9f, 1.0f };
    GLfloat sunAmbient[] = { 0.22f, 0.24f, 0.32f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, sunPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunColor);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);

    // Dynamic combat light (near player / battle zone)
    GLfloat battleLightPos[] = { player.x, player.y + 5.0f, player.z - 8.0f, 1.0f };
    GLfloat battleLightColor[] = { 0.3f, 0.5f, 0.9f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, battleLightPos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, battleLightColor);

    // 3. Render 3D Environment Elements
    renderStars();
    renderAsteroids();

    // 4. Render Power-ups
    for (const auto& pup : powerups) {
        pup.draw();
    }

    // 5. Render Projectiles
    for (const auto& b : playerBullets) {
        b.draw();
    }
    for (const auto& b : enemyBullets) {
        b.draw();
    }

    // 6. Render Enemies
    for (const auto& enemy : enemies) {
        enemy.draw();
    }

    // 7. Render Player Ship (in 3rd Person & Free modes)
    if (camera.mode != CAM_FIRST_PERSON && state != STATE_GAMEOVER) {
        player.draw();
    }

    // 8. Render Particles & Explosions
    particles.draw();

    // 9. Render 2D HUD / Menus Overlay
    renderHUD();

    glutSwapBuffers();
}
