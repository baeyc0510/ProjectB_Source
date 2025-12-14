#pragma once

class GameObject;

// 상호작용 가능한 오브젝트 인터페이스
class IInteractable
{
public:
    virtual ~IInteractable() = default;

    // 상호작용 가능 여부 체크 (각 오브젝트가 자체 조건 검사)
    virtual bool CanInteract(GameObject* interactor) const = 0;

    // 상호작용 실행
    virtual void OnInteract(GameObject* interactor) = 0;
    
    // 상호작용 애니메이션 키
    virtual const wstring GetInteractionAnimKey(GameObject* interactor) const = 0; 
    
    // 상호작용 사운드 키
    virtual const wstring GetInteractionSFXKey(GameObject* interactor) const = 0;
};
