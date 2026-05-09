#include "Main.h"
#include "Manager.h"
#include "Renderer.h"
#include "Sprite.h"
#include "keyboard.h"
#include "Game.h"






static SCENE g_Scene = SCENE_NONE;

void InitManager()
{
	InitSprite();
	SetScene(SCENE_GAME);

}

void UninitManager()
{
	UninitSprite();
	SetScene(SCENE_NONE);

}

void UpdateManager()
{


	switch (g_Scene)
	{
	case SCENE_NONE:
		break;

	case SCENE_TITLE:
		break;

	case SCENE_GAME:
		UpdateGame();
		break;

	case SCENE_RESULT:
		break;

	default:
		break;
	}

}

void DrawManager()
{

	switch (g_Scene)
	{
	case SCENE_NONE:
		break;

	case SCENE_TITLE:
		break;

	case SCENE_GAME:
		DrawGame();
		break;

	case SCENE_RESULT:
		break;

	default:
		break;
	}

}



void SetScene(SCENE Scene)
{
	//現在のシーンを終了
	switch (g_Scene)
	{
	case SCENE_NONE:
		break;

	case SCENE_TITLE:
		break;

	case SCENE_GAME:
		UninitGame();
		break;

	case SCENE_RESULT:
		break;

	default:
		break;
	}


	//次のシーンを初期化
	g_Scene = Scene;
	switch (g_Scene)
	{
	case SCENE_NONE:
		break;

	case SCENE_TITLE:
		break;

	case SCENE_GAME:
		InitGame();
		break;

	case SCENE_RESULT:
		break;

	default:
		break;
	}


}