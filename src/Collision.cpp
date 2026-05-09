
#include	"Collision.h"

//Collision.cpp


//球対球当たり判定
// true:当たり
bool		CollisionSphere(BallObject* pball, BoxObject* pbox)
{
	XMFLOAT3	vec;
	vec.x = pbox->position.x - pball->position.x;
	vec.y = pbox->position.y - pball->position.y;
	vec.z = pbox->position.z - pball->position.z;

	float	dst = (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
	float	rad = pbox->Radius + pball->Radius;
	rad *= rad;

	return (rad > dst);

}

float		CollisionSphereBox(BallObject* pball, BoxObject* pbox, XMFLOAT3 *cpoint)
{
	XMFLOAT3	boxSizeMax = XMFLOAT3(pbox->position.x + pbox->SizeMax.x, pbox->position.y + pbox->SizeMax.y, pbox->position.z + pbox->SizeMax.z);
	XMFLOAT3	boxSizeMin = XMFLOAT3(pbox->position.x + pbox->SizeMin.x, pbox->position.y + pbox->SizeMin.y, pbox->position.z + pbox->SizeMin.z);;

	XMFLOAT3	point;
	point.x = fmaxf(boxSizeMin.x, fminf(pball->position.x, boxSizeMax.x));
	point.y = fmaxf(boxSizeMin.y, fminf(pball->position.y, boxSizeMax.y));
	point.z = fmaxf(boxSizeMin.z, fminf(pball->position.z, boxSizeMax.z));

	float	dst =
		(
			(point.x - pball->position.x) * (point.x - pball->position.x) +
			(point.y - pball->position.y) * (point.y - pball->position.y) +
			(point.z - pball->position.z) * (point.z - pball->position.z)
		);
	float radius = pball->Radius * pball->Radius;

	if (dst > radius)
	{
		return 0;
	}
	*cpoint = point;
	return (radius - dst);
//	return (dst < radius);

}

bool		CollisionBoxBox(BallObject* pball, BoxObject* pbox)
{
	XMFLOAT3	ballSizeMax = 
		XMFLOAT3(pball->position.x + pball->SizeMax.x, pball->position.y + pball->SizeMax.y, pball->position.z + pball->SizeMax.z);
	XMFLOAT3	ballSizeMin =
		XMFLOAT3(pball->position.x + pball->SizeMin.x, pball->position.y + pball->SizeMin.y, pball->position.z + pball->SizeMin.z);
	XMFLOAT3	boxSizeMax = 
		XMFLOAT3(pbox->position.x + pbox->SizeMax.x, pbox->position.y + pbox->SizeMax.y, pbox->position.z + pbox->SizeMax.z);
	XMFLOAT3	boxSizeMin = 
		XMFLOAT3(pbox->position.x + pbox->SizeMin.x, pbox->position.y + pbox->SizeMin.y, pbox->position.z + pbox->SizeMin.z);

	return
		(
			(ballSizeMin.x <= boxSizeMax.x) &&
			(ballSizeMax.x >= boxSizeMin.x) &&
			(ballSizeMin.y <= boxSizeMax.y) &&
			(ballSizeMax.y >= boxSizeMin.y) &&
			(ballSizeMin.z <= boxSizeMax.z) &&
			(ballSizeMax.z >= boxSizeMin.z)
		);

}

bool		CollisionBoxPoint(XMFLOAT3 point, BoxObject* box)
{

	//XMFLOAT3	point = XMFLOAT3(ball->position.x, ball->position.y, ball->position.z);

	//背景
	XMFLOAT3	LeftTop;
	XMFLOAT3	RightBottom;

	//箱の左上奥座標
	LeftTop.x = box->position.x + box->SizeMin.x;
	LeftTop.y = box->position.y + box->SizeMax.y;
	LeftTop.z = box->position.z + box->SizeMax.z;
	
	//箱の右下手前座標
	RightBottom.x = box->position.x + box->SizeMax.x;
	RightBottom.y = box->position.y + box->SizeMin.y;
	RightBottom.z = box->position.z + box->SizeMin.z;

	//点が箱の左右の範囲内か？
	if ((point.x >= LeftTop.x) && (point.x <= RightBottom.x))
	{
		//点が箱の前後の範囲内か？
		if ((point.z <= LeftTop.z) && (point.z >= RightBottom.z))
		{
			//点が箱の上下の範囲内か？
			if ((point.y >= RightBottom.y) && (point.y <= LeftTop.y))
			{
				return	true;
			}
		}
	}
	return	false;
}


