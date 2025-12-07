#pragma once
#include "CBoss.h"

class CBoss_TenPiedad : public CBoss
{
public:
	CBoss_TenPiedad();

protected:
	void Init() override;
	void OnAppearanceComplete() override;
	void UpdateBossAnimation() override;

private:
	void RegisterAnimations();
	void RegisterAbilities();
	void ConfigureAI();

	// 공격 범위 상수
	static constexpr float SLASH_RANGE = 80.f;
	static constexpr float STOMP_RANGE = 60.f;
	static constexpr float SPIT_MIN_RANGE = 150.f;
	static constexpr float GROUND_SMASH_RANGE = 100.f;
};
