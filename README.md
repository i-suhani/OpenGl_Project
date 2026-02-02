# OpenGl_Project
# 🎮 Lyra – Maze of Destiny

Lyra – Maze of Destiny is a **2D maze-based adventure game** developed in **C using OpenGL (GLUT)**.  
The player controls Lyra as she travels through multiple maze levels, recovers lost gear, and defeats a final boss to complete her journey.

---

## 📌 Project Overview

- The game is built using **tile-based maze design**
- It contains **5 handcrafted levels**
- Each level has a short story introduction
- The final level includes a **boss fight**
- The project demonstrates **OpenGL rendering, game states, and basic algorithms**

---

## 🧩 Game Story

- **Level 1:** Lyra wakes up in a ruined land without her armour, shield, or sword  
- **Level 2:** She explores a river area and recovers her armour  
- **Level 3:** She enters a broken fortress to find her shield  
- **Level 4:** She crosses a burning land to reclaim her sword  
- **Level 5:** Fully equipped, Lyra reaches the final gate and defeats the boss  

---

## 🎮 Game Features

- 5 maze-based levels with increasing size and difficulty  
- Story screen with fade-in animation before each level  
- Tile-based movement with wall collision  
- Inventory system (Armour, Shield, Sword)  
- Player appearance upgrades after collecting items  
- Final boss with health bar and attack mechanics  
- Automatic maze validation using BFS path checking  
- Dynamic window resizing with aspect-ratio preservation  

---

## 🛠 Technologies Used

- **C Programming Language**
- **OpenGL**
- **GLUT (OpenGL Utility Toolkit)**
- Basic algorithms (BFS for path checking)
- 2D orthographic projection

---

## ⌨ Controls

| Key | Action |
|----|--------|
| Arrow Keys | Move player |
| A | Attack boss (when adjacent) |
| ENTER | Continue story / Start level |
| ESC | Exit game |

---

## 🗂 Project Structure

- Maze layouts defined using ASCII characters  
- Levels stored using a `FixedLevel` structure  
- Game states managed using an enum:
  - `STORY`
  - `PLAY`
  - `ENDING`
- Rendering handled using OpenGL quads  
- Input handled via GLUT keyboard callbacks  

---

### Requirements
- GCC compiler
- OpenGL & GLUT installed
