#pragma once

class Character;

class CharacterFactory
{
public:
    static Character* CreateCharacter(const string& characterType);
};