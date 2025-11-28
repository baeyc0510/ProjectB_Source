#pragma once
#include "Game/Enum.h"

class CStateSystem : public Component<CGameObject>
{
public:
	CStateSystem();
	virtual ~CStateSystem();

	//========================================
	// 태그 관리
	//========================================
	void AddTag(StateTag tag);
	void AddTagUnique(StateTag tag);
	void RemoveTag(StateTag tag);
	void ClearTags();

	//========================================
	// 태그 검사
	//========================================
	bool HasTag(StateTag tag) const;
	bool HasAllTags(StateTag tags) const;
	bool HasAnyTag(StateTag tags) const;
	StateTag GetTags() const { return currentTags; }

	//========================================
	// 디버그
	//========================================
	wstring GetStateTagString() const;

private:
	void ComponentInit() override;
	void ComponentOnEnable() override {}
	void ComponentUpdate() override {}
	void ComponentRender() override {}
	void ComponentOnDisable() override {}
	void ComponentRelease() override {}

public:
	// 상태 변경 이벤트 (oldTags, newTags)
	MulticastDelegate<StateTag, StateTag> OnStateChanged;

private:
	StateTag currentTags;
	// tag stack counts
	map<StateTag, int> tagCountMap;
};
