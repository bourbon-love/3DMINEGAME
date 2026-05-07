
#pragma once

//Bg.h

#include	"main.h"
#include	"renderer.h"

class BgObject
{
public:
	bool		Use;
	XMFLOAT3	position;
	XMFLOAT3	scale;
	XMFLOAT3	rotate;
	float		height;
	ID3D11ShaderResourceView* TexID;
};


void	InitBg();
void	UninitBg();
void	UpdateBg();
void	DrawBg();


