#pragma once

class VFXObject : public GameObject
{
public:
    VFXObject();
    ~VFXObject() override;

    /*~ VFXObject Interface ~*/

    void SetLooping(bool inLooping);
    bool IsLooping() const { return bLooping; }

    void SetAnimation(AnimationResource* inAnimation);

    void PlayVFX();
    void StopVFX();

protected:
    /*~ GameObject Interface ~*/
    void Init() override;
    void OnEnable() override;
    void Update() override;
    void Render() override;
    void OnDisable() override;
    void Release() override;

    /*~ VFXObject Interface ~*/
    virtual void OnFinishedAnimation();

private:
    Animator* animator;
    AnimationResource* animation;
    bool bIsPlaying;
    bool bLooping;
};
