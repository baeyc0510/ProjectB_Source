#pragma once
#include "Hazard.h"

class Hazard_Vine : public Hazard
{
public:
	Hazard_Vine();

protected:
	void Init() override;
	void OnSpawn() override;
};
