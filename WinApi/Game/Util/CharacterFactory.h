#pragma once

class CCharacter;

class CharacterFactory
{
public:
    static CCharacter* CreateCharacter(const string& characterType);
};