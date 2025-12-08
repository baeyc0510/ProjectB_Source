#pragma once

class CTransitionArea;
class CPlayer;

class CMapScene : public CScene
{
public:
    CMapScene();
    ~CMapScene();
    
    /*~ CMapScene Interfaces ~*/
    CPlayer* SpawnPlayer();
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
