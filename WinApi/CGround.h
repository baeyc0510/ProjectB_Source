#pragma once
#include "CGameObject.h"

class CGround :public CGameObject
{
public:
	CGround();
	virtual ~CGround();

	// CGameObject을(를) 통해 상속됨
	void Render() override;
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void OnDisable() override;
	void Release() override;
};

