#pragma once
#pragma once

//Ball.h

#include	"main.h"
#include	"renderer.h"
#include	"Box.h"
#include    "ParticlaEffect.h"
#include	 "PixelCharacters.h"

#define		BALL_VELOCITY	(0.01f)
#define		BALL_RADIUS	(0.50f)
#define		BALL_START_POSX	(0.5f*3)
#define		BALL_START_POSY	(BALL_RADIUS*1)
#define		START_POSZ	(0.5f)



enum BALL_MODE
{
	BALL_IDLE = 0,
	BALL_RUN,
	BALL_JUMP,
	BALL_FALL,



};


class BallObject
{
public:
	bool		Use;
	XMFLOAT3	position;
	XMFLOAT3	scale;
	XMFLOAT3	rotate;

	XMFLOAT3	Acceleration;
	XMFLOAT3	Velocity;
	float		Radius;

	XMFLOAT3	SizeMin;
	XMFLOAT3	SizeMax;
	bool        isSlipping;

	// 特效相关属性
	std::vector<ParticleEffect> activeEffects;  // 活跃的粒子效果
	float effectTimer;       // 效果计时器

	ID3D11ShaderResourceView* TexID;

	//BoxObject*	HitObject;
//		float		Offset;
//		bool		Jump;
	BALL_MODE	Mode;

	// 战旗游戏新增属性
	GridPosition gridPos;      // 网格位置
	GridPosition targetPos;    // 目标网格位置
	int movementPoints;        // 回合可用移动力
	int maxMovementPoints;     // 最大移动力
	int health;                // 生命值
	int maxHealth;             // 最大生命值
	bool isMoving;             // 是否正在移动
	float moveProgress;        // 移动进度 (0-1)
	bool turnEnded;            // 回合是否结束

	int collectedCoins = 0;  // Count of collected coins
	bool isBouncing = false; // Flag for bomb bounce-back effect
	float bounceTime = 0.0f; 
	struct {
		int x; // 网格X坐标
		int z; // 网格Z坐标
	} previousGridPos; // 上一个网格位置，用于炸弹反弹效果


	// 炸弹倒计时相关
	bool isCountingDown;      // 是否正在倒计时
	float countdownTimer;     // 倒计时计时器
	int countdownStage;       // 倒计时阶段 (0-3: 滴滴滴，4: 轰)
	XMFLOAT3 bombPosition;    // 炸弹位置
	bool startBounceNextFrame = false;


	// 添加角色类型属性
	CHARACTER_TYPE characterType; // 当前使用的角色类型
};

void	InitBall();
void	UninitBall();
void	UpdateBall();
void	DrawBall();
void InitBallTactical();
bool MoveBallToAdjacentGrid(int dx, int dz);
void UpdateBallMovement();
void EndTurn();
void SwitchToNextCharacter();

BallObject* GetBall();
