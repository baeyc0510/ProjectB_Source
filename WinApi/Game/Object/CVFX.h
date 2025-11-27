#pragma once

class CVFX : public CGameObject
{
public:
    CVFX();
    ~CVFX() override;
    
    void Init() override;
    void OnEnable() override;
    void Update() override;
    void Render() override;
    void OnDisable() override;
    void Release() override;
    
    void SetLifetime(float inLifetime);
    float GetRemainingLifetime() const;
    
    void SetLooping(bool inLooping);
    bool IsLooping() const { return bLooping; }
    
    void SetAnimation(CAnimation* inAnimation);
    
    void PlayVFX();
    void StopVFX();

protected:
    virtual void OnFinishedAnimation();
    
protected:
    CAnimator* animator;
    
private:
    CAnimation* animation;
    float lifetime;
    float remainingLifetime;
    bool bHasLifetime;
    bool bIsPlaying;
    bool bLooping;
};
