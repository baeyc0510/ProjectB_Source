#include "pch.h"
#include "LadderCollider.h"
#include "Game/Enum.h"
#include "Game/Object/Character/Player.h"
#include "Game/Component/StateSystem.h"

void LadderCollider::SetBoxCollider(const Vec2& center, const Vec2& size)
{
	WorldCollider::SetBoxCollider(center, size, ELayer::Ladder);
	ladderX = center.x;
	ladderTopY = center.y - size.y * 0.5f;
	ladderBottomY = center.y + size.y * 0.5f;
}

void LadderCollider::OnCollisionEnter(Collider* other)
{
	if (other->GetLayer() != (UINT)ELayer::Player)
		return;

	if (Player* player = dynamic_cast<Player*>(other->GetOwner()))
	{
		player->SetLadderInfo(ladderX, ladderTopY, ladderBottomY);
		player->GetStateSystem()->AddTagUnique(Tag_CanClimb);
	}
}

void LadderCollider::OnCollisionExit(Collider* other)
{
	if (other->GetLayer() != (UINT)ELayer::Player)
		return;
	
	if (Player* player = dynamic_cast<Player*>(other->GetOwner()))
	{
		player->GetStateSystem()->RemoveTag(Tag_CanClimb);
	}
}
