// src/HighHeelDetector/HighHeelDetector.h
#pragma once
#include <nlohmann/json.hpp>
#include <vector>
#include <optional>
#include <string>

class HighHeelDetector {
public:
    struct FormIDRange {
        uint32_t min{};
        uint32_t max{};
    };

    struct Condition {
        std::optional<FormIDRange> formIDRange;
    };

    struct PluginRule {
        std::string pluginName;
        std::vector<Condition> conditions;  // AND 条件
    };

    struct HighHeelRule {
        std::vector<std::string> keywords;    // ByKeywords
        std::vector<PluginRule> pluginRules;  // ByPlugin
    };

    using HighHeelRules = std::vector<HighHeelRule>;  // OR

private:
    HighHeelRules rules_;

public:
    static HighHeelDetector& GetSingleton();
    bool Init(const std::string& jsonPath);
    bool IsHighHeel(RE::TESObjectARMO* armor) const;

private:
    void ParseJson(const nlohmann::json& j);

    HighHeelDetector() = default;
    ~HighHeelDetector() = default;
    HighHeelDetector(const HighHeelDetector&) = delete;
    HighHeelDetector(HighHeelDetector&&) = delete;
    HighHeelDetector& operator=(const HighHeelDetector&) = delete;
    HighHeelDetector& operator=(HighHeelDetector&&) = delete;
};