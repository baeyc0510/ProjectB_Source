#pragma once

class TransitionArea;
class Player;

class MapScene : public CScene
{
public:
    MapScene();
    ~MapScene();
    
    /*~ CMapScene Interfaces ~*/
    Player* SpawnPlayer();
    void SetSpawnId(int inSpawnId) { spawnId = inSpawnId; }
    
protected:
    /*~ CScene Interfaces ~*/
    void Init()		override;
    void Enter()	override;
    void Update()	override;
    void Render()	override;
    void Exit()		override;
    void Release()	override;
    
    // 맵 레이어 렌더링
    void RenderBackground() override;
    void RenderForeground() override;
    
    /*~ CMapScene Interfaces ~*/
    void SpawnTransitionArea();
    virtual void OnLoadMap();

protected:
    wstring mapFilePath;
    int spawnId;
};
