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

	/*~ CBoss Interface ~*/
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

	
protected:
	/*~ CGameObject Interface ~*/
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

	/*~ CCharacter Interface ~*/
	void OnStateChanged(EStateTag oldTags, EStateTag newTags) override;
	
	/*~ ICombatInterface ~*/
	void OnDamage(CGameObject* source, const CombatContext& context) override;

	/*~ CBoss Interface ~*/
	virtual void UpdateBossAnimation();
	virtual void UpdateBossAI();
	virtual bool CheckEncounterPlayer() = 0;
	void OnStatChanged(EStatType type, float current, float max) override;
	
protected:
	BossAIController* bossAI = nullptr;

	// 아레나 경계
	float arenaMinX = 0.f;
	float arenaMaxX = 0.f;

	// 상태
	bool bHasAppeared = false;
	bool bHasEncountered = false;
};
