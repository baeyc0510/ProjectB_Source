#include "pch.h"
#include "CLadder.h"
#include "Game/Enum.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Component/CStateSystem.h"

void CLadder::SetBoxCollider(const Vec2& center, const Vec2& size)
{
	CWorldCollider::SetBoxCollider(center, size, ELayer::Ladder);
	ladderX = center.x;
	ladderTopY = center.y - size.y * 0.5f;
	ladderBottomY = center.y + size.y * 0.5f;
}

void CLadder::OnCollisionEnter(CCollider* other)
{
	if (other->GetLayer() != ELayer::Player)
		return;
	
	if (CPlayer* player = dynamic_cast<CPlayer*>(other->GetOwner()))
	{
		player->SetLadderInfo(ladderX, ladderTopY, ladderBottomY);
		player->GetStateSystem()->AddTagUnique(Tag_CanClimb);
	}
}

void CLadder::OnCollisionExit(CCollider* other)
{
	if (other->GetLayer() != ELayer::Player)
		return;
	
	if (CPlayer* player = dynamic_cast<CPlayer*>(other->GetOwner()))
	{
		player->GetStateSystem()->RemoveTag(Tag_CanClimb);
	}
}
