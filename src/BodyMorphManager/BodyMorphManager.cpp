// src/BodyMorphManager/BodyMorphManager.cpp
#include "PCH.h"
#include "BodyMorphManager.h"

BodyMorphManager::BodyMorphManager() : m_bodyMorphInterface(nullptr) {}

BodyMorphManager& BodyMorphManager::GetSingleton() {
    static BodyMorphManager instance;
    return instance;
}

bool BodyMorphManager::Init() {
    SKSE::log::trace(">>>> Entering BodyMorphManager::Init");

    auto messagingInterface = SKSE::GetMessagingInterface();
    if (!messagingInterface) {
        SKSE::log::error("BodyMorphManager::Init - Failed to get messaging interface");
        SKSE::log::trace("<<<< Exiting BodyMorphManager::Init (false)");
        return false;
    }

    SKSE::log::trace("BodyMorphManager::Init - Dispatching interface exchange message");
    SKEE::InterfaceExchangeMessage interfaceExchangeMessage{};
    messagingInterface->Dispatch(static_cast<uint32_t>(SKEE::InterfaceExchangeMessage::kMessage_ExchangeInterface),
                                 &interfaceExchangeMessage, sizeof(interfaceExchangeMessage), "SKEE");

    auto interfaceMap = interfaceExchangeMessage.interfaceMap;
    if (!interfaceMap) {
        SKSE::log::error("BodyMorphManager::Init - Failed to get interface map");
        SKSE::log::trace("<<<< Exiting BodyMorphManager::Init (false)");
        return false;
    }

    SKSE::log::trace("BodyMorphManager::Init - Querying BodyMorph interface");
    m_bodyMorphInterface = static_cast<SKEE::IBodyMorphInterface*>(interfaceMap->QueryInterface("BodyMorph"));
    if (!m_bodyMorphInterface) {
        SKSE::log::error("BodyMorphManager::Init - Failed to get BodyMorph interface");
        SKSE::log::trace("<<<< Exiting BodyMorphManager::Init (false)");
        return false;
    }

    int version = m_bodyMorphInterface->GetVersion();
    if (m_bodyMorphInterface->GetVersion() < 4) {
        SKSE::log::error("BodyMorphManager::Init - Interface version too old (require >=4, got {})", version);
        SKSE::log::trace("<<<< Exiting BodyMorphManager::Init (false)");
        return false;
    }

    SKSE::log::trace("BodyMorphManager::Init - Initialization successful");
    SKSE::log::trace("<<<< Exiting BodyMorphManager::Init (true)");
    return true;
}

void BodyMorphManager::UpdateHighHeelMorph(RE::Actor* a_actor, bool isHighHeel) {
    SKSE::log::trace(">>>> Entering BodyMorphManager::UpdateHighHeelMorph");

    if (!a_actor) {
        SKSE::log::warn("BodyMorphManager::UpdateHighHeelMorph - Actor is null");
        SKSE::log::trace("<<<< Exiting BodyMorphManager::UpdateHighHeelMorph (no actor)");
        return;
    }

    const char* actorName = a_actor->GetName() ? a_actor->GetName() : "unnamed";
    SKSE::log::trace("BodyMorphManager::UpdateHighHeelMorph - Actor: {}", actorName);

    if (!m_bodyMorphInterface) {
        SKSE::log::error("BodyMorphManager::UpdateHighHeelMorph - BodyMorph interface not available");
        SKSE::log::trace("<<<< Exiting BodyMorphManager::UpdateHighHeelMorph (no interface)");
        return;
    }

    // clear lagacy morph with lagacy morph key
    SKSE::log::trace("BodyMorphManager::UpdateHighHeelMorph - Clearing legacy morph keys");
    m_bodyMorphInterface->ClearMorph(a_actor, "NoHeel", "CH_AdeptivePantyhoseMorphKey");

    m_bodyMorphInterface->ClearMorph(a_actor, "NoHeel", "AdeptivePantyhoseMorphKey");

    if (!isHighHeel) {
        SKSE::log::trace("BodyMorphManager::UpdateHighHeelMorph - Applying NoHeel morph");
        m_bodyMorphInterface->SetMorph(a_actor, "NoHeel", "AdeptivePantyhoseMorphKey", 1.0f);
    } else {
        SKSE::log::trace("BodyMorphManager::UpdateHighHeelMorph - Clearing NoHeel morph");
    }

    SKSE::log::trace("BodyMorphManager::UpdateHighHeelMorph - Updating model weight");
    m_bodyMorphInterface->UpdateModelWeight(a_actor);

    SKSE::log::trace("<<<< Exiting BodyMorphManager::UpdateHighHeelMorph");
}