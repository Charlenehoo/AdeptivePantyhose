// src/Plugin.cpp
#include "PCH.h"

#include <spdlog/sinks/basic_file_sink.h>
#include "SKEE/IPluginInterface.h"
#include "BodyMorphManager/BodyMorphManager.h"
#include "EventProcessor/EventProcessor.h"

void InitLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {
    SKSE::Init(a_skse);

    InitLog();

    SKSE::log::info("Load plugin: {} by {}.", "AdeptivePantyhose", "CharleneHoo@hotmail.com");

    auto messaging = SKSE::GetMessagingInterface();

    if (!messaging) SKSE::stl::report_and_fail(("{} failed to get messaging interface", "AdeptivePantyhose"));

    if (!messaging->RegisterListener([](SKSE::MessagingInterface::Message* a_msg) {
            if (!a_msg) return;
            switch (a_msg->type) {
                case SKSE::MessagingInterface::kPostPostLoad:
                    if (!BodyMorphManager::GetSingleton().Init())
                        SKSE::stl::report_and_fail(("{} failed to init body morph manager", "AdeptivePantyhose"));
                    break;

                default:
                    break;
            }
        })) {
        SKSE::stl::report_and_fail(("{} failed to register messaging interface listener", "AdeptivePantyhose"));
    }

    auto& eventProcessor = EventProcessor::GetSingleton();
    RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESEquipEvent>(&eventProcessor);

    SKSE::log::info("Plugin loaded.");
    return true;
}