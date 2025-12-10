#pragma once
class ImageResource;
class GameObject;

// 카메라 흔들림 파라미터
struct FShakeParams
{
	float duration = 0.3f;		// 지속 시간
	float intensity = 5.f;		// 최대 흔들림 (픽셀)
	float frequency = 20.f;		// 초당 진동 횟수
	float decay = 1.f;			// 감쇠율 (0=감쇠없음, 1=선형감쇠)
	bool bRandomOffset = true;	// 랜덤 오프셋 사용 여부
};

// Shake 프리셋
namespace ShakePreset
{
	constexpr FShakeParams Light  = { 0.15f, 2.f,  30.f, 1.f, true };
	constexpr FShakeParams Medium = { 0.25f, 5.f,  25.f, 1.f, true };
	constexpr FShakeParams Heavy  = { 0.4f,  10.f, 20.f, 0.8f, true };
	constexpr FShakeParams Boss   = { 0.6f,  15.f, 15.f, 0.6f, true };
}

class CameraManager : public SingleTon<CameraManager>
{
	friend SingleTon<CameraManager>;
private:
	CameraManager();
	virtual ~CameraManager();

public:
	void				Init();
	void				Update();
	void				Render();
	void				Release();

	Vec2				WorldToScreenPoint(Vec2 worldPoint);	// 게임위치->화면위치
	Vec2				ScreenToWorldPoint(Vec2 screenPoint);	// 화면위치->게임위치

	void				Scroll(Vec2 dir, float velocity);

	void				FadeIn(float duration);	// 씬 진입 효과
	void				FadeOut(float duration);	// 씬 탈출 효과

	void				SetTargetPos(const Vec2& targetPos, float timeToTarget = 0);
	void				SetTargetObj(GameObject* targetObj);

	// 줌 기능 (연출용)
	void				SetZoom(float zoom, float duration = 0);
	float				GetZoom() const						{ return curZoom; }

	// 화면 흔들림
	void				Shake(const FShakeParams& params);
	void				Shake(float intensity, float duration);	// 간편 버전
	void				StopShake();
	bool				IsShaking() const					{ return shakeRemaining > 0; }

	// 오프셋 (타겟 기준 카메라 위치 조정)
	void				SetOffset(const Vec2& offset)		{ this->offset = offset; }
	const Vec2&			GetOffset() const					{ return offset; }

	// 데드존 (타겟이 이 범위 안에 있으면 카메라가 따라가지 않음)
	void				SetDeadZone(const Vec2& size)		{ deadZone = size; }
	const Vec2&			GetDeadZone() const					{ return deadZone; }

	// 부드러운 따라가기 (0 = 즉시, 값이 클수록 부드럽게)
	void				SetSmoothSpeed(float speed)			{ smoothSpeed = speed; }

	// 카메라 이동 제한 영역 (월드 좌표)
	void				SetBounds(const Rect& bounds)		{ this->bounds = bounds; hasBounds = true; }
	void				ClearBounds()						{ hasBounds = false; }
	const Rect&			GetBounds() const					{ return bounds; }

	Vec2				GetLookAt() const					{ return lookAt + shakeOffset; }
	const Vec2&			GetTargetPos()						{ return targetPos; }
	const GameObject*	GetTargetObj()						{ return targetObj; }

private:
	void				MoveToTarget();
	void				BrightToTarget();
	void				ZoomToTarget();
	void				UpdateShake();

	// 목표 오브젝트를 지정할 경우 목표 위치는 목표 오브젝트의 위치로 지정됨
	Vec2				lookAt;				// 카메라가 보고있는 위치
	Vec2				targetPos;			// 카메라의 목표 위치
	GameObject*		targetObj;			// 카메라의 목표 오브젝트
	float				timeToTarget;		// 목표까지 남은 시간

	ImageResource*				fadeImage;			// 전환 효과용 이미지
	float				targetBright;		// 카메라의 목표 밝기
	float				curBright;			// 카메라의 현재 밝기
	float				timeToBright;		// 카메라의 밝기 변화 남은시간

	// 줌 (연출용) - 1.0 = 기본, >1.0 = 확대, <1.0 = 축소
	float				curZoom;			// 현재 줌 레벨
	float				targetZoom;			// 목표 줌 레벨
	float				timeToZoom;			// 줌 변화 남은시간

	// 오프셋 (타겟 기준 카메라 위치 조정, 예: (0, 50) = 타겟이 화면 위쪽에 위치)
	Vec2				offset;

	// 데드존 (타겟이 이 범위 안에 있으면 카메라가 따라가지 않음)
	Vec2				deadZone;

	// 부드러운 따라가기 속도 (0 = 즉시, 값이 클수록 부드럽게)
	float				smoothSpeed;

	// 카메라 이동 제한 영역
	Rect				bounds;
	bool				hasBounds;

	// 화면 흔들림
	FShakeParams		shakeParams;
	float				shakeRemaining;		// 남은 흔들림 시간
	float				shakeElapsed;		// 경과 시간
	Vec2				shakeOffset;		// 현재 흔들림 오프셋
	float				shakeRandomSeed;	// 랜덤 시드
};

#define CAMERA	CameraManager::GetInstance()