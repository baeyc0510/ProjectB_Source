#pragma once
#include "Game/Ability/Ability.h"

class Ability_LedgeClimb : public Ability
{
public:
	Ability_LedgeClimb();

	EStateTag GetRequiredTags() const override { return Tag_CanClimbLedge; }
	EStateTag GetBlockedTags() const override { return Tag_BlockMovement | Tag_Hit | Tag_Attacking; }
	EStateTag GetTagsToAdd() const override { return Tag_HangingLedge | Tag_AbilityAnimation | Tag_BlockMovement; }
	EStateTag GetCancelTags() const override { return Tag_Jumping | Tag_Sliding; }

	void OnActivate() override;
	void OnEnd() override;
	
private:
	// 입력 처리
	void OnUpHold();
	void OnJumpPressed();

	// 올라가기 완료
	void OnClimbOverFinished();

	// Ledge에서 떨어지기
	void DropFromLedge();

private:
	// 플레이어 캐시 (플레이어 전용 메서드 사용)
	class CPlayer* player = nullptr;

	// Ledge 정보 캐시
	float ledgeX = 0.f;
	float ledgeTop = 0.f;
	int ledgeDirection = 0;

	// 올라가기 중인지 여부
	bool bClimbingOver = false;
};
