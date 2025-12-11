#pragma once
#include "Character.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Component/StatComponent.h"

class BossAIController;

class Boss : public Character, public ICombatInterface
{
public:
	Boss();
	~Boss() override;

	/*~ Boss Interface ~*/
	BossAIController* GetBossAI() const { return bossAI; }

	// 아레나 경계
	void SetArenaBounds(float minX, float maxX) { arenaMinX = minX; arenaMaxX = maxX; }
	float GetArenaMinX() const { return arenaMinX; }
	float GetArenaMaxX() const { return arenaMaxX; }
	bool IsAtArenaBoundary(int dir) const;		// 해당 방향으로 이동 시 아레나 경계에 도달했는지

	// 등장 시퀀스
	bool HasAppeared() const { return bHasAppeared; }
	void TriggerAppearance();
	virtual void OnAppearanceComplete();

	// 등장 사운드
	virtual wstring GetIntroSoundKey() const {return L"";}
	
protected:
	/*~ GameObject Interface ~*/
	void Init() override;
	void Update() override;
	void OnDisable() override;

	/*~ Character Interface ~*/
	void OnStateChanged(EStateTag oldTags, EStateTag newTags) override;
	
	/*~ ICombatInterface ~*/
	void OnDamage(GameObject* source, const CombatContext& context) override;
	bool IsDead() override;
	
	/*~ Boss Interface ~*/
	virtual void UpdateBossAnimation();
	virtual void UpdateBossAI();
	virtual bool CheckEncounterPlayer() = 0;
	bool IsTargetDead();
	void OnStatChanged(EStatType type, float& current, float& max) override;
	
protected:
	BossAIController* bossAI = nullptr;

	// 아레나 경계
	float arenaMinX = 0.f;
	float arenaMaxX = 0.f;

	// 상태
	bool bHasAppeared = false;
	bool bHasEncountered = false;
};
