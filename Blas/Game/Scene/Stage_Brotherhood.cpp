#include "pch.h"
#include "Stage_Brotherhood.h"

#include "Game/Manager/SFXManager.h"

Stage_Brotherhood::Stage_Brotherhood()
{
}

Stage_Brotherhood::Stage_Brotherhood(const wstring& inMapPath) : SimpleStage(inMapPath)
{
}

void Stage_Brotherhood::Enter()
{
    SimpleStage::Enter();
    
    SFX->PlayBGM(SFXKey::BGM_Brotherhood, 0.5f);
    SFX->PlayAmbient(SFXKey::Ambient_Brotherhood, 0.5f);
}
