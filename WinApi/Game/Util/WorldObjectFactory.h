#pragma once

class Character;

class WorldObjectFactory
{
public:
    static GameObject* CreateWorldObject(const string& objectType);
};