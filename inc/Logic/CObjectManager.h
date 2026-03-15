#pragma once

class CObjectManager {
public:
    CObjectManager() = default;
    ~CObjectManager() = default;

    // Removes all active game objects from the scene.
    void DelAllObject();
};

extern CObjectManager g_ObjectManager;
