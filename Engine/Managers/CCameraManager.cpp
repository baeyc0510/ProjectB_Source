#include "pch.h"
#include "CCameraManager.h"

CCameraManager::CCameraManager()
{
	lookAt			= Vec2(0, 0);
	targetPos		= Vec2(0, 0);
	targetObj		= nullptr;
	timeToTarget	= 0;

	fadeImage		= nullptr;
	targetBright	= 1;
	curBright		= 1;
	timeToBright	= 0;

	curZoom			= 1.0f;
	targetZoom		= 1.0f;
	timeToZoom		= 0;

	offset			= Vec2(0, 0);
	deadZone		= Vec2(0, 0);
	smoothSpeed		= 0;

	bounds			= Rect();
	hasBounds		= false;
}

CCameraManager::~CCameraManager()
{
}

void CCameraManager::Init()
{
	// fadeImage는 가상 해상도 크기로 생성
	Vec2 virtualSize = SINGLE(CEngine)->GetVirtualSize();
	fadeImage = new CImage();
	fadeImage->Create((UINT)virtualSize.x, (UINT)virtualSize.y);
	PatBlt(fadeImage->GetImageDC(), 0, 0, fadeImage->GetBmpWidth(), fadeImage->GetBmpHeight(), BLACKNESS);
}

void CCameraManager::Update()
{
	// 추적할 게임오브젝트가 있을 경우
	if (nullptr != targetObj)
	{
		if (targetObj->IsReservedDelete())
		{
			// 추적할 게임오브젝트가 삭제예정인 경우 추적 해제
			targetObj = nullptr;
		}
		else
		{
			// 타겟의 실제 위치 (오프셋 적용)
			Vec2 targetWorldPos = targetObj->GetPos() + offset;

			// 데드존 적용: 타겟이 데드존 밖으로 나간 만큼만 카메라 이동
			Vec2 diff = targetWorldPos - lookAt;
			float halfDeadX = deadZone.x * 0.5f;
			float halfDeadY = deadZone.y * 0.5f;

			// X축 데드존 체크
			if (diff.x > halfDeadX)
				targetPos.x = lookAt.x + (diff.x - halfDeadX);
			else if (diff.x < -halfDeadX)
				targetPos.x = lookAt.x + (diff.x + halfDeadX);
			else
				targetPos.x = lookAt.x;

			// Y축 데드존 체크
			if (diff.y > halfDeadY)
				targetPos.y = lookAt.y + (diff.y - halfDeadY);
			else if (diff.y < -halfDeadY)
				targetPos.y = lookAt.y + (diff.y + halfDeadY);
			else
				targetPos.y = lookAt.y;
		}
	}

	MoveToTarget();
	BrightToTarget();
	// ZoomToTarget(); // 줌 기능 비활성화 (나중에 제대로 구현 필요)

	// 바운딩 영역 클램핑
	if (hasBounds)
	{
		Vec2 virtualSize = SINGLE(CEngine)->GetVirtualSize();
		Vec2 halfScreen = virtualSize * 0.5f;

		// 카메라 lookAt이 바운딩 영역 내에 있도록 클램핑
		// lookAt은 화면 중앙이 보는 월드 좌표
		float minX = bounds.x + halfScreen.x;
		float maxX = bounds.x + bounds.w - halfScreen.x;
		float minY = bounds.y + halfScreen.y;
		float maxY = bounds.y + bounds.h - halfScreen.y;

		// 맵이 화면보다 작으면 중앙에 고정
		if (minX > maxX) lookAt.x = bounds.x + bounds.w * 0.5f;
		else lookAt.x = max(minX, min(maxX, lookAt.x));

		if (minY > maxY) lookAt.y = bounds.y + bounds.h * 0.5f;
		else lookAt.y = max(minY, min(maxY, lookAt.y));
	}
}

void CCameraManager::Render()
{
	if (curBright >= 1)
		return;

	// 가상 해상도 기준으로 페이드 이미지 렌더링
	Vec2 virtualSize = SINGLE(CEngine)->GetVirtualSize();
	RENDER->BlendImage(
		fadeImage,
		0, 0,
		virtualSize.x,
		virtualSize.y,
		0, 0,
		(float)(fadeImage->GetBmpWidth()),
		(float)(fadeImage->GetBmpHeight()),
		(1 - curBright)
	);
}

void CCameraManager::Release()
{
	delete fadeImage;
}

Vec2 CCameraManager::WorldToScreenPoint(Vec2 worldPoint)
{
	// 가상 해상도 기준으로 좌표 변환
	Vec2 virtualSize = SINGLE(CEngine)->GetVirtualSize();
	Vec2 center = virtualSize * 0.5f;

	// 카메라 오프셋을 한 번만 반올림 - 모든 오브젝트가 동시에 이동하도록
	float offsetX = floorf(lookAt.x - center.x + 0.5f);
	float offsetY = floorf(lookAt.y - center.y + 0.5f);

	Vec2 screenPos;
	screenPos.x = worldPoint.x - offsetX;
	screenPos.y = worldPoint.y - offsetY;
	return screenPos;
}

Vec2 CCameraManager::ScreenToWorldPoint(Vec2 screenPoint)
{
	// 가상 해상도 기준으로 좌표 변환
	Vec2 virtualSize = SINGLE(CEngine)->GetVirtualSize();
	Vec2 center = virtualSize * 0.5f;
	return screenPoint + (lookAt - center);
}

void CCameraManager::Scroll(Vec2 dir, float velocity)
{
	// 스크롤의 방향 크기가 없는 경우 진행하지 않음
	if (dir.Magnitude() == 0)
		return;

	// 스크롤의 목표 위치는 현재위치에서 스크롤 방향으로 떨어진 위치
	targetPos = lookAt;
	targetPos += dir.Normalized() * velocity * DT;
	timeToTarget = 0;	// 스크롤은 시간차를 두지 않은 즉각 이동
}

void CCameraManager::FadeIn(float duration)
{
	targetBright = 1;
	timeToBright = duration;
}

void CCameraManager::FadeOut(float duration)
{
	targetBright = 0;
	timeToBright = duration;
}

void CCameraManager::SetTargetPos(const Vec2& targetPos, float timeToTarget)
{
	this->targetPos = targetPos;
	this->timeToTarget = timeToTarget;
}

void CCameraManager::SetTargetObj(CGameObject* targetObj)
{
	this->targetObj = targetObj;

	// 타겟 설정 시 즉시 카메라를 타겟 위치로 이동 (오프셋 적용)
	if (targetObj != nullptr)
	{
		lookAt = targetObj->GetPos() + offset;
		targetPos = lookAt;
	}
}

void CCameraManager::MoveToTarget()
{
	// timeToTarget이 있으면 시간 기반 이동 (연출용)
	if (timeToTarget > 0)
	{
		timeToTarget -= DT;
		lookAt += (targetPos - lookAt) / timeToTarget * DT;
		return;
	}

	// smoothSpeed가 0이면 즉시 이동
	if (smoothSpeed <= 0)
	{
		lookAt = targetPos;
		return;
	}

	// 픽셀 단위 부드러운 이동 (lookAt은 항상 정수 유지)
	Vec2 diff = targetPos - lookAt;

	// X축
	if (abs(diff.x) > 0.5f)
	{
		float moveX = diff.x * smoothSpeed * DT;
		// 최소 1픽셀 이동 보장
		if (abs(moveX) < 1.f) moveX = (diff.x > 0) ? 1.f : -1.f;
		// 오버슈트 방지
		if (abs(moveX) > abs(diff.x)) moveX = diff.x;
		lookAt.x = floorf(lookAt.x + moveX + 0.5f);
	}

	// Y축
	if (abs(diff.y) > 0.5f)
	{
		float moveY = diff.y * smoothSpeed * DT;
		if (abs(moveY) < 1.f) moveY = (diff.y > 0) ? 1.f : -1.f;
		if (abs(moveY) > abs(diff.y)) moveY = diff.y;
		lookAt.y = floorf(lookAt.y + moveY + 0.5f);
	}
}

void CCameraManager::BrightToTarget()
{
	timeToBright -= DT;

	if (timeToBright <= 0)
	{
		// 목표위치까지 남은 시간이 없을 경우 목적지로 현재위치 고정
		curBright = targetBright;
	}
	else
	{
		// 목표위치까지 남은 시간이 있을 경우
		// 목적지까지 남은시간만큼의 속도로 이동
		// 이동거리 = 속력 * 시간
		// 속력 = (도착지 - 출발지) / 소요시간
		// 시간 = 프레임단위시간
		curBright += (targetBright - curBright) / timeToBright * DT;
		if		(curBright > 1) curBright = 1;
		else if (curBright < 0) curBright = 0;
	}
}

void CCameraManager::SetZoom(float zoom, float duration)
{
	targetZoom = zoom;
	timeToZoom = duration;

	// duration이 0이면 즉시 적용
	if (duration <= 0)
		curZoom = zoom;
}

void CCameraManager::ZoomToTarget()
{
	timeToZoom -= DT;

	if (timeToZoom <= 0)
	{
		curZoom = targetZoom;
	}
	else
	{
		// 부드러운 줌 전환
		curZoom += (targetZoom - curZoom) / timeToZoom * DT;
	}
}
