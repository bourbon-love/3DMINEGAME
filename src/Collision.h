#pragma once
//Collision.h

#include	"main.h"
#include	"renderer.h"
#include	"Ball.h"
#include	"Box.h"

bool		CollisionSphere(BallObject*, BoxObject*);
float		CollisionSphereBox(BallObject*, BoxObject*, XMFLOAT3*);
bool		CollisionBoxBox(BallObject*, BoxObject*);
bool		CollisionBoxPoint(XMFLOAT3, BoxObject*);




