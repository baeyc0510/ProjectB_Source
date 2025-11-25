#include "pch.h"
#include "CSoundController.h"

CSoundController::CSoundController()
{
	soundOnce = nullptr;
	soundLoop = nullptr;
}

CSoundController::~CSoundController()
{
}

void CSoundController::Init()
{
}

void CSoundController::OnEnable()
{
}

void CSoundController::Update()
{
	if (INPUT->ButtonDown('U'))
	{
		SOUND->PlayOnce(soundOnce);
	}
	if (INPUT->ButtonDown('I'))
	{
		SOUND->PlayLoop(TEXT("soundLoop"), soundLoop);
	}
	if (INPUT->ButtonDown('O'))
	{
		SOUND->Stop(TEXT("soundLoop"));
	}
	if (INPUT->ButtonDown('P'))
	{
		SOUND->StopAll();
	}
	if (INPUT->ButtonDown('J'))
	{
		SOUND->Pause(TEXT("soundLoop"));
	}
	if (INPUT->ButtonDown('K'))
	{
		SOUND->Resume(TEXT("soundLoop"));
	}
}

void CSoundController::Render()
{
}

void CSoundController::OnDisable()
{
}

void CSoundController::Release()
{
}
