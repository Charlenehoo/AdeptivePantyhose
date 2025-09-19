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
    SKSE::log::trace(">>>> Entering EventProcessor::ProcessEvent");
    if (!a_event) {
        SKSE::log::warn("EventProcessor::ProcessEvent - Received null event");
        SKSE::log::trace("<<<< Exiting EventProcessor::ProcessEvent (null event)");
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::Actor* actor = a_event->actor->As<RE::Actor>();
    if (!actor) {
        SKSE::log::warn("EventProcessor::ProcessEvent - Actor reference is not an Actor");
        SKSE::log::trace("<<<< Exiting EventProcessor::ProcessEvent (not an actor)");
        return RE::BSEventNotifyControl::kContinue;
    }

    // These two check is too strick according to user report by DemiGod4789
    // RE::TESNPC* actorBase = actor->GetActorBase();
    // if (!actorBase) return RE::BSEventNotifyControl::kContinue;

    // if (actorBase->GetSex() != RE::SEXES::kFemale) return RE::BSEventNotifyControl::kContinue;

    // auto race = actorBase->race;
    // if (!race || !race->GetPlayable()) return RE::BSEventNotifyControl::kContinue;
    // Above two check is too strick according to user report by DemiGod4789

    RE::TESForm* form = RE::TESForm::LookupByID(a_event->baseObject);
    if (!form) {
        SKSE::log::warn("EventProcessor::ProcessEvent - Form not found for ID: {:X}", a_event->baseObject);
        SKSE::log::trace("<<<< Exiting EventProcessor::ProcessEvent (no form)");
        return RE::BSEventNotifyControl::kContinue;
    }
    if (!form->IsArmor()) {
        SKSE::log::trace("EventProcessor::ProcessEvent - Form is not armor");
        SKSE::log::trace("<<<< Exiting EventProcessor::ProcessEvent (not armor)");
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::TESObjectARMO* armor = form->As<RE::TESObjectARMO>();
    if (!armor) {
        SKSE::log::warn("EventProcessor::ProcessEvent - Failed to cast form to armor");
        SKSE::log::trace("<<<< Exiting EventProcessor::ProcessEvent (cast failed)");
        return RE::BSEventNotifyControl::kContinue;
    }
    if (!armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet)) {
        SKSE::log::trace("EventProcessor::ProcessEvent - Armor is not footwear");
        SKSE::log::trace("<<<< Exiting EventProcessor::ProcessEvent (not footwear)");
        return RE::BSEventNotifyControl::kContinue;
    }  // calve

    const char* actorName = actor->GetName() ? actor->GetName() : "unnamed";
    const char* armorName = armor->GetName() ? armor->GetName() : "unnamed";
    SKSE::log::trace("Processing equip event: actor={}, feetArmor={}, equipped={}", actorName, armorName,
                     a_event->equipped);

    if (a_event->equipped) {
        SKSE::log::trace("EventProcessor::ProcessEvent - Equipment equipped");
        BodyMorphManager::GetSingleton().UpdateHighHeelMorph(actor, HighHeelDetector::GetSingleton().IsHighHeel(armor));
    } else {
        SKSE::log::trace("EventProcessor::ProcessEvent - Equipment unequipped");
        BodyMorphManager::GetSingleton().UpdateHighHeelMorph(actor, false);
    }

    SKSE::log::trace("<<<< Exiting EventProcessor::ProcessEvent");
    return RE::BSEventNotifyControl::kContinue;
};