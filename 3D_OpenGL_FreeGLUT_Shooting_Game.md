# 3D Shooting Game using OpenGL & FreeGLUT

# Project Title
3D Space Defender: OpenGL FreeGLUT Shooting Game

## Project Description
A 3D shooting game where the player controls a spaceship in a 3D environment. The player must destroy enemy ships, avoid collisions, survive enemy attacks, and achieve the highest score.

## Main Features

### 3D Graphics
- 3D player spaceship
- 3D enemy ships
- 3D bullets and missiles
- Skybox / space background
- Camera movement

### Gameplay
- Player movement (left, right, forward, backward)
- Shooting system
- Enemy spawning
- Collision detection
- Score system
- Health system
- Game over screen

### Advanced Features
- Multiple levels
- Boss battle
- Power-ups
- Health packs
- Shield system
- Particle explosion effects

## Controls

| Key | Action |
|------|---------|
| W | Move Forward |
| S | Move Backward |
| A | Move Left |
| D | Move Right |
| Space | Fire Bullet |
| Mouse | Camera Control |
| R | Restart |
| ESC | Exit |

## Technologies

- C++
- OpenGL
- FreeGLUT
- GLU

## Required Headers

```cpp
#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
```
## Core Classes

### Player

```cpp
class Player{
public:
    float x,y,z;
    int health;
};
```

### Enemy

```cpp
class Enemy{
public:
    float x,y,z;
    bool alive;
};
```

### Bullet

```cpp
class Bullet{
public:
    float x,y,z;
    bool active;
};
```

## 3D Scene Design

### Environment
- Space battlefield
- Stars
- Asteroids
- Lighting effects

### Camera Modes
- Third Person Camera
- First Person Camera
- Free Camera

## OpenGL Concepts Used

- Perspective Projection
- Lighting
- Materials
- Transformations
- Animation
- Collision Detection
- Depth Buffering

## Game Loop

1. Render Scene
2. Update Player
3. Update Enemies
4. Update Bullets
5. Detect Collisions
6. Update Score
7. Display UI
8. Repeat

## Suggested Folder Structure

```text
3D_Shooting_Game/
│
├── main.cpp
├── Player.h
├── Enemy.h
├── Bullet.h
├── Camera.h
├── Game.h
├── Game.cpp
├── assets/
│   ├── textures/
│   ├── models/
│   └── sounds/
└── README.md
```

## Computer Graphics Topics Covered

- 3D Modeling
- Viewing Transformation
- Projection Transformation
- Lighting and Shading
- Animation
- Camera Systems
- Interactive Graphics

## Future Improvements

- OBJ model loading
- Realistic textures
- Shadow mapping
- Multiplayer mode
- AI enemies
- Physics engine

## Expected Learning Outcome

After completing this project you will understand:

- OpenGL 3D Rendering
- FreeGLUT Event Handling
- Camera Control
- 3D Transformations
- Collision Detection
- Real-time Game Development

## Author

Polash
AIUB - CSE
