#pragma once

// 

class CComboSystem : public Component<CGameObject>
{
public:
	CComboSystem();
	~CComboSystem();

	// Component을(를) 통해 상속됨
	void ComponentInit() override;
	void ComponentUpdate() override;
	void ComponentRender() override;
	void ComponentRelease() override;


};

