// src/Plugin.cpp
#include "PCH.h"

#include <spdlog/sinks/basic_file_sink.h>
#include "SKEE/IPluginInterface.h"
#include "BodyMorphManager/BodyMorphManager.h"
#include "EventProcessor/EventProcessor.h"
#include "HighHeelDetector/HighHeelDetector.h"

bool InitLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) return false;
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
    return true;
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {
    SKSE::Init(a_skse);

    const auto plugin = SKSE::PluginDeclaration::GetSingleton();
    const auto pluginName = plugin->GetName();
    const auto pluginVersion = plugin->GetVersion().string(".");
    const auto gameVersion = REL::Module::get().version().string(".");
    const auto authorName = plugin->GetAuthor();
    const auto supportEmail = plugin->GetSupportEmail();

    if (!InitLog()) SKSE::stl::report_and_fail(("{} failed to init log", pluginName));

    SKSE::log::info("Load plugin: {} v{} by {} with game version {}, if any question, you might reach her by {}.",
                    pluginName, pluginVersion, authorName, gameVersion, supportEmail);

    auto messaging = SKSE::GetMessagingInterface();

    if (!messaging) SKSE::stl::report_and_fail(("{} failed to get messaging interface", pluginName));

    if (!messaging->RegisterListener([](SKSE::MessagingInterface::Message* a_msg) {
            if (!a_msg) return;
            switch (a_msg->type) {
                case SKSE::MessagingInterface::kPostPostLoad: {
                    const auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
                    if (!BodyMorphManager::GetSingleton().Init()) {
                        SKSE::stl::report_and_fail(("{} failed to init body morph manager", pluginName));
                    }

                    if (!HighHeelDetector::GetSingleton().Init("Data/SKSE/AdeptivePantyhose/HighHeelRules.json")) {
                        SKSE::stl::report_and_fail(("{} failed to init high heel detector",
                                                    SKSE::PluginDeclaration::GetSingleton()->GetName()));
                    }
                } break;

                default:
                    break;
            }
        })) {
        SKSE::stl::report_and_fail(("{} failed to register messaging interface listener", pluginName));
    }

    auto& eventProcessor = EventProcessor::GetSingleton();
    RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESEquipEvent>(&eventProcessor);

    SKSE::log::info("Plugin loaded.");
    return true;
}