# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Open `20240425_DX21_ウィンドウ作成版.sln` in Visual Studio 2022 and build via **Build → Build Solution** (`Ctrl+Shift+B`). Target configurations are **Debug x64** and **Release x64**. There is no command-line build script — MSBuild is used internally by Visual Studio.

Run the resulting `.exe` directly; the working directory must contain `asset/` and the shader files (`shader.hlsl`, `skyshader.hlsl`).

## Architecture Overview

This is a DirectX 11 3D game written in C++/Win32. All source files sit in the root directory (no subdirectory organization except `FrameWork/` and `MultiBattle/`).

### Execution flow

```
WinMain (main.cpp)
  └── 60 FPS game loop
        ├── UpdateManager()   → routes to active scene's Update
        └── DrawManager()     → routes to active scene's Draw
```

`Manager.cpp` manages top-level scenes (`SCENE_TITLE`, `SCENE_GAME`, `SCENE_RESULT`). Currently only `SCENE_GAME` is wired up.

### Scene and mode layering

Inside the game scene, `SceneManager.cpp` manages **app modes** (`AppMode` enum):

| Mode | Description |
|---|---|
| `APP_MODE_PREVIEW` | Default flyover view |
| `APP_MODE_GAME` | Single-player gameplay |
| `APP_MODE_EDIT` | In-game map editor |
| `APP_MODE_MULTIPLAYER_BATTLE` | Local multiplayer battle (30×30 map) |
| `APP_MODE_BATTLE_HALL` | Battle lobby |
| `APP_MODE_CHARACTER` | Character selection |
| `APP_MODE_PAUSE` / `APP_MODE_GAMEOVER` | Overlay states |

`Game.cpp` is the coordinator for the game scene: it calls Init/Uninit/Update/Draw on every subsystem and routes per-mode logic.

### Rendering pipeline (two-pass)

1. **`BeginPE()`** — switches render target to an offscreen texture; all 3D content is drawn here (`Draw3DScene()`).
2. **`Begin()`** — switches back to the backbuffer; the offscreen texture is drawn as a fullscreen sprite, then 2D UI is drawn on top (`Draw2DOverlay()`).
3. **`Present()`** — flips the swapchain.

`renderer.cpp` wraps the DirectX 11 device/context and exposes helpers for world/view/projection matrices, materials, lighting, depth, blend, and cull states.

### Map / world

- `Box.cpp/h`: Tile-based map. Single-player maps are **15×15** (`g_Box[]`), battle maps are **30×30** (`g_Box[]` + `g_Obstacle[]`). Each cell is a `BoxObject` with a `MAP_ELEMENT_TYPE` (NORMAL_GROUND, WALL, TREE, ICE_GROUND, SAND_GROUND, COIN, BOMB, etc.).
- `MapEditor.cpp`: Runtime map editing, saved/loaded from file.

### Key subsystems

| File | Role |
|---|---|
| `renderer.cpp/h` | DX11 device wrapper, shader constants, render states |
| `Camera.cpp/h` | `Camera` class; preset angles for PREVIEW and EDIT modes |
| `Ball.cpp/h` | Player object (`BallObject`); physics, movement, coin collection, bomb mechanics |
| `Collision.cpp/h` | Sphere-box, box-box, box-point collision helpers |
| `UIManager.h/cpp` | Static class managing all UI elements, screens, button callbacks |
| `GeometricTextRenderer.cpp/h` | Custom pixel-font text rendering on GPU |
| `ParticlaEffect.cpp/h` | `ParticleSystem` and `FloatingTextSystem` |
| `model.cpp/h` | Assimp-based FBX model loading; `ModelLoad` / `ModelDraw` |
| `sprite.cpp/h` | 2D sprite and billboard drawing |
| `texture.cpp/h` | Texture loading via DirectXTex |
| `MultiplayerBattle.cpp/h` | `MultiplayerBattleManager` — local multiplayer battle logic |
| `BattleHall.cpp/h` | Battle lobby UI and flow |
| `CharactorScene.cpp/h` | Character selection screen |
| `SkyDome.cpp/h` | Sky dome background |
| `MiniMap.cpp/h` | Mini-map rendering |
| `FrameWork/TextureManager.cpp` | Global texture cache |
| `TextureAtlas.cpp/h` | Sprite sheet / atlas management |
| `ProceduralModels.cpp/h` | GPU-generated geometry (no FBX required) |

### Third-party dependencies (pre-built, in repo root)

- **DirectX 11** — `d3d11.lib`, `d3dcompiler.lib`, `dxguid.lib`, `dinput8.lib`
- **DirectXTex** — `DirectXTex_Debug.lib` / `DirectXTex_Release.lib`
- **Assimp** — `assimp-vc143-mt.lib` / `assimp-vc143-mt.dll`

### Input

- **Keyboard**: `keyboard.h/cpp` — DirectInput8. Query with `Keyboard_IsKeyDown(KK_*)`. Call `keycopy()` once per frame (done in `WinMain`) to latch previous state.
- **Mouse**: `mouse.h/cpp` — DirectX Mouse wrapper. Supports absolute (`MOUSE_POSITION_MODE_ABSOLUTE`) and relative modes.

### Map size constants

`Box.h` is the single source of truth for map dimensions:

```cpp
#define MAPSIZE_X / MAPSIZE_Z       15   // single-player map
#define BATTLE_MAP_SIZE_X / _Z      30   // multiplayer battle map
```

`MultiplayerBattle.h` **must not** redefine these — it includes `Box.h` and inherits the macros.
`MOVEMENT_COST[13]` and `TERRAIN_EFFECTS[13]` are defined in `Box.cpp` and declared `extern const` in `Box.h`; the index corresponds directly to `MAP_ELEMENT_TYPE` (0-12).

### Conventions

- Every subsystem follows the `Init` / `Uninit` / `Update` / `Draw` pattern.
- Globals use the `g_` prefix.
- `SCREEN_WIDTH` / `SCREEN_HEIGHT` are 1920×1080 — this is a **virtual 2D coordinate space**, not the window size. `SetWorldViewProjection2D()` sets up an orthographic projection over this space; the 1280×720 window scales it automatically. Do not change these values.
- `GetBall()` is declared only in `Ball.h`. Do not add it to other headers.
- Never use `memset` on `BoxObject` — use `obj = BoxObject{}` instead (`BoxObject` has in-class default initializers that `memset` would silently corrupt).
- Comments are a mix of Japanese, Chinese, and English.
- Shaders: `shader.hlsl` (general 3D), `skyshader.hlsl` (sky dome). Compiled at runtime via `D3DCompileFromFile`.
