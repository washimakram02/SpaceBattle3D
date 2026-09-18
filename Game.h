#ifndef GAME_H
#define GAME_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/glut.h>
#include <GL/glu.h>
#include <vector>
#include <string>

#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Particle.h"
#include "PowerUp.h"
#include "Camera.h"

enum GameState {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER,
    STATE_VICTORY
};

struct Star {
    float x, y, z;
    float r, g, b;
    float size;
    float speed;
};

struct Asteroid {
    float x, y, z;
    float vx, vy, vz;
    float rotX, rotY, rotZ;
    float rotSpeedX, rotSpeedY;
    float radius;
    int health;
    int maxHealth;
    bool active;
};

class Game {
public:
    int windowWidth;
    int windowHeight;
    GameState state;

    Player player;
    Camera camera;
    ParticleSystem particles;

    std::vector<Enemy> enemies;
    std::vector<Bullet> playerBullets;
    std::vector<Bullet> enemyBullets;
    std::vector<PowerUp> powerups;
    std::vector<Asteroid> asteroids;
    std::vector<Star> stars;

    // Progression
    int score;
    int highScore;
    int level;
    int kills;
    int killsThisLevel;
    int killsNeeded;

    // Spawning timers
    float enemySpawnTimer;
    float enemySpawnInterval;
    float asteroidSpawnTimer;
    float levelBannerTimer;

    // Input tracking
    bool keys[256];
    bool specialKeys[256];
    int mouseX, mouseY;
    bool spacePressed;
    bool mouseLeftDown;
    bool mouseRightDown;

    // Mouse targeting & lock-on state
    float aimTargetX, aimTargetY, aimTargetZ;
    bool targetLocked;
    std::string lockedTargetName;
    int lockedTargetHealth;
    int lockedTargetMaxHealth;

    Game();

    void init(int w, int h);
    void reset();
    void startNextLevel();

    void handleKeyDown(unsigned char key, int x, int y);
    void handleKeyUp(unsigned char key, int x, int y);
    void handleSpecialKeyDown(int key, int x, int y);
    void handleSpecialKeyUp(int key, int x, int y);
    void handlePassiveMouse(int x, int y);
    void handleMouseClick(int button, int state, int x, int y);

    void update(float dt);
    void render();

private:
    void initStars();
    void spawnAsteroid();
    void spawnEnemies(float dt);
    void checkCollisions();
    void renderStars();
    void renderAsteroids();
    void renderHUD();
    void renderText(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18);
    void drawBar(float x, float y, float width, float height, float percent, float r, float g, float b);
};

#endif // GAME_H
