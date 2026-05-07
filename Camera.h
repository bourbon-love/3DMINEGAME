
#pragma once

//Camera.h
#include "main.h"
#include "renderer.h"
#include  "SceneManager.h"
//

#define PREVIEW_ANGLE_X (0.0f)
#define PREVIEW_ANGLE_Y (220.2f)
#define PREVIEW_OFFSET_Y (50.0f)
#define PREVIEW_OFFSET_Z (-45.83f)
#define PREVIEW_OFFSET_X (0.0f)

#define EDIT_ANGLE_X (-70.94f)
#define EDIT_ANGLE_Y (90.09f)
#define EDIT_OFFSET_Y (30.93f)
#define EDIT_OFFSET_Z (20.04f)
#define EDIT_OFFSET_X (0.0f)


#define CAMERA_SPEED	(0.05f)
#define LOOKAT_SPEED	(0.5f)
#define LOOKAT_ROTATE	(0.03f)
#define FOV_SPEED		(0.1f)
#define FOV_MAX		(30.0f)
#define FOV_MIN		(10.0f)

extern HWND g_hWnd;
template<typename T>
T Clamp(const T& value, const T& minVal, const T& maxVal)
{
	return (value < minVal) ? minVal : (value > maxVal) ? maxVal : value;
}

float Lerp(float a, float b, float t);

inline float VecLength(const XMFLOAT3& a, const XMFLOAT3& b)
{
	XMVECTOR va = XMLoadFloat3(&a);
	XMVECTOR vb = XMLoadFloat3(&b);
	return XMVectorGetX(XMVector3Length(va - vb));
}



class Camera
{
public:
	XMFLOAT3	Position;		//カメラの座標
	XMFLOAT3	AtPosition;		//カメラの注視点
	XMFLOAT3	UpVector;		//上方ベクトル
	float		fov;			//視野角
	float		fovbuffer;		//視野角
	float		nearclip;		//どこまで近くが見えるか
	float		farclip;		//どこまで遠くが見えるか

	XMFLOAT3	AtPositionOffset;	//注視点までの差分
	XMFLOAT3	AtPositionAngle;	//カメラの回転角度

	XMMATRIX	ViewMatrix;
	XMMATRIX	ProjectionMatrix;
	//XMMATRIX	RotationOld;

	XMFLOAT3	FwdVec;
	XMFLOAT3	RitVec;

	XMFLOAT3	Velocity;

};

void	InitCamera();
void	UninitCamera();
void	UpdateCamera();
void	DrawCamera();

XMFLOAT3	GetCameraRightVec();
XMFLOAT3	GetCameraForwardVec();
XMMATRIX	GetViewMatrix();
XMMATRIX	GetProjectMatrix();
void		SetCameraVelocity(XMFLOAT3);
Camera* GetCamera();

void InitCameraTransitionPath(AppMode fromMode, AppMode toMode);


bool WorldToScreen(const XMFLOAT3& worldPos, XMFLOAT2* screenPos);