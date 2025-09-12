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

    nlohmann::json j;
    file >> j;
    ParseJson(j);
    return true;
}

void HighHeelDetector::ParseJson(const nlohmann::json& j) {
    for (auto& item : j) {
        HighHeelRule rule;

        // ByKeywords
        if (item.contains("ByKeywords")) {
            for (auto& kw : item["ByKeywords"]) {
                rule.keywords.push_back(kw.get<std::string>());
            }
        }

        // ByPlugin
        if (item.contains("ByPlugin")) {
            auto& bp = item["ByPlugin"];
            PluginRule pr;
            pr.pluginName = bp["Name"].get<std::string>();

            for (auto& cond : bp["Rule"]) {
                Condition c;
                if (cond.contains("ByFormIDRange")) {
                    auto& r = cond["ByFormIDRange"];
                    c.formIDRange = FormIDRange{std::stoul(r["Min"].get<std::string>(), nullptr, 16),
                                                std::stoul(r["Max"].get<std::string>(), nullptr, 16)};
                }
                pr.conditions.push_back(c);
            }
            rule.pluginRules.push_back(pr);
        }

        rules_.push_back(rule);
    }
}

bool HighHeelDetector::IsHighHeel(RE::TESObjectARMO* a_armor) const {
    if (!a_armor) return false;

    RE::FormID localFormID = a_armor->GetLocalFormID();
    auto file = a_armor->GetFile(0);
    std::string pluginName = file ? file->fileName : "";

    for (const auto& rule : rules_) {  // OR between rules

        // ByPlugin
        if (file) {
            for (auto& pr : rule.pluginRules) {
                if (pr.pluginName != pluginName) continue;

                // OR 多段 FormIDRange
                bool formIDMatch =
                    pr.conditions.empty() ||
                    std::any_of(pr.conditions.begin(), pr.conditions.end(), [localFormID](const Condition& c) {
                        return c.formIDRange && localFormID >= c.formIDRange->min && localFormID <= c.formIDRange->max;
                    });

                if (formIDMatch) return true;
            }
        }

        // ByKeywords
        if (std::any_of(rule.keywords.begin(), rule.keywords.end(),
                        [a_armor](const std::string& kw) { return a_armor->HasKeywordString(kw.c_str()); })) {
            return true;
        }
    }

    return false;
}