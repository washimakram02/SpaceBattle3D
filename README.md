# 3D Space Defender: OpenGL & FreeGLUT Shooting Game

A full-featured 3D arcade space combat simulator built with **C++**, **OpenGL**, **GLU**, and **GLUT / FreeGLUT**.

![Language](https://img.shields.io/badge/Language-C%2B%2B11-blue.svg)
![Graphics](https://img.shields.io/badge/Graphics-OpenGL%20%7C%20GLUT-red.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)

---

## Game Overview

In **3D Space Defender**, the player pilots an advanced starfighter through deep space sectors filled with rogue asteroids and hostile alien armadas. Fight through waves of agile scout interceptors and heavy strike fighters before confronting the massive **Dreadnought Mothership** in an epic multi-phase boss confrontation.

---

## Main Features

### 1. 3D Graphics & Visual Effects
- **Detailed 3D Player Spaceship**: Aerodynamic fuselage, delta wings, wingtip blaster cannons, tinted cockpit canopy, and dual pulsating cyan rocket engine plumes.
- **Dynamic 3D Enemy Models**:
  - **Scout Drone**: Aggressive crimson needle-nose interceptor with swept-forward wings and high mobility.
  - **Heavy Strike Fighter**: Dual-pod bronze gunship armed with twin forward plasma cannons.
  - **Dreadnought Mothership (Boss)**: Colossal warship with armored outrigger wings, bridge tower, exposed glowing energy reactor core, rotating weapon turrets, and quad heavy thrusters.
- **Atmospheric 3D Environment**:
  - 650+ multicolored deep-space stars with real-time warp flight drift.
  - Tumbling craggy 3D asteroids with independent rotation axes and collision physics.
- **Lighting & Materials**:
  - Dual light sources (`GL_LIGHT0` stellar sun illumination and `GL_LIGHT1` dynamic combat battle glow).
  - Material properties with ambient, diffuse, and specular highlights (`GL_SPECULAR`, `GL_SHININESS`).
- **Explosion & Particle System**:
  - Multi-stage fiery explosions with debris fragments, sparks, and alpha blending (`GL_BLEND`).
  - Expanding energy shockwave rings upon ship destruction.
  - Rear engine trail plumes emitted during flight.

### 2. Gameplay Mechanics
- **3D Flight Controls**: Full 6-degree responsiveness with dynamic banking roll and pitch tilt during turns.
- **Combat & Collision Detection**: Accurate 3D spherical bounding collision checks between player, enemy ships, lasers, asteroids, and collectibles.
- **Energy Shield & Hull System**: Dynamic shield barrier that absorbs damage before hull degradation, featuring automatic shield regeneration after avoiding damage.
- **Collectible Power-Ups**:
  - **Shield Booster** (Cyan Orb): Restores +45 energy shield.
  - **Nano-Repair Kit** (Green Cross): Restores +35 hull integrity.
  - **Triple-Shot Blaster** (Golden Prism): Unleashes converging triple laser barrages for 14 seconds.
- **Dynamic Camera Modes**:
  - **3rd Person**: Dynamic trailing chase camera with smooth banking follow.
  - **1st Person (Cockpit)**: Forward viewpoint for dogfighting precision.
  - **Free / Cinematic**: Orbital panoramic inspection view.

### 3. Multi-Level Progression
- **Sector 1 (Asteroid Belt)**: Navigating space debris and hunting fast Scout swarms (Target: 12 Kills).
- **Sector 2 (Nebula Outpost)**: Coordinated strike waves of Scouts and Heavy Strike Fighters (Target: 22 Kills).
- **Sector 3 (Mothership Confrontation)**: Face the Dreadnought Boss featuring multi-angle spread attacks, heavy beam barrages, and rage mode under 45% HP.

### 4. 2D HUD & Overlay Systems
- Orthographic 2D interface (`gluOrtho2D`):
  - Hull integrity & Energy shield meters with dynamic color bars.
  - Dreadnought Boss Health Bar on top center during Sector 3.
  - Score, High Score, Sector progression, and Kill counter.
  - Active weapon status and power-up cooldown display.
  - Tactical center targeting reticle / crosshair.
  - Interactive Start Menu, Pause screen (`P`), Game Over screen, and Victory screen with restart (`R`).

---

## Controls

| Control / Key | Action |
|---|---|
| **MOUSE** | **Aim Target Reticle / Crosshair** in 3D space |
| **LEFT CLICK** / **SPACEBAR** | Fire Plasma Blasters towards Crosshair |
| **RIGHT CLICK** / **C** | Toggle Camera Mode (3rd Person / Cockpit / Free) |
| **W** / **Up Arrow** | Move Up / Climb Altitude |
| **S** / **Down Arrow** | Move Down / Descend Altitude |
| **A** / **Left Arrow** | Bank & Strafe Left |
| **D** / **Right Arrow** | Bank & Strafe Right |
| **Q** | Reverse Thrust / Brake |
| **E** | Forward Thrust / Accelerate |
| **P** | Pause / Unpause Game |
| **R** | Restart Mission (on Game Over or Victory) |
| **ESC** | Exit Game |

---

## Project Structure

```
shooting game/
├── main.cpp                  # GLUT window initialization and main callback loops
├── Game.h & Game.cpp         # Master game engine, state machine, collisions, HUD
├── Player.h                  # Player spaceship geometry, flight mechanics, weapons
├── Enemy.h                   # Scout, Fighter, and Dreadnought Boss models & AI
├── Bullet.h                  # 3D player lasers and enemy plasma projectiles
├── Particle.h                # Particle explosion system and shockwaves
├── PowerUp.h                 # 3D rotating collectible items
├── Camera.h                  # Multi-mode camera with smooth interpolation
├── SpaceDefender3D.cbp       # Code::Blocks Project file
├── build.bat                 # One-click Windows batch compiler
├── run.bat                   # Game launcher script
├── SpaceDefender3D.exe       # Ready-to-play compiled executable
├── glut32.dll                # Bundled GLUT dynamic library
├── include/GL/               # Local OpenGL GLUT / FreeGLUT header files
└── lib/                      # Link libraries (libglut32.a, libfreeglut.a)
```

---

## How to Build and Run

### Method 1: One-Click Scripts (Easiest)
1. **To build**: Double-click [`build.bat`](file:///c:/Projects%20with%20programing_hero/New%20Workspace/shooting%20game/build.bat). It automatically detects MinGW / Code::Blocks and compiles `SpaceDefender3D.exe`.
2. **To play**: Double-click [`run.bat`](file:///c:/Projects%20with%20programing_hero/New%20Workspace/shooting%20game/run.bat) or [`SpaceDefender3D.exe`](file:///c:/Projects%20with%20programing_hero/New%20Workspace/shooting%20game/SpaceDefender3D.exe).

### Method 2: Code::Blocks IDE
1. Open [`SpaceDefender3D.cbp`](file:///c:/Projects%20with%20programing_hero/New%20Workspace/shooting%20game/SpaceDefender3D.cbp) in **Code::Blocks**.
2. Press **F9** (or go to `Build -> Build and Run`).

### Method 3: Command Line (MinGW / GCC)
Open PowerShell or Command Prompt in the project folder and run:
```bash
g++ -std=c++11 -O2 -Wall -I.\include -L.\lib main.cpp Game.cpp -o SpaceDefender3D.exe -lglut32 -lglu32 -lopengl32 -lwinmm
.\SpaceDefender3D.exe
```

---

## Computer Graphics Concepts Implemented
- **3D Geometric Modeling**: Complex multi-part hierarchical meshes constructed using OpenGL primitives (`glutSolidCube`, `glutSolidCone`, `glutSolidSphere`, `glutSolidDodecahedron`, `glutSolidOctahedron`).
- **Viewing & Projections**: Real-time perspective transformations via `gluPerspective` and `gluLookAt` coupled with 2D orthographic overlays via `gluOrtho2D`.
- **Illumination & Materials**: Dual light sources with diffuse, ambient, and specular components (`GL_LIGHT0`, `GL_LIGHT1`, `glMaterialfv`).
- **Alpha Blending & Depth Testing**: Additive blending for energy shields and shockwaves (`GL_SRC_ALPHA`, `GL_ONE`) with depth mask management (`glDepthMask`).
- **Kinematics & Particle Dynamics**: Trajectory physics, velocity decay, drag simulation, and spherical dispersion models.
- **Bounding Volume Collision Detection**: 3D Euclidean distance calculations for hit detection across all moving entities.
"# SpaceBattle3D" 
