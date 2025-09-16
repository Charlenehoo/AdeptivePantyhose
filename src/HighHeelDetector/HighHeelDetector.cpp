// src/HighHeelDetector/HighHeelDetector.cpp
#include "PCH.h"
#include "HighHeelDetector.h"
#include <fstream>
#include <sstream>

HighHeelDetector& HighHeelDetector::GetSingleton() {
    static HighHeelDetector instance;
    return instance;
}

bool HighHeelDetector::Init(const std::string& jsonPath) {
    SKSE::log::trace(">>>> Entering HighHeelDetector::Init");
    SKSE::log::info("Initializing HighHeelDetector from '{}'...", jsonPath);

    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        SKSE::log::error("Failed to open high heel definition file: {}", jsonPath);
        SKSE::log::trace("<<<< Exiting HighHeelDetector::Init (result: false)");
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
        SKSE::log::debug("JSON file parsed successfully.");
    } catch (const nlohmann::json::parse_error& e) {
        SKSE::log::error("Failed to parse JSON from '{}'. Details: {}", jsonPath, e.what());
        SKSE::log::trace("<<<< Exiting HighHeelDetector::Init (result: false)");
        return false;
    }

    if (ParseJson(j)) {
        SKSE::log::info(
            "HighHeelDetector initialized successfully. Loaded {} keyword rule(s) and {} FormID range rule(s).",
            keywordsRules_.size(), formIDRangeRules_.size());
        SKSE::log::trace("<<<< Exiting HighHeelDetector::Init (result: true)");
        return true;
    } else {
        SKSE::log::error("Failed to process rules from high heel definition file.");
        SKSE::log::trace("<<<< Exiting HighHeelDetector::Init (result: false)");
        return false;
    }
}

namespace {
    inline bool IsValidFormIDRangeRulesJson(const nlohmann::json& a_ruleJson) {
        if (!a_ruleJson.is_object() || !a_ruleJson.contains("Plugin") || !a_ruleJson.contains("Min") ||
            !a_ruleJson.contains("Max")) {
            return false;
        }
        return true;
    }
};

bool HighHeelDetector::ParseFormIDRange(const nlohmann::json& j) {
    SKSE::log::trace(">>>> Entering HighHeelDetector::ParseFormIDRange");

    if (!j.contains("ByFormIDRange")) {
        SKSE::log::warn("'ByFormIDRange' section not found in JSON, skipping.");
        SKSE::log::trace("<<<< Exiting HighHeelDetector::ParseFormIDRange (result: true)");
        return true;
    }

    const auto& formIDRangeRulesJson = j["ByFormIDRange"];
    if (!formIDRangeRulesJson.is_array()) {
        SKSE::log::error("'ByFormIDRange' must be an array. JSON content: {}", formIDRangeRulesJson.dump());
        SKSE::log::trace("<<<< Exiting HighHeelDetector::ParseFormIDRange (result: false)");
        return false;
    }

    for (const auto& ruleJson : formIDRangeRulesJson) {
        if (!IsValidFormIDRangeRulesJson(ruleJson)) {
            SKSE::log::error("Invalid FormID range rule found, skipping. Rule content: {}", ruleJson.dump());
            continue;
        }
        try {
            FormIDRangeRule rule;
            rule.plugin = ruleJson["Plugin"].get<std::string>();
            rule.min = static_cast<RE::FormID>(std::stoul(ruleJson["Min"].get<std::string>(), nullptr, 16));
            rule.max = static_cast<RE::FormID>(std::stoul(ruleJson["Max"].get<std::string>(), nullptr, 16));

            if (rule.max <= rule.min) {
                SKSE::log::warn(
                    "Invalid FormID range rule for plugin '{}', skipping: Min ({:#x}) is greater than Max ({:#x}). ",
                    rule.plugin, rule.min, rule.max);
                continue;
            }

            formIDRangeRules_.push_back(rule);
            SKSE::log::debug("Loaded FormID range rule: Plugin='{}', Min={:#x}, Max={:#x}", rule.plugin, rule.min,
                             rule.max);
        } catch (const std::exception& e) {
            SKSE::log::error("Failed to parse FormIDRange rule. Content: {}. Details: {}", ruleJson.dump(), e.what());
            continue;
        }
    }
    return true;
}

bool HighHeelDetector::ParseKeywords(const nlohmann::json& j) {
    SKSE::log::trace(">>>> Entering HighHeelDetector::ParseKeywords");

    if (!j.contains("ByKeywords")) {
        SKSE::log::warn("'ByKeywords' section not found in JSON, skipping.");
        SKSE::log::trace("<<<< Exiting HighHeelDetector::ParseKeywords (result: true)");
        return true;
    }

    const auto& keywords = j["ByKeywords"];
    if (!keywords.is_array()) {
        SKSE::log::error("'ByKeywords' must be an array. JSON content: {}", keywords.dump());
        SKSE::log::trace("<<<< Exiting HighHeelDetector::ParseKeywords (result: false)");
        return false;
    }

    for (const auto& kw : keywords) {
        if (!kw.is_string()) {
            SKSE::log::error("Keyword must be a string, skipping. Content: {}", kw.dump());
            continue;
        }
        auto keywordStr = kw.get<std::string>();
        keywordsRules_.push_back(keywordStr);
        SKSE::log::debug("Loaded keyword rule: '{}'", keywordStr);
    }
    SKSE::log::trace("<<<< Exiting HighHeelDetector::ParseKeywords (result: true)");
    return true;
}

bool HighHeelDetector::ParseJson(const nlohmann::json& j) {
    SKSE::log::trace(">>>> Entering HighHeelDetector::ParseJson");
    keywordsRules_.clear();
    formIDRangeRules_.clear();

    bool keywordsParsed = ParseKeywords(j);
    bool formIDRangeParsed = ParseFormIDRange(j);

    bool result = keywordsParsed && formIDRangeParsed;
    SKSE::log::trace("<<<< Exiting HighHeelDetector::ParseJson (result: {})", result);
    return result;
}

bool HighHeelDetector::IsHighHeel(RE::TESObjectARMO* a_armor) const {
    SKSE::log::trace(">>>> Entering HighHeelDetector::IsHighHeel");

    if (!a_armor) {
        SKSE::log::trace("<<<< Exiting HighHeelDetector::IsHighHeel (result: false, armor is null)");
        return false;
    }

    SKSE::log::debug("Checking if armor '{}' (FormID: {:#x}) is a high heel...", a_armor->GetName(),
                     a_armor->GetFormID());

    for (const auto& kw : keywordsRules_) {
        if (a_armor->HasKeywordString(kw)) {
            SKSE::log::debug("Decision: YES. Matched keyword rule: '{}'.", kw);
            SKSE::log::trace("<<<< Exiting HighHeelDetector::IsHighHeel (result: true)");
            return true;
        }
    }

    const RE::TESFile* file = a_armor->GetFile(0);
    if (!file) {
        SKSE::log::trace("Cannot check by FormID: armor has an invalid file pointer.");
        SKSE::log::trace("<<<< Exiting HighHeelDetector::IsHighHeel (result: false)");
        return false;
    }

    std::string_view fileName = file->GetFilename();
    const RE::FormID localFormID = a_armor->GetLocalFormID();

    for (const auto& rule : formIDRangeRules_) {
        if (fileName == rule.plugin && localFormID >= rule.min && localFormID <= rule.max) {
            SKSE::log::debug(
                "Decision: YES. Matched FormID range rule. Plugin: '{}', FormID {:#x} is in [{:#x} - {:#x}].",
                rule.plugin, localFormID, rule.min, rule.max);
            SKSE::log::trace("<<<< Exiting HighHeelDetector::IsHighHeel (result: true)");
            return true;
        }
    }

    SKSE::log::debug("Decision: NO. No matching rules found for armor '{}'.", a_armor->GetName());
    SKSE::log::trace("<<<< Exiting HighHeelDetector::IsHighHeel (result: false)");
    return false;
}