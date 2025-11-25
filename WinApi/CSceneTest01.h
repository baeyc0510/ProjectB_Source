#pragma once
#include "CScene.h"

class CSceneTest01 : public CScene
{
public:
	CSceneTest01();
	virtual ~CSceneTest01();

public:
	void Init()		override;
	void Enter()	override;
	void Update()	override;
	void Render()	override;
	void Exit()		override;
	void Release()	override;
};

