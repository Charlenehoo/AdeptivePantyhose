// src/BodyMorphManager/BodyMorphManager.h
#pragma once
#include "SKEE/IPluginInterface.h"

class BodyMorphManager {
private:
    BodyMorphManager();
    ~BodyMorphManager() = default;
    BodyMorphManager(const BodyMorphManager&) = delete;
    BodyMorphManager(BodyMorphManager&&) = delete;
    BodyMorphManager& operator=(const BodyMorphManager&) = delete;
    BodyMorphManager& operator=(BodyMorphManager&&) = delete;

    SKEE::IBodyMorphInterface* m_bodyMorphInterface;

public:
    static BodyMorphManager& GetSingleton();
    bool Init();
    SKEE::IBodyMorphInterface* GetInterface() const;
};