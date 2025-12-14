#pragma once

class Character;

class WorldObjectFactory
{
public:
    static GameObject* CreateWorldCharacter(const string& objectType);
};