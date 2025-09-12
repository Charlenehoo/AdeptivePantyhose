// src/EventProcessor/EventProcessor.h
#pragma once

class EventProcessor : public RE::BSTEventSink<RE::TESEquipEvent> {
private:
    EventProcessor() = default;
    ~EventProcessor() = default;
    EventProcessor(const EventProcessor&) = delete;
    EventProcessor(EventProcessor&&) = delete;
    EventProcessor& operator=(const EventProcessor&) = delete;
    EventProcessor& operator=(EventProcessor&&) = delete;

public:
    static EventProcessor& GetSingleton();
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event,
                                          RE::BSTEventSource<RE::TESEquipEvent>*) override;
};

// bool IsHighHeel(RE::TESObjectARMO* a_feetArmor);
