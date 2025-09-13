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
                        rule.min = std::stoul(ruleJson["Min"].get<std::string>(), nullptr, 16);
                        rule.max = std::stoul(ruleJson["Max"].get<std::string>(), nullptr, 16);

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
        return false;
    }

    // 规则 1: 检查关键字是否匹配
    // std::any_of: 如果集合中至少有一个元素使 lambda 表达式返回 true, 则它也返回 true
    if (std::any_of(keywordsRules_.begin(), keywordsRules_.end(),
                    [a_armor](const std::string& kw) { return a_armor->HasKeywordString(kw.c_str()); })) {
        return true;  // 找到匹配的关键字，判定为高跟鞋
    }

    // 规则 2: 检查插件名和 FormID 范围是否匹配
    const auto* file = a_armor->GetFile(0);
    // 确保护甲来自一个有效的插件文件
    if (!file || !file->fileName) {
        return false;
    }

    const RE::FormID localFormID = a_armor->GetLocalFormID();
    const std::string_view armorPluginName(file->fileName);

    // 用于不区分大小写比较字符串的 lambda 表达式
    auto caseInsensitiveCompare = [](std::string_view s1, std::string_view s2) {
        return std::equal(s1.begin(), s1.end(), s2.begin(), s2.end(), [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
        });
    };

    if (std::any_of(formIDRangeRules_.begin(), formIDRangeRules_.end(), [&](const FormIDRangeRule& rule) {
            // 同时满足以下两个条件：
            // 1. 插件名匹配 (不区分大小写)
            // 2. 本地 FormID 在指定的 [Min, Max] 范围内
            return caseInsensitiveCompare(armorPluginName, rule.plugin) &&
                   (localFormID >= rule.min && localFormID <= rule.max);
        })) {
        return true;  // 找到匹配的 FormID 范围，判定为高跟鞋
    }

    // 所有规则都不匹配
    return false;
}