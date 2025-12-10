#pragma once
#include "ResourceTypes.h"
#include "VFXKeys.h"

struct VFXData
{
    static const vector<VFXEntry>& GetAllVFX()
    {
        static const vector<VFXEntry> vfxList = {
            /*~ Combat Effects ~*/
            {VFXKey::AttackHit1, TEXT("Animations/Effects/attack_spark1.json")},
            {VFXKey::AttackHit2, TEXT("Animations/Effects/attack_spark2.json")},
            {VFXKey::AttackHit3, TEXT("Animations/Effects/attack_spark3.json")},
            {VFXKey::PlayerHit, TEXT("Animations/Effects/playerhitspark.json")},
            {VFXKey::Blood1, TEXT("Animations/Effects/blood1.json")},
            {VFXKey::Blood2, TEXT("Animations/Effects/blood2.json")},

            /*~ Title Screen ~*/
            {VFXKey::Title_Bg, TEXT("Animations/MainMenu/crisanta-bg.json")},
            {VFXKey::Title_Main, TEXT("Animations/MainMenu/crisanta-main-menu-anim.json")},
            {VFXKey::Title_Petals, TEXT("Animations/MainMenu/crisanta-petals.json")},
            {VFXKey::Title_Fg, TEXT("Animations/MainMenu/crisanta-fg-petals.json")},
        };
        return vfxList;
    }
};
