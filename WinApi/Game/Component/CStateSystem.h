#pragma once
#include "Game/Enum.h"

class CStateSystem : public Component<CGameObject>
{
public:
	CStateSystem();
	virtual ~CStateSystem();

	/*~ CStateSystem Interface ~*/
	void AddTag(EStateTag tag);
	void AddTagUnique(EStateTag tag);
	void RemoveTag(EStateTag tag);
	void RemoveTagAll(EStateTag tag);  // 해당 태그의 모든 스택 제거
	void ClearTags();
	
	bool HasTag(EStateTag tag) const;
	bool HasAllTags(EStateTag tags) const;
	bool HasAnyTag(EStateTag tags) const;
	EStateTag GetTags() const { return currentTags; }

	// 디버그
	wstring GetStateTagString() const;

private:
	/*~ Component Interface ~*/
	void ComponentInit() override;
	void ComponentOnEnable() override {}
	void ComponentUpdate() override {}
	void ComponentRender() override {}
	void ComponentOnDisable() override {}
	void ComponentRelease() override {}

public:
	// 상태 변경 이벤트 (oldTags, newTags)
	MulticastDelegate<EStateTag, EStateTag> OnStateChanged;

private:
	EStateTag currentTags;
	// 단일 비트 태그별 참조 카운트 (Tag_Grounded, Tag_Airborne 등 -> 카운트)
	map<EStateTag, int> bitCountMap;
};
