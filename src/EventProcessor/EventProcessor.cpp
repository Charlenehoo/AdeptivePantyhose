// src/EventProcessor/EventProcessor.cpp
#include "PCH.h"
#include "EventProcessor.h"

#include "BodyMorphManager/BodyMorphManager.h"

EventProcessor& EventProcessor::GetSingleton() {
    static EventProcessor instance;
    return instance;
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::TESEquipEvent* a_event,
                                                      RE::BSTEventSource<RE::TESEquipEvent>*) {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;
    RE::TESObjectREFR* actorRefr = a_event->actor.get();
    if (!actorRefr) return RE::BSEventNotifyControl::kContinue;
    RE::Actor* actor = actorRefr->As<RE::Actor>();
    if (!actor) return RE::BSEventNotifyControl::kContinue;

    RE::TESForm* form = RE::TESForm::LookupByID(a_event->baseObject);
    if (!form || !form->IsArmor()) return RE::BSEventNotifyControl::kContinue;

    RE::TESObjectARMO* armor = form->As<RE::TESObjectARMO>();
    if (!armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet)) return RE::BSEventNotifyControl::kContinue;

    const char* armorName = armor->GetName() ? armor->GetName() : "unnamed";

    BodyMorphManager::GetSingleton().GetInterface()->ClearMorph(actorRefr, "NoHeel", "AdeptivePantyhoseMorphKey");

    if (!a_event->equipped || !IsHighHeel(armor)) {
        BodyMorphManager::GetSingleton().GetInterface()->SetMorph(actorRefr, "NoHeel", "AdeptivePantyhoseMorphKey",
                                                                  1.0f);
        SKSE::log::trace("{} is barefoot or wearing non-high-heel footwear ({}), applying NoHeel morph",
                         actor->GetName(), armorName);
    } else {
        SKSE::log::trace("{} equipped high-heel footwear ({}), clearing NoHeel morph", actor->GetName(), armorName);
    }

    BodyMorphManager::GetSingleton().GetInterface()->UpdateModelWeight(actorRefr);
    return RE::BSEventNotifyControl::kContinue;
};

bool IsHighHeel(RE::TESObjectARMO* a_feetArmor) {
    if (!a_feetArmor) return false;

    if (a_feetArmor->HasKeywordString("SLA_KillerHeels")) {
        return true;
    }

    return false;
}