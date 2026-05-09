//Box.cpp

#include	"ball.h"
#include	"model.h"
#include	"keyboard.h"
#include	"Camera.h"
#include	"Box.h"
#include	"sprite.h"
#include	"mouse.h"
#include	"ParticlaEffect.h"
#include    "PixelCharacters.h"
#include "FrameWork/TextureManager.h"
static	ID3D11Buffer* g_VertexBuffer = NULL;//テスト用頂点バッファ
static	ID3D11Buffer* g_IndexBuffer = NULL;//テスト用インデックスバッファ//追加

static	ID3D11ShaderResourceView* g_Texture;	//テクスチャ変数
static  ID3D11ShaderResourceView* g_TexturePartical = GET_TEXTURE(PARTICLE);

//extern ID3D11ShaderResourceView* g_TexturePartical;   // 粒子纹理


BallObject	g_Ball;	//BALLオブジェクト
MODEL* BallModel;

static		LIGHT	Light;

//void	DrawBillboard();

void	BallRun();
void	BallIdle();
void	BallJump();
void	BallFall();


void	DrawBillboard()
{

	XMMATRIX	WorldMatrix = GetViewMatrix();
	WorldMatrix.r[3].m128_f32[0] = 0;
	WorldMatrix.r[3].m128_f32[1] = 0;
	WorldMatrix.r[3].m128_f32[2] = 0;
	WorldMatrix.r[3].m128_f32[3] = 1;
	WorldMatrix = XMMatrixTranspose(WorldMatrix);
	WorldMatrix.r[3].m128_f32[0] = g_Ball.position.x;
	WorldMatrix.r[3].m128_f32[1] = g_Ball.position.y + 0.5f;
	WorldMatrix.r[3].m128_f32[2] = g_Ball.position.z;
	WorldMatrix.r[3].m128_f32[3] = 1;
	SetWorldMatrix(WorldMatrix);

	//マテリアル設定
	MATERIAL	material;
	ZeroMemory(&material, sizeof(MATERIAL));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	LIGHT	Light;
	Light.Enable = false;//12/11
	SetLight(Light);

	//テクスチャセット
	ID3D11ShaderResourceView* g_TexturePartical = GET_TEXTURE(PARTICLE);
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_TexturePartical);//12/11


	XMFLOAT2	size = XMFLOAT2(0.5f, 0.5f);
	DrawBillboard(size, material.Diffuse);

}


BallObject* GetBall()
{
	return &g_Ball;
}

void	InitBall()
{
	TexMetadata		metadata;
	ScratchImage	image;
	//	LoadFromWICFile(L"asset\\texture\\texture.png", WIC_FLAGS_NONE, &metadata, image);
	LoadFromWICFile(L"asset\\texture\\texture.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(GetDevice(), image.GetImages(),
		image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);


	//3Dオブジェクト初期化
	g_Ball.Use = true;
	g_Ball.position = XMFLOAT3(BALL_START_POSX, BALL_START_POSY, START_POSZ);
	g_Ball.rotate = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Ball.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	BallModel = ModelLoad("asset\\model\\ball.fbx");

	//ライト構造体作成　//////////////////////////Lighting追加
	Light.Direction = XMFLOAT4(1.0f, -1.0f, 1.0f, 0.0f);
	Light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Light.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	Light.Enable = true;//12/11
	XMVECTOR	vec = XMLoadFloat4(&Light.Direction);
	vec = XMVector4Normalize(vec);
	XMStoreFloat4(&Light.Direction, vec);

	g_Ball.Radius = BALL_RADIUS;
	g_Ball.SizeMin.x = g_Ball.SizeMin.y = g_Ball.SizeMin.z = -BALL_RADIUS;
	g_Ball.SizeMax.x = g_Ball.SizeMax.y = g_Ball.SizeMax.z = BALL_RADIUS;


	g_Ball.Mode = BALL_IDLE;
	g_Ball.Acceleration = XMFLOAT3(0, -9.8f / 600.0f * 0.5f, 0);
	g_Ball.Velocity = XMFLOAT3(0, 0.0f, 0);
	g_Ball.characterType = CHARACTER_DOCTOR;//默认医生


	InitPixelCharacters();
	InitBallTactical();


	ParticleSystem::SetParticleTexture(g_TexturePartical);


}

void	UninitBall()
{

	ModelRelease(BallModel);

	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	if (g_IndexBuffer != NULL)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = NULL;
	}

	if (g_Texture != NULL)
	{
		g_Texture->Release();
		g_Texture = NULL;
	}

	//test.Unload();

}

// Ball.cpp  (放到 UpdateBall 之前)
static void TriggerBombFinalStage()
{
	BallObject* ball = GetBall();

	// ① 文字
	/*FloatingTextSystem::ShowText(
		XMFLOAT3(ball->bombPosition.x, ball->bombPosition.y + 1.0f, ball->bombPosition.z),
		"BOOM!!!", XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));*/

	// ② 粒子：主爆炸 + 次级火花 + 冲击波（与原来一致）
	ParticleSystem::CreateEffect(ball->bombPosition, 70,
		XMFLOAT4(1.0f, 0.3f, 0.0f, 1.0f), 0.18f, 0.25f, 1.2f);
	ParticleSystem::CreateEffect(ball->bombPosition, 40,
		XMFLOAT4(1.0f, 0.7f, 0.0f, 1.0f), 0.15f, 0.20f, 0.8f);
	ParticleSystem::CreateEffect(
		XMFLOAT3(ball->bombPosition.x, ball->bombPosition.y - 0.4f, ball->bombPosition.z),
		30, XMFLOAT4(0.5f, 0.5f, 0.5f, 0.7f), 0.25f, 0.15f, 0.5f);

	// ③ 小球弹回起始设置
	ball->isCountingDown = false;
	ball->isBouncing = true;
	ball->bounceTime = 0.0f;
	ball->isMoving = false;   // 确保不再受普通移动影响

}


void UpdateBall() {
	if (g_Ball.startBounceNextFrame) {
		g_Ball.startBounceNextFrame = false; // 只触发一次
		g_Ball.isBouncing = true;
		g_Ball.bounceTime = 0.0f;
		g_Ball.isMoving = false;
		// 此帧不做 UpdateBallMovement() —— 让渲染保持静止
		return;   // 直接回去渲染 “爆炸帧”
	}

	// 计算帧时间
	static float lastTime = 0;
	float currentTime = GetTickCount() * 0.001f;  // 转换为秒
	float deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	// 限制deltaTime，防止大跳跃
	deltaTime = std::min(deltaTime, 0.05f);

	// 更新粒子效果
	ParticleSystem::Update(deltaTime);

	// 更新浮动文本
	FloatingTextSystem::Update(deltaTime);

	// 如果炸弹正在倒计时，处理倒计时逻辑
	if (g_Ball.isCountingDown) {
		// 更新倒计时
		g_Ball.countdownTimer += deltaTime;

		// 每个阶段持续时间
		const float STAGE_DURATION = 0.6f;

		// 先检查我们现在应该处于哪个阶段
		int currentStage = (int)(g_Ball.countdownTimer / STAGE_DURATION) + 1;

		// 如果进入了新阶段且还没有超过最大阶段数
		if (currentStage > g_Ball.countdownStage && currentStage <= 4) {
			// 更新当前阶段
			g_Ball.countdownStage = currentStage;

			// 处理当前阶段
			if (g_Ball.countdownStage < 4) { // 阶段1-3: 显示"DI"
				// 显示"DI"文本，颜色逐渐变红
				float red = 1.0f;
				float green = 0.7f - (g_Ball.countdownStage - 1) * 0.2f;
		
				// 创建警告粒子效果，逐渐增强
				ParticleSystem::CreateEffect(
					XMFLOAT3(g_Ball.bombPosition.x, g_Ball.bombPosition.y + 0.2f, g_Ball.bombPosition.z),
					10 + (g_Ball.countdownStage - 1) * 5,
					XMFLOAT4(1.0f, green, 0.0f, 1.0f),
					0.05f + (g_Ball.countdownStage - 1) * 0.02f,
					0.1f + (g_Ball.countdownStage - 1) * 0.05f,
					0.5f
				);

			}
			else if (g_Ball.countdownStage == 4) { 
				TriggerBombFinalStage();
			}
		}

		// 在倒计时过程中不接受输入
		return;
	}

	// 如果球体正在反弹，更新反弹动画
	if (g_Ball.isBouncing) {
		UpdateBallMovement();
		return;
	}

	// 如果小球正在移动，则更新移动动画
	if (g_Ball.isMoving) {
		UpdateBallMovement();
		return;
	}

	// 检测按键输入和处理玩家移动
	bool upKey = Keyboard_IsKeyDownTrigger(KK_UP) || Keyboard_IsKeyDownTrigger(KK_W);
	bool downKey = Keyboard_IsKeyDownTrigger(KK_DOWN) || Keyboard_IsKeyDownTrigger(KK_S);
	bool leftKey = Keyboard_IsKeyDownTrigger(KK_LEFT) || Keyboard_IsKeyDownTrigger(KK_A);
	bool rightKey = Keyboard_IsKeyDownTrigger(KK_RIGHT) || Keyboard_IsKeyDownTrigger(KK_D);

	// 处理玩家输入
	if (!g_Ball.turnEnded && !g_Ball.isMoving && !g_Ball.isBouncing && !g_Ball.isCountingDown) {
		if (upKey) {
			OutputDebugStringA("按下上键，尝试向上移动\n");
			MoveBallToAdjacentGrid(0, -1);  // 向上（Z-）移动
		}
		else if (downKey) {
			OutputDebugStringA("按下下键，尝试向下移动\n");
			MoveBallToAdjacentGrid(0, 1);   // 向下（Z+）移动
		}
		else if (leftKey) {
			OutputDebugStringA("按下左键，尝试向左移动\n");
			MoveBallToAdjacentGrid(-1, 0);  // 向左（X-）移动
		}
		else if (rightKey) {
			OutputDebugStringA("按下右键，尝试向右移动\n");
			MoveBallToAdjacentGrid(1, 0);   // 向右（X+）移动
		}
		else if (Keyboard_IsKeyDownTrigger(KK_SPACE)) {
			OutputDebugStringA("按下空格键，结束回合\n");
			EndTurn();  // 结束回合
		}

		// 添加角色切换键
		if (Keyboard_IsKeyDownTrigger(KK_C)) {
			OutputDebugStringA("按下C键，切换角色\n");
			SwitchToNextCharacter();
		}
	}
}


void	DrawBall()
{
	//ライト構造体セット///////////////////////Lighting追加
	SetLight(Light);


	// 根据角色位置和当前动画状态设置变换矩阵
	//平行移動行列作成
	XMMATRIX	TranslationMatrix =
		XMMatrixTranslation(
			g_Ball.position.x,
			g_Ball.position.y,
			g_Ball.position.z
		);


	// 根据运动方向设置旋转角度
	float rotationY = 0.0f;
	if (g_Ball.isMoving) {
		// 通过目标位置和当前位置计算角度
		float dx = g_Ball.targetPos.x - g_Ball.previousGridPos.x;
		float dz = g_Ball.targetPos.z - g_Ball.previousGridPos.z;

		if (dx > 0) rotationY = 90.0f;
		else if (dx < 0) rotationY = -90.0f;
		else if (dz > 0) rotationY = 180.0f;
		else if (dz < 0) rotationY = 0.0f;
	}
	// 设置旋转矩阵
	XMMATRIX RotationMatrix = XMMatrixRotationY(XMConvertToRadians(rotationY));

	// 获取角色动画参数（上下移动和手臂摆动）
	float bobHeight = GetCharacterBobHeight(g_Ball.characterType);

	// 添加上下移动高度到Y坐标
	XMMATRIX BobMatrix = XMMatrixTranslation(0.0f, bobHeight, 0.0f);

	// 构建最终世界矩阵
	XMMATRIX WorldMatrix = BobMatrix * RotationMatrix * TranslationMatrix;
	SetWorldMatrix(WorldMatrix);

	// 使用像素角色绘制函数替代旧的球体绘制
	switch (g_Ball.characterType) {
	case CHARACTER_DOCTOR:
		DrawPixelCharacter(&g_Doctor, WorldMatrix);
		break;
	case CHARACTER_SOLDIER:
		DrawPixelCharacter(&g_Soldier, WorldMatrix);
		break;
	case CHARACTER_SCOUT:
		DrawPixelCharacter(&g_Scout, WorldMatrix);
		break;
	case CHARACTER_BOMB_TECH:
		DrawPixelCharacter(&g_BombTech, WorldMatrix);
		break;
	}

}

void	BallRun()
{
	g_Ball.Velocity.x += g_Ball.Acceleration.x;
	g_Ball.Velocity.y += g_Ball.Acceleration.y;
	g_Ball.Velocity.z += g_Ball.Acceleration.z;

	g_Ball.Velocity.x *= 0.98f;
	g_Ball.Velocity.z *= 0.98f;

	g_Ball.position.x += g_Ball.Velocity.x;
	g_Ball.position.y += g_Ball.Velocity.y;
	g_Ball.position.z += g_Ball.Velocity.z;


	////ボールの下の点
	int no = GroundCollision();
	if (no == -1)
	{	//地面が無かったら落ちる
		g_Ball.Mode = BALL_FALL;
	}
	else
	{
		g_Ball.Velocity.y = 0.0f;
	}
	//速度がほぼ０になったら
	float len = (g_Ball.Velocity.x * g_Ball.Velocity.x) + (g_Ball.Velocity.z * g_Ball.Velocity.z);
	if (len <= (BALL_VELOCITY * 0.002f))
	{
		g_Ball.Mode = BALL_IDLE;
		g_Ball.Velocity.x = 0.0f;
		g_Ball.Velocity.z = 0.0f;
	}

	//壁との当たり
	no = ObstacleCollision();



}



void	BallIdle()
{

	Mouse_State Mouse;
	Mouse_GetState(&Mouse);

	if (Mouse.leftButton || Mouse.rightButton)//Zキーでボールを打ち出す
		//	if (Keyboard_IsKeyDownTrigger(KK_Z))//Zで移動
	{
		//とりあえずカメラの向いているベクトル方向へ移動
		//そのうちキー操作で方向を決められるようにしましょう
		XMFLOAT3	vec = GetCameraForwardVec();
		g_Ball.Velocity.x = vec.x * BALL_VELOCITY;

		if (Mouse.leftButton)
		{
			g_Ball.Velocity.y = 0.25f;
			g_Ball.Mode = BALL_JUMP;
		}
		else 		if (Mouse.rightButton)
		{
			g_Ball.Velocity.x = vec.x * (BALL_VELOCITY * 3.5);
			g_Ball.Velocity.y = 0.0f;
			g_Ball.Mode = BALL_RUN;
		}

		g_Ball.Velocity.z = vec.z * BALL_VELOCITY;

	}

}

void	BallJump()//おまけ
{
	g_Ball.Velocity.x += g_Ball.Acceleration.x;
	g_Ball.Velocity.y += g_Ball.Acceleration.y;
	g_Ball.Velocity.z += g_Ball.Acceleration.z;

	g_Ball.position.x += g_Ball.Velocity.x;
	g_Ball.position.y += g_Ball.Velocity.y;
	g_Ball.position.z += g_Ball.Velocity.z;

	if (g_Ball.Velocity.y < 0.001f)
	{
		g_Ball.Velocity.y = 0.0f;
		g_Ball.Mode = BALL_FALL;
	}

	int no = GroundCollision();

	no = ObstacleCollision();


}
void	BallFall()
{
	g_Ball.Velocity.x += g_Ball.Acceleration.x;
	g_Ball.Velocity.y += g_Ball.Acceleration.y;
	g_Ball.Velocity.z += g_Ball.Acceleration.z;

	g_Ball.position.x += g_Ball.Velocity.x;
	g_Ball.position.y += g_Ball.Velocity.y;
	g_Ball.position.z += g_Ball.Velocity.z;

	int no = GroundCollision();

	if (no != -1)
	{
		g_Ball.Velocity.y *= -0.8f;
		if (g_Ball.Velocity.y < 0.01f)
		{
			//モード変更
//			g_Ball.Velocity.x = 0.0f;
			g_Ball.Velocity.y = 0.0f;
			//			g_Ball.Velocity.z = 0.0f;
			g_Ball.Mode = BALL_RUN;
		}
		//else
		//{
		//	////モード変更
		//	//g_Ball.Mode = BALL_RUN;
		//}
		return;
	}

	//壁との当たり
	no = ObstacleCollision();
	//if (no == -1)
	//{
	//	no = ObstacleGroundCollision();
	//	if (no != -1)
	//	{
	//		g_Ball.Velocity.y *= -0.8f;
	//		if (g_Ball.Velocity.y < 0.01f)
	//		{
	//			//モード変更
	//			g_Ball.Velocity.x = 0.0f;
	//			g_Ball.Velocity.y = 0.0f;
	//			g_Ball.Velocity.z = 0.0f;
	//			g_Ball.Mode = BALL_IDLE;
	//		}
	//		else
	//		{
	//			////モード変更
	//			g_Ball.Mode = BALL_JUMP;
	//		}

	//	}
	//}

	return;

}


// 初始化小球的战旗游戏属性
void InitBallTactical() {
	g_Ball.gridPos.x = 10;  // 起始格子X
	g_Ball.gridPos.z = 10;  // 起始格子Z
	g_Ball.targetPos = g_Ball.gridPos;
	g_Ball.movementPoints = 95;
	g_Ball.maxMovementPoints = 100;
	g_Ball.health = 100;
	g_Ball.maxHealth = 100;
	g_Ball.isMoving = false;
	g_Ball.moveProgress = 0.0f;
	g_Ball.turnEnded = false;
	g_Ball.collectedCoins = 0;  // 初始化金币计数器
	g_Ball.isBouncing = false;  // 初始化弹回标志
	g_Ball.bounceTime = 0.0f;   // 初始化弹回计时器

	// 初始化炸弹倒计时相关变量
	g_Ball.isCountingDown = false;
	g_Ball.countdownTimer = 0.0f;
	g_Ball.countdownStage = 0;
	g_Ball.bombPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// 将小球定位到网格位置
	g_Ball.position.x = g_Ball.gridPos.x * BOXSIZE_X + (BOXSIZE_X / 2);
	g_Ball.position.z = -g_Ball.gridPos.z * BOXSIZE_Z + (BOXSIZE_Z / 2);
	char debug[256];
	sprintf_s(debug, "初始化小球战术属性: 位置(%d,%d), 移动力:%d\n",
		g_Ball.gridPos.x, g_Ball.gridPos.z, g_Ball.movementPoints);
	OutputDebugStringA(debug);
}
void EndTurn() {
	g_Ball.turnEnded = true;

	// 回合结束时的逻辑
	// 例如：重置移动力、更新状态效果等

	// 下一回合开始
	g_Ball.movementPoints = g_Ball.maxMovementPoints;
	g_Ball.turnEnded = false;
}

// 修改 CheckObstacleInteractions 函数，完全重置炸弹计时参数
void CheckObstacleInteractions() {
	BallObject* ball = GetBall();
	BoxObject* obstacles = GetObstacle();

	// 如果当前正在反弹或倒计时，不进行交互检查
	if (ball->isBouncing || ball->isCountingDown) {
		return;
	}

	// 转换当前网格位置为世界坐标
	float worldX = ball->gridPos.x * BOXSIZE_X + (BOXSIZE_X / 2);
	float worldZ = -ball->gridPos.z * BOXSIZE_Z + (BOXSIZE_Z / 2);

	// 检查所有障碍物的碰撞
	for (int i = 0; i < MAPSIZE_X * MAPSIZE_Z; i++) {
		if (!obstacles[i].Use) continue;

		// 计算球体与障碍物中心的距离
		float dx = obstacles[i].position.x - worldX;
		float dz = obstacles[i].position.z - worldZ;
		float distanceSq = dx * dx + dz * dz;

		// 如果球体与障碍物在同一个网格
		if (distanceSq < 0.5f * 0.5f) {
			// 检查是否是金币
			if (obstacles[i].ObjectNo == COIN) {
				// [金币逻辑保持不变]
				ball->collectedCoins++;

				// 创建粒子效果
				ParticleSystem::CreateEffect(
					obstacles[i].position,
					30,
					XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f),
					0.1f,
					0.15f,
					1.5f
				);

				// 显示文本反馈 - 改为英文
				char coinText[32];
				sprintf_s(coinText, "Coin Collected! Total: %d", ball->collectedCoins);
				FloatingTextSystem::ShowText(ball->position, coinText, XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f));

				// 移除金币
				obstacles[i].Use = false;

				// 调试信息
				char debug[128];
				sprintf_s(debug, "收集到金币! 总数: %d\n", ball->collectedCoins);
				OutputDebugStringA(debug);

				continue;
			}

			// 检查是否是炸弹
			if (obstacles[i].ObjectNo == BOMB) {
				// 完全重置所有炸弹相关参数，确保干净的状态
				ball->isCountingDown = true;
				ball->countdownTimer = 0.0f;
				ball->countdownStage = 0;
				ball->bombPosition = obstacles[i].position;
				ball->isBouncing = false;
				ball->bounceTime = 0.0f;

				// 调试信息
				OutputDebugStringA("炸弹倒计时开始! 所有参数已重置\n");

				// 不再检查其他障碍物
				break;
			}
		}
	}
}
bool MoveBallToAdjacentGrid(int dx, int dz) {
	// Calculate target grid
	BallObject* ball = GetBall();
	int targetX = ball->gridPos.x + dx;
	int targetZ = ball->gridPos.z + dz;

	char debug[256];
	sprintf_s(debug, "Trying to move to grid (%d,%d), current position (%d,%d)\n",
		targetX, targetZ, ball->gridPos.x, ball->gridPos.z);
	OutputDebugStringA(debug);

	// Check if we can move to target grid
	if (IsGridWalkable(targetX, targetZ)) {
		int cost = GetGridMovementCost(targetX, targetZ);

		sprintf_s(debug, "Grid is walkable, movement cost: %d, current movement points: %d\n",
			cost, ball->movementPoints);
		OutputDebugStringA(debug);

		// Check movement points
		if (ball->movementPoints >= cost) {
			// Store previous position before moving (for bomb bounce-back)

			ball->previousGridPos.x = ball->gridPos.x;
			ball->previousGridPos.z = ball->gridPos.z;
			// Set target position
			ball->targetPos.x = targetX;
			ball->targetPos.z = targetZ;
			ball->isMoving = true;
			ball->moveProgress = 0.0f;

			// Consume movement points
			ball->movementPoints -= cost;

			sprintf_s(debug, "Movement successful! Remaining movement points: %d\n", ball->movementPoints);
			OutputDebugStringA(debug);
			return true;
		}
		else {
			OutputDebugStringA("Movement failed: Not enough movement points!\n");
		}
	}
	else {
		OutputDebugStringA("Movement failed: Grid not walkable!\n");
	}

	return false;
}

void UpdateBallMovement() {
	BallObject* ball = GetBall();

	// 处理炸弹反弹动画
	if (ball->isBouncing) {
	

		// 动画完成
		if (ball->bounceTime >= 1.0f) {
			// 重置反弹状态
			ball->isBouncing = false;
			ball->bounceTime = 0.0f;

			
			// 设置网格位置回到前一个位置
			ball->gridPos.x = ball->previousGridPos.x;
			ball->gridPos.z = ball->previousGridPos.z;
			ball->targetPos.x = ball->previousGridPos.x;
			ball->targetPos.z = ball->previousGridPos.z;

			// 重要：确保回到正确的地面高度
			const float GROUND_HEIGHT = BALL_RADIUS;
			ball->position.x = ball->gridPos.x * BOXSIZE_X + (BOXSIZE_X / 2);
			ball->position.y = GROUND_HEIGHT; // 严格设置为地面高度
			ball->position.z = -ball->gridPos.z * BOXSIZE_Z + (BOXSIZE_Z / 2);

			// 添加落地粒子效果
			ParticleSystem::CreateEffect(
				ball->position,
				15,
				XMFLOAT4(0.7f, 0.7f, 0.7f, 0.8f),
				0.04f,
				0.1f,
				0.6f
			);
			OutputDebugStringA("炸弹反弹完成，回到上一位置并固定在地面高度\n");
			return;
		}

		// 开始位置
		XMFLOAT3 startPos;
		startPos.x = ball->gridPos.x * BOXSIZE_X + (BOXSIZE_X / 2);
		startPos.y = BALL_RADIUS; // 使用标准地面高度
		startPos.z = -ball->gridPos.z * BOXSIZE_Z + (BOXSIZE_Z / 2);

		// 结束位置（前一个位置）
		XMFLOAT3 endPos;
		endPos.x = ball->previousGridPos.x * BOXSIZE_X + (BOXSIZE_X / 2);
		endPos.y = BALL_RADIUS; // 使用标准地面高度
		endPos.z = -ball->previousGridPos.z * BOXSIZE_Z + (BOXSIZE_Z / 2);

		// 使用弹跳效果插值
		float progress = ball->bounceTime;

		// 使用更加可控的弧形轨迹，限制最大高度
		float maxHeight = 0.7f; // 稍微增加最大弹跳高度，让效果更明显
		float heightFunction = sinf(progress * 3.14159f) * maxHeight; // 弧形高度

		// 计算当前位置
		ball->position.x = startPos.x + (endPos.x - startPos.x) * progress;
		ball->position.y = BALL_RADIUS + heightFunction; // 基础高度 + 弧形高度
		ball->position.z = startPos.z + (endPos.z - startPos.z) * progress;

		// 创建轨迹粒子 - 增加数量，让效果更明显
		if (rand() % 4 == 0) { // 从6改为4，增加粒子生成频率
			ParticleSystem::CreateEffect(
				ball->position,
				3,
				XMFLOAT4(1.0f, 0.5f, 0.0f, 0.8f), // 橙色
				0.02f,
				0.08f,
				0.3f
			);
		}
		// 例如，更新特定角色的摆动高度
		g_CharacterBobHeights[ball->characterType] = sinf(progress * 3.14159f) * 0.2f;

		// 动画进度 - 稍稍加快反弹速度
		ball->bounceTime += 0.025f;
		

		return;
	}

	// 常规移动动画逻辑
	if (ball->isMoving) {
		// 计算起始和目标世界坐标
		XMFLOAT3 startPos;
		startPos.x = ball->gridPos.x * BOXSIZE_X + (BOXSIZE_X / 2);
		startPos.y = BALL_RADIUS; // 确保正确的地面高度
		startPos.z = -ball->gridPos.z * BOXSIZE_Z + (BOXSIZE_Z / 2);

		XMFLOAT3 endPos;
		endPos.x = ball->targetPos.x * BOXSIZE_X + (BOXSIZE_X / 2);
		endPos.y = BALL_RADIUS; // 确保正确的地面高度
		endPos.z = -ball->targetPos.z * BOXSIZE_Z + (BOXSIZE_Z / 2);

		// 递增移动进度
		ball->moveProgress += 0.05f;

		if (ball->moveProgress >= 1.0f) {
			// 移动完成
			ball->moveProgress = 1.0f;
			ball->isMoving = false;
			ball->gridPos = ball->targetPos;

			// 确保正确位置和高度
			ball->position.x = endPos.x;
			ball->position.y = BALL_RADIUS; // 严格设置为地面高度
			ball->position.z = endPos.z;

			// 应用格子效果
			ApplyGridEffect(ball->gridPos.x, ball->gridPos.z);

			// 检查障碍物交互
			CheckObstacleInteractions();

			// 重置动画参数
			g_CharacterBobHeights[ball->characterType] = 0.0f;
			g_CharacterArmSwings[ball->characterType] = 0.0f;
		}
		else {
			// 根据进度插值计算当前位置
			float t = ball->moveProgress;

			// 可以添加轻微的弹跳效果，使移动更生动
			float bounce = 0.0f;
			if (t < 0.5f) {
				bounce = sinf(t * 3.14159f) * 0.1f; // 轻微的弹跳
			}
			// 使用正弦函数创建更自然的行走动画
			g_CharacterBobHeights[ball->characterType] = sinf(t * 2.0f * 3.14159f) * 0.1f;
			g_CharacterArmSwings[ball->characterType] = sinf(t * 2.0f * 3.14159f) * 0.3f;

			ball->position.x = startPos.x + (endPos.x - startPos.x) * t;
			ball->position.y = BALL_RADIUS + bounce; // 基础高度 + 轻微弹跳
			ball->position.z = startPos.z + (endPos.z - startPos.z) * t;
		}
	}
}

// 切换到下一个角色类型
void SwitchToNextCharacter()
{
	// 当前角色类型
	int currentType = (int)g_Ball.characterType;

	// 切换到下一个角色
	currentType = (currentType + 1) % CHARACTER_TYPE_COUNT;
	g_Ball.characterType = (CHARACTER_TYPE)currentType;

	// 输出调试信息
	char debug[128];
	sprintf_s(debug, "切换到角色类型: %d\n", currentType);
	OutputDebugStringA(debug);
}

// 获取当前角色类型
CHARACTER_TYPE GetCurrentCharacterType()
{
	return g_Ball.characterType;
}

// 设置当前角色类型
void SetCurrentCharacterType(CHARACTER_TYPE type)
{
	if (type >= 0 && type < CHARACTER_TYPE_COUNT) {
		g_Ball.characterType = type;
	}
}