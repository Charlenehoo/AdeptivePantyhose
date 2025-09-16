// src/Plugin.cpp
#include "PCH.h"

#include <spdlog/sinks/basic_file_sink.h>
#include "SKEE/IPluginInterface.h"
#include "BodyMorphManager/BodyMorphManager.h"
#include "EventProcessor/EventProcessor.h"
#include "HighHeelDetector/HighHeelDetector.h"

namespace {
    static bool g_firstTimePostLoadGame = true;
}

void HandleFirstTimePostLoadGame() {
    SKSE::log::trace(">>>> Entering HandleFirstTimePostLoadGame");

    RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        SKSE::log::warn("HandleFirstTimePostLoadGame: PlayerCharacter not found");
        SKSE::log::trace("<<<< Exiting HandleFirstTimePostLoadGame (no player)");
    }
    SKSE::log::trace("HandleFirstTimePostLoadGame: Player found: {}", player->GetName());

    RE::TESObjectARMO* feetArmor = player->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet);
    if (!feetArmor) {
        SKSE::log::trace("HandleFirstTimePostLoadGame: Player is barefoot");
        BodyMorphManager::GetSingleton().UpdateHighHeelMorph(player, false);
        SKSE::log::trace("<<<< Exiting HandleFirstTimePostLoadGame (barefoot)");
        return;
    }

    BodyMorphManager::GetSingleton().UpdateHighHeelMorph(player,
                                                         HighHeelDetector::GetSingleton().IsHighHeel(feetArmor));

    SKSE::log::trace("<<<< Exiting HandleFirstTimePostLoadGame");
}

void MessagingInterfaceEventCallback(SKSE::MessagingInterface::Message* a_msg) {
    SKSE::log::warn("MessagingInterfaceEventCallback: Received null message pointer");
    SKSE::log::trace(">>>> Entering MessagingInterfaceEventCallback");

    if (!a_msg) {
        SKSE::log::trace("<<<< Exiting MessagingInterfaceEventCallback (null message)");
    }

    SKSE::log::trace("Processing message type: {}", a_msg->type);
    switch (a_msg->type) {
        case SKSE::MessagingInterface::kPostPostLoad: {
            SKSE::log::trace("Handling kPostPostLoad message");

            SKSE::log::trace("Initializing BodyMorphManager...");
            BodyMorphManager::GetSingleton().Init();
        }

        break;
        case SKSE::MessagingInterface::kPostLoadGame: {
            SKSE::log::trace("Handling kPostLoadGame message");

            if (!g_firstTimePostLoadGame) {
                SKSE::log::trace("Not first time post-load game, skipping");
            } else {
                SKSE::log::trace("First time post-load game detected");

                g_firstTimePostLoadGame = false;
                HandleFirstTimePostLoadGame();

                SKSE::log::trace("HandleFirstTimePostLoadGame completed");
            }

        } break;
        default:
            break;
    }

    SKSE::log::trace("<<<< Exiting MessagingInterfaceEventCallback");
}

void InitLog() {
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();

    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder)
        SKSE::stl::report_and_fail(std::format("{}: failed to init log - logs folder not found", pluginName));

    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));

#ifdef DEBUG
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::warn);
#endif
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {
    SKSE::Init(a_skse);

    const auto plugin = SKSE::PluginDeclaration::GetSingleton();
    const auto pluginName = plugin->GetName();
    const auto pluginVersion = plugin->GetVersion().string(".");
    const auto gameVersion = REL::Module::get().version().string(".");
    const auto authorName = plugin->GetAuthor();
    const auto supportEmail = plugin->GetSupportEmail();

    InitLog();

    SKSE::log::info("==================================================");
    SKSE::log::info("Loading plugin: {}", pluginName);
    SKSE::log::info("Plugin version: {}", pluginVersion);
    SKSE::log::info("Game version: {}", gameVersion);
    SKSE::log::info("Author: {}", authorName);
    SKSE::log::info("Support: {}", supportEmail);
    SKSE::log::info("==================================================");

    SKSE::log::trace("Getting messaging interface...");
    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging) {
        SKSE::log::critical("Failed to get messaging interface");
        SKSE::stl::report_and_fail(std::format("{} failed to get messaging interface", pluginName));
    }
    SKSE::log::trace("Messaging interface acquired");

    SKSE::log::trace("Registering message listener...");
    if (!messaging->RegisterListener(MessagingInterfaceEventCallback)) {
        SKSE::log::critical("Failed to register message listener");
        SKSE::stl::report_and_fail(std::format("{} failed to register messaging interface listener", pluginName));
    }
    SKSE::log::trace("Message listener registered successfully");

    SKSE::log::trace("Initializing HighHeelDetector...");
    if (!HighHeelDetector::GetSingleton().Init("Data/SKSE/AdeptivePantyhose/DefineHighHeel.json")) {
        SKSE::log::critical("Failed to initialize HighHeelDetector");
        SKSE::stl::report_and_fail(std::format("{} failed to init high heel detector", pluginName));
    }

    SKSE::log::trace("Registering equip event sink...");
    RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESEquipEvent>(&EventProcessor::GetSingleton());
    SKSE::log::trace("Equip event sink registered");

    SKSE::log::info("Plugin loaded successfully");

    return true;
}