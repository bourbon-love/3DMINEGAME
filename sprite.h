/*==============================================================================

  sprite.h]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#pragma once


#include "main.h"
#include "renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitSprite(void);
void UninitSprite(void);
//スプライト表示　行列使用版
void DrawSprite(XMFLOAT2 size, XMFLOAT4 color);
void DrawBillboard(XMFLOAT2 size, XMFLOAT4 color);
void DrawSpriteLeftTop(
	XMFLOAT2 position, XMFLOAT2 size, XMFLOAT2 texCoordMin, XMFLOAT2 texCoordMax, XMFLOAT4 color, float angle);

// 添加一个支持自定义UV的Sprite绘制函数
void DrawSpriteUV(XMFLOAT2 size, XMFLOAT4 color, XMFLOAT2 uvMin, XMFLOAT2 uvMax);
