#pragma once

class OverlayUI;

class ScenePlayerDeath : public Scene
{
public:
    void Init()		override;
    void Enter()	override;
    void Update()	override;
    void Render()	override;
    void Exit()		override;
    void Release()	override;
    
private:
    OverlayUI* deathUI; 
};
