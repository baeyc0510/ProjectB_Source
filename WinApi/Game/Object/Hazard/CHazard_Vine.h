#pragma once
#include "CHazard.h"

class CHazard_Vine : public CHazard
{
public:
	CHazard_Vine();

protected:
	void Init() override;
	void OnSpawn() override;
	void OnExpire() override;
};
