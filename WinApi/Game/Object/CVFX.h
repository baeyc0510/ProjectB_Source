#pragma once

class CVFX : public CGameObject
{
public:
    CVFX();
    ~CVFX() override;

    /*~ CVFX Interface ~*/
    // SetLifetime, GetRemainingLifetime은 CGameObject에서 상속

    void SetLooping(bool inLooping);
    bool IsLooping() const { return bLooping; }

    void SetAnimation(CAnimation* inAnimation);

    void PlayVFX();
    void StopVFX();

protected:
    /*~ CGameObject Interface ~*/
    void Init() override;
    void OnEnable() override;
    void Update() override;
    void Render() override;
    void OnDisable() override;
    void Release() override;

    /*~ CVFX Interface ~*/
    virtual void OnFinishedAnimation();

private:
    CAnimator* animator;
    CAnimation* animation;
    bool bIsPlaying;
    bool bLooping;
};
