#pragma once
#include "CMapScene.h"

class CStage_Beginning : public CMapScene
{
public:
	CStage_Beginning();
	~CStage_Beginning() override;

public:
	void Init()		override;
	void Enter()	override;
	void Update()	override;
	void Render()	override;
	void Exit()		override;
	void Release()	override;
	
	void OnLoadMap() override;
private:
	class CPlayer* _player;
	class CStateSystem* _stateSystem;
};

