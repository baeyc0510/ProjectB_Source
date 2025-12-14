#pragma once
#include "MapScene.h"

class Stage_Beginning : public MapScene
{
public:
	Stage_Beginning();
	~Stage_Beginning() override;

public:
	void Init()		override;
	void Enter()	override;
	void Update()	override;
	void Render()	override;
	void Exit()		override;
	void Release()	override;
	
	void OnLoadMap() override;
private:
	class Player* _player;
	class StateSystem* _stateSystem;
};

