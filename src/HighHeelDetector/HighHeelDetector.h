// src/HighHeelDetector/HighHeelDetector.h
#pragma once
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

class HighHeelDetector {
public:
    struct FormIDRangeRule {
        std::string plugin{};
        RE::FormID min{};
        RE::FormID max{};
    };

private:
    std::vector<FormIDRangeRule> formIDRangeRules_;
    std::vector<std::string> keywordsRules_;

public:
    static HighHeelDetector& GetSingleton();
    bool Init(const std::string& jsonPath);
    bool IsHighHeel(RE::TESObjectARMO* armor) const;

private:
    bool ParseJson(const nlohmann::json& j);
    bool ParseKeywords(const nlohmann::json& j);
    bool ParseFormIDRange(const nlohmann::json& j);

    HighHeelDetector() = default;
    ~HighHeelDetector() = default;
    HighHeelDetector(const HighHeelDetector&) = delete;
    HighHeelDetector(HighHeelDetector&&) = delete;
    HighHeelDetector& operator=(const HighHeelDetector&) = delete;
    HighHeelDetector& operator=(HighHeelDetector&&) = delete;
};