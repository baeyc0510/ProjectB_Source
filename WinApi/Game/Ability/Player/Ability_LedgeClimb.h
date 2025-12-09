#pragma once
#include "Game/Ability/Ability.h"

class Ability_LedgeClimb : public Ability
{
public:
	Ability_LedgeClimb();

	/*~ Ability Interface ~*/
	EStateTag GetRequiredTags() const override { return Tag_CanClimbLedge; }
	EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_BlockMovement | Tag_Hit | Tag_Attacking; }
	EStateTag GetTagsToAdd() const override { return Tag_HangingLedge | Tag_AbilityAnimation | Tag_BlockMovement; }
	EStateTag GetCancelTags() const override { return Tag_Jumping | Tag_Sliding; }

	void OnActivate() override;
	void OnEnd() override;

private:
	void OnUpHold();
	void OnJumpPressed();
	void OnClimbOverFinished();
	void DropFromLedge();

private:
	class CPlayer* player = nullptr;
	float ledgeX = 0.f;
	float ledgeTop = 0.f;
	int ledgeDirection = 0;
	bool bClimbingOver = false;
};
