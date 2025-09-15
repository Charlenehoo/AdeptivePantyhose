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
    SKSE::log::trace("Loading high heel definitions from: {}", jsonPath);

    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        SKSE::log::error("HighHeelDetector::Init - Failed to open JSON file: {}", jsonPath);
        SKSE::log::trace("<<<< Exiting HighHeelDetector::Init (false)");
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
        SKSE::log::trace("HighHeelDetector::Init - JSON file loaded successfully");
    } catch (const nlohmann::json::parse_error& e) {
        SKSE::log::error("HighHeelDetector::Init - Failed to parse JSON: {}", e.what());
        SKSE::log::trace("<<<< Exiting HighHeelDetector::Init (false)");
        return false;
    }

    bool result = ParseJson(j);
    SKSE::log::trace("<<<< Exiting HighHeelDetector::Init ({})", result);
    return result;
}

namespace {
    inline bool IsValidFormIDRangeRulesJson(const nlohmann::json& a_ruleJson) {
        bool isValid = true;
        if (!a_ruleJson.is_object()) {
            isValid = false;
            SKSE::log::error("HighHeelDetector::ParseFormIDRange - Rule is not an object: {}", a_ruleJson.dump());
        }
        if (!a_ruleJson.contains("Plugin")) {
            isValid = false;
            SKSE::log::error("HighHeelDetector::ParseFormIDRange - Missing 'Plugin' field: {}", a_ruleJson.dump());
        }
        if (!a_ruleJson.contains("Min") || !a_ruleJson.contains("Max")) {
            isValid = false;
            SKSE::log::error("HighHeelDetector::ParseFormIDRange - Missing 'Min' or 'Max' field: {}",
                             a_ruleJson.dump());
        }
        return isValid;
    }
};

bool HighHeelDetector::ParseFormIDRange(const nlohmann::json& j) {
    const auto& formIDRangeRulesJson = j["ByFormIDRange"];
    if (!formIDRangeRulesJson.is_array()) {
        SKSE::log::error("Failed to parse JSON: 'ByFormIDRange' is not an array, got {}", formIDRangeRulesJson.dump());
        return false;
    }

    for (const auto& ruleJson : formIDRangeRulesJson) {
        if (!IsValidFormIDRangeRulesJson(ruleJson)) return false;
        try {
            FormIDRangeRule rule;
            rule.plugin = ruleJson["Plugin"].get<std::string>();
            rule.min = static_cast<RE::FormID>(std::stoul(ruleJson["Min"].get<std::string>(), nullptr, 16));
            rule.max = static_cast<RE::FormID>(std::stoul(ruleJson["Max"].get<std::string>(), nullptr, 16));
            formIDRangeRules_.push_back(rule);
        } catch (const std::exception& e) {
            SKSE::log::error("Failed to parse FormIDRange rule {}: {}", ruleJson.dump(), e.what());
            return false;
        }
    }
    return true;
}

bool HighHeelDetector::ParseKeywords(const nlohmann::json& j) {
    const auto& keywords = j["ByKeywords"];
    if (!keywords.is_array()) {
        SKSE::log::error("Failed to parse JSON: 'ByKeywords' is not an array, got {}", keywords.dump());
        return false;
    }

    for (const auto& kw : keywords) {
        if (!kw.is_string()) {
            SKSE::log::error("Failed to parse Keywords: expected string but got {}", kw.dump());
            return false;
        }

        keywordsRules_.push_back(kw.get<std::string>());
    }
    return true;
}

bool HighHeelDetector::ParseJson(const nlohmann::json& j) {
    keywordsRules_.clear();
    formIDRangeRules_.clear();
    return (ParseKeywords(j) && ParseFormIDRange(j));
}

bool HighHeelDetector::IsHighHeel(RE::TESObjectARMO* a_armor) const {
    if (!a_armor) return false;
    SKSE::log::trace("IsHighHeel called with armor: {}", a_armor->GetName());

    for (const auto& kw : keywordsRules_) {
        bool hasKeyword = a_armor->HasKeywordString(kw);
        SKSE::log::trace("IsHighHeel checking keyword: {}, result: {}", kw, hasKeyword);
        if (hasKeyword) return hasKeyword;
    }

    const RE::TESFile* file = a_armor->GetFile(0);
    if (!file) {
        SKSE::log::trace("IsHighHeel skiped FormID check due to invalid file pointer");
        return false;
    }

    std::string_view fileName = file->GetFilename();
    const RE::FormID localFormID = a_armor->GetLocalFormID();

    for (const auto& rule : formIDRangeRules_) {
        bool isPluginMatch = (fileName == rule.plugin);
        SKSE::log::trace("IsHighHeel checking armor plugin name: {}, required plugin nmae: {}, result: {}", fileName,
                         rule.plugin, isPluginMatch);
        if (!isPluginMatch) continue;

        bool isFormIDMatch = (localFormID >= rule.min && localFormID <= rule.max);
        SKSE::log::trace("IsHighHeel checking armor Form ID: 0x{:X}, required Form ID: 0x{:X} to 0x{:X}, result: {}",
                         localFormID, rule.min, rule.max, isFormIDMatch);
        if (isFormIDMatch) return true;
    }

    SKSE::log::trace("IsHighHeel return with armor: {}, overall result: {}", a_armor->GetName(), "mismatch");
    return false;
}