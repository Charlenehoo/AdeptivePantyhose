// src/BodyMorphManager/BodyMorphManager.cpp
#include "PCH.h"
#include "BodyMorphManager.h"

BodyMorphManager::BodyMorphManager() : m_bodyMorphInterface(nullptr) {}

BodyMorphManager& BodyMorphManager::GetSingleton() {
    static BodyMorphManager instance;
    return instance;
}

bool BodyMorphManager::Init() {
    auto messagingInterface = SKSE::GetMessagingInterface();
    if (!messagingInterface) return false;

    SKEE::InterfaceExchangeMessage interfaceExchangeMessage{};
    messagingInterface->Dispatch(static_cast<uint32_t>(SKEE::InterfaceExchangeMessage::kMessage_ExchangeInterface),
                                 &interfaceExchangeMessage, sizeof(interfaceExchangeMessage), "SKEE");

    auto interfaceMap = interfaceExchangeMessage.interfaceMap;
    if (!interfaceMap) return false;

    m_bodyMorphInterface = static_cast<SKEE::IBodyMorphInterface*>(interfaceMap->QueryInterface("BodyMorph"));
    if (!m_bodyMorphInterface) return false;

    if (m_bodyMorphInterface->GetVersion() < 4) return false;

    return true;
}

SKEE::IBodyMorphInterface* BodyMorphManager::GetInterface() const { return m_bodyMorphInterface; }