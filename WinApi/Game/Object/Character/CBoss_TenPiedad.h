#pragma once
#include "CBoss.h"

class CBoss_TenPiedad : public CBoss
{
public:
	CBoss_TenPiedad();

protected:
	void Init() override;
	void OnAppearanceComplete() override;
	void UpdateBossAI() override;
	void UpdateBossAnimation() override;
	bool CheckEncounterPlayer() override;
	void OnDamage(CGameObject* source, const CombatContext& context) override;
	
private:
	void RegisterAnimations();
	void RegisterAbilities();
	void ConfigureAI();
	void UpdateChaseMovement();

	// 방향 전환
	bool NeedsTurnaround() const;	// 플레이어가 뒤에 있는지
	void StartTurnaround();			// 턴어라운드 애니메이션 시작
	void OnTurnaroundComplete();	// 턴어라운드 완료 콜백

private:
	// 공격 범위 상수
	static constexpr float ENCOUNTER_RANGE = 400.f;
	static constexpr float SLASH_RANGE = 240.f;
	static constexpr float STOMP_RANGE = 200.f;
	static constexpr float SPIT_MIN_RANGE = 220.f;
	static constexpr float GROUND_SMASH_RANGE = 100.f;

	// 추격 설정 상수
	static constexpr float CHASE_SPEED = 100.0f;
	static constexpr float CHASE_RANGE = 400.0f;	// 추격 시작 거리
	static constexpr float STOP_RANGE = 20.0f;		// 정지 거리 (근접 공격 범위)
	
	// 추격 상태
	bool bIsChasing = false;
	bool bIsTurningAround = false;
	
	CImage* nameImg = nullptr;
};
