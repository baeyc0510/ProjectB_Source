#include "pch.h"
#include "Ability_BossAppear.h"

#include "Game/AnimKey.h"
#include "Game/Object/Character/CBoss.h"

void Ability_BossAppear::OnActivate()
{
    Ability::OnActivate();
    
    GetAnimator()->Play(AnimKey::BossAppear,true, BIND(this,EndAbility));
}

void Ability_BossAppear::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
    
    CBoss* boss = dynamic_cast<CBoss*>(GetOwner());
    if (!boss)
    {
        return;
    }
    
    boss->OnAppearanceComplete();
}
