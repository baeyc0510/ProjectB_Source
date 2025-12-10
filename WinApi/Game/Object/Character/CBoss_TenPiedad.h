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
	
	void OnDieStart() override;
	void OnDieComplete() override;
	
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
	// 보스 설정 상수
	struct Config
	{
		struct Encounter
		{
			static constexpr float Range = 400.f;
		};

		struct Attack
		{
			static constexpr float SlashRange = 240.f;
			static constexpr float StompRange = 200.f;
			static constexpr float SpitMinRange = 220.f;
			static constexpr float GroundSmashRange = 100.f;
		};

		struct Chase
		{
			static constexpr float Speed = 100.0f;
			static constexpr float Range = 400.0f;
			static constexpr float StopRange = 20.0f;
		};
	};

	// 추격 상태
	bool bIsChasing = false;
	bool bIsTurningAround = false;
	
	CImage* nameImg = nullptr;
};
