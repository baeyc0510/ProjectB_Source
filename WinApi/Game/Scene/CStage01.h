#pragma once
#include "CMapScene.h"

class CStage01 : public CMapScene
{
public:
	CStage01();
	~CStage01() override;

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

