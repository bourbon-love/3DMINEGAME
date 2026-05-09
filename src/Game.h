#pragma once
#include "SceneManager.h"  
extern int g_FrameCount;

void InitGame();
void UninitGame();
void UpdateGame();
void DrawGame();

void	SetPause(bool);
bool	GetPause();
void UpdateDeltaTime();
void Draw3DScene(AppMode currentMode);
void Draw2DOverlay(AppMode currentMode);
void DrawStandardGameScene(AppMode currentMode);
void DrawGameOverScene();
void DrawEditModeOverlay(ID3D11ShaderResourceView* ptexture);
void DrawPauseModeOverlay(ID3D11ShaderResourceView* ptexture);
void DrawGameOverOverlay(ID3D11ShaderResourceView* ptexture);
void DrawStandardGameOverlay(ID3D11ShaderResourceView* ptexture, AppMode currentMode);
void DrawTraditionalMinimap(ID3D11ShaderResourceView* ptexture);
void UpdateSinglePlayerCoins();
void UpdateDebugControls();


void DebugMultiplayerBattle();