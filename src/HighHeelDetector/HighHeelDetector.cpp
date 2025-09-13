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
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        SKSE::log::error("Failed to open JSON: {}", jsonPath);
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const nlohmann::json::parse_error& e) {
        SKSE::log::error("Failed to parse JSON: {}", e.what());
        return false;
    }

    return ParseJson(j);
}

namespace {
    inline bool IsValidFormIDRangeRulesJson(const nlohmann::json& a_ruleJson) {
        return a_ruleJson.is_object() && a_ruleJson.contains("Plugin") && a_ruleJson.contains("Min") &&
               a_ruleJson.contains("Max");
    }
};

bool HighHeelDetector::ParseFormIDRange(const nlohmann::json& j) {
    const auto& formIDRangeRulesJson = j["ByFormIDRange"];
    if (!formIDRangeRulesJson.is_array()) return false;

    for (const auto& ruleJson : formIDRangeRulesJson) {
        if (!IsValidFormIDRangeRulesJson(ruleJson)) return false;
        try {
            FormIDRangeRule rule;
            rule.plugin = ruleJson["Plugin"].get<std::string>();
            rule.min = static_cast<RE::FormID>(std::stoul(ruleJson["Min"].get<std::string>(), nullptr, 16));
            rule.max = static_cast<RE::FormID>(std::stoul(ruleJson["Max"].get<std::string>(), nullptr, 16));
            formIDRangeRules_.push_back(rule);
        } catch (const std::exception& e) {
            SKSE::log::error("Failed to parse FormIDRange rule: {}", e.what());
            return false;
        }
    }
    return true;
}

bool HighHeelDetector::ParseKeywords(const nlohmann::json& j) {
    const auto& keywords = j["ByKeywords"];
    if (!keywords.is_array()) return false;

    for (const auto& kw : keywords) {
        if (!kw.is_string()) return false;

        keywordsRules_.push_back(kw.get<std::string>());
    }
    return true;
}

bool HighHeelDetector::ParseJson(const nlohmann::json& j) {
    keywordsRules_.clear();
    formIDRangeRules_.clear();
    if (!ParseKeywords(j)) {
        SKSE::log::error("Failed to parse JSON: {}", "Keywords");
        return false;
    }
    if (!ParseFormIDRange(j)) {
        SKSE::log::error("Failed to parse JSON: {}", "FormIDRange");
        return false;
    }
    return true;
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