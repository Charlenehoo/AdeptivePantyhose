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
        SKSE::log::error("Failed to open HighHeel JSON: {}", jsonPath);
        return false;
    }

    // 从文件流解析 JSON
    nlohmann::json j;
    try {
        file >> j;
    } catch (const nlohmann::json::parse_error& e) {
        SKSE::log::error("Failed to parse HighHeel JSON: {}", e.what());
        return false;
    }

    ParseJson(j);
    return true;
}

void HighHeelDetector::ParseJson(const nlohmann::json& j) {
    // 解析前先清空旧规则
    keywordsRules_.clear();
    formIDRangeRules_.clear();

    // 1. 解析 "ByKeywords" 数组
    if (j.contains("ByKeywords")) {
        const auto& keywords = j["ByKeywords"];
        if (keywords.is_array()) {
            for (const auto& kw : keywords) {
                if (kw.is_string()) {
                    keywordsRules_.push_back(kw.get<std::string>());
                }
            }
        }
    }

    // 2. 解析 "ByFormIDRange" 数组
    if (j.contains("ByFormIDRange")) {
        const auto& ranges = j["ByFormIDRange"];
        if (ranges.is_array()) {
            for (const auto& ruleJson : ranges) {
                // 确保每个元素都是对象且包含必要的键
                if (ruleJson.is_object() && ruleJson.contains("Plugin") && ruleJson.contains("Min") &&
                    ruleJson.contains("Max")) {
                    try {
                        FormIDRangeRule rule;
                        rule.plugin = ruleJson["Plugin"].get<std::string>();

                        // 从十六进制字符串转换为无符号整数 (RE::FormID)
                        rule.min = static_cast<RE::FormID>(std::stoul(ruleJson["Min"].get<std::string>(), nullptr, 16));
                        rule.max = static_cast<RE::FormID>(std::stoul(ruleJson["Max"].get<std::string>(), nullptr, 16));

                        formIDRangeRules_.push_back(rule);
                    } catch (const std::exception& e) {
                        // 捕获 std::stoul 可能抛出的异常 (如格式错误)
                        SKSE::log::error("Failed to parse FormIDRange rule: {}", e.what());
                    }
                }
            }
        }
    }
}

bool HighHeelDetector::IsHighHeel(RE::TESObjectARMO* a_armor) const {
    if (!a_armor) {
        SKSE::log::trace("IsHighHeel: armor is null");
        return false;
    }

    const char* armorName = a_armor->GetName();
    const auto* file = a_armor->GetFile(0);
    const char* fileName = file ? file->fileName : "<null>";
    const RE::FormID localFormID = a_armor->GetLocalFormID();
    std::string_view armorPluginName(fileName);

    SKSE::log::trace("IsHighHeel called: armor name: '{}', plugin: '{}', localFormID: {:X}", armorName, fileName,
                     localFormID);

    // 规则 1: 检查关键字
    for (const auto& kw : keywordsRules_) {
        bool hasKw = a_armor->HasKeywordString(kw.c_str());
        SKSE::log::trace("Checking keyword '{}': {}", kw, hasKw ? "matched" : "not matched");
        if (hasKw) return true;
    }

    // 规则 2: 检查插件名和 FormID
    if (!file || !file->fileName) {
        SKSE::log::trace("Plugin file null, cannot match FormIDRange");
        return false;
    }

    for (const auto& rule : formIDRangeRules_) {
        bool pluginMatch = (armorPluginName == rule.plugin);
        bool formIDMatch = (localFormID >= rule.min && localFormID <= rule.max);

        SKSE::log::trace(
            "Checking FormIDRange: rule.plugin='{}', rule.min={:X}, rule.max={:X} => pluginMatch={}, formIDMatch={}",
            rule.plugin, rule.min, rule.max, pluginMatch, formIDMatch);

        if (pluginMatch && formIDMatch) return true;
    }

    SKSE::log::trace("IsHighHeel result: false for armor '{}', plugin '{}', localFormID {:X}", armorName, fileName,
                     localFormID);

    return false;
}