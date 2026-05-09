# 3D Mine Game

A personal project — a 3D tactical game built from scratch in C++ and DirectX 11.

> **Demo video:** <https://youtu.be/4HiV34tTbH0>  
>
> **This is an early demo.** Core systems are in place but content, balance, and polish are still in progress.

---

## Overview

Players navigate a tile-based 3D world, collecting coins and avoiding bombs across varied terrain. The project started as a minesweeper concept and grew into a small tactical game with multiplayer support.

**Single-player** — 15×15 grid map. Move your character across terrain tiles, trigger bomb countdowns, collect coins, and reach the goal.

**Multiplayer Battle** — 30×30 grid shared between up to 4 players. Turn-based movement with a movement-point system. Each player manages their own map region and competes for score.

**Map Editor** — Paint and save custom maps in-game, switching between terrain types in real time.

## Characters

Four playable characters, each procedurally rendered (no external model files):

| Character | Role |
|---|---|
| Doctor | Support |
| Soldier | Combat |
| Scout | Mobility |
| Bomb Tech | Bomb specialist |

## Terrain

Movement and effects vary by tile type — ice causes slipping, sand slows movement, grass restores health, water deals damage.

## Tech

- **Renderer**: DirectX 11, hand-written HLSL shaders, two-pass render-to-texture pipeline
- **Input**: DirectInput 8 (keyboard) + DirectX Mouse
- **Models**: Assimp (FBX), with additional procedurally generated geometry
- **Textures**: DirectXTex
- **Window**: 1280×720, virtual UI space 1920×1080

## Build

Open `20240425_DX21_ウィンドウ作成版.sln` in **Visual Studio 2022** and build (`Ctrl+Shift+B`). Targets: **Debug x64** / **Release x64**.

The executable must be run from the project root so it can find `asset/`, `shader.hlsl`, and `skyshader.hlsl`.

---

*Solo developed as a learning and portfolio project.*
