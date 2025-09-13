// src/EventProcessor/EventProcessor.cpp
#include "PCH.h"
#include "EventProcessor.h"

#include "BodyMorphManager/BodyMorphManager.h"
#include "HighHeelDetector/HighHeelDetector.h"

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

    RE::TESNPC* actorBase = actor->GetActorBase();
    if (!actorBase) return RE::BSEventNotifyControl::kContinue;

    if (actorBase->GetSex() != RE::SEXES::kFemale) return RE::BSEventNotifyControl::kContinue;

    auto race = actorBase->race;
    if (!race || !race->GetPlayable()) return RE::BSEventNotifyControl::kContinue;

    RE::TESForm* form = RE::TESForm::LookupByID(a_event->baseObject);
    if (!form || !form->IsArmor()) return RE::BSEventNotifyControl::kContinue;

    RE::TESObjectARMO* armor = form->As<RE::TESObjectARMO>();
    if (!armor || !armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet))
        return RE::BSEventNotifyControl::kContinue;

    const char* armorName = armor->GetName() ? armor->GetName() : "unnamed";

    SKEE::IBodyMorphInterface* bodyInterface = BodyMorphManager::GetSingleton().GetInterface();
    if (!bodyInterface) return RE::BSEventNotifyControl::kContinue;

    bodyInterface->ClearMorph(actorRefr, "NoHeel", "CH_AdeptivePantyhoseMorphKey");
    bodyInterface->ClearMorph(actorRefr, "NoHeel", "AdeptivePantyhoseMorphKey");

    if (!a_event->equipped || !HighHeelDetector::GetSingleton().IsHighHeel(armor)) {
        bodyInterface->SetMorph(actorRefr, "NoHeel", "AdeptivePantyhoseMorphKey", 1.0f);
        SKSE::log::trace("{} is barefoot or wearing non-high-heel footwear ({}), applying NoHeel morph",
                         actor->GetName(), armorName);
    } else {
        SKSE::log::trace("{} equipped high-heel footwear ({}), clearing NoHeel morph", actor->GetName(), armorName);
    }

    bodyInterface->UpdateModelWeight(actorRefr);
    return RE::BSEventNotifyControl::kContinue;
};