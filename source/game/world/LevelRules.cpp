#include "game/world/LevelRules.hpp"

#include <algorithm>

#include "game/core/DLCPack.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"


static const int g_mapRuleOrder[27] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    19, 18, 16, 17, 15, 14, 13, 12, 10,
    11, 9, 4, 8, 6, 3, 2, 1, 0
};

LevelRules::LevelRules()
{
}

void LevelRules::addLevelRule(const std::wstring&, LevelRuleset* ruleset)
{
    m_levelRules.push_back(ruleset);
}

void LevelRules::removeLevelRule(LevelRuleset* ruleset)
{
    std::vector<LevelRuleset*>::iterator rule;
    while ((rule = std::find(m_levelRules.begin(), m_levelRules.end(), ruleset)) != m_levelRules.end())
    {
        m_levelRules.erase(rule);
    }

    std::vector<LevelRulesetGroup*>::iterator group = m_ruleGroups.begin();
    while (group != m_ruleGroups.end())
    {
        std::vector<LevelRuleset*>::iterator member;
        while ((member = std::find((*group)->rulesets.begin(), (*group)->rulesets.end(), ruleset)) != (*group)->rulesets.end())
        {
            (*group)->rulesets.erase(member);
        }

        if ((*group)->rulesets.size() == 0)
        {
            delete *group;
            group = m_ruleGroups.erase(group);
        }
        else
        {
            ++group;
        }
    }
}

void LevelRules::getRulesByType(ELevelRuleType type, std::vector<LevelRuleset*>* rules, bool requirePurchase)
{
    rules->clear();
    std::vector<LevelRuleset*>::iterator rule = m_levelRules.begin();
    while (rule != m_levelRules.end())
    {
        LevelRuleset* ruleset = *rule;
        if (ruleset->getRuleType() == type)
        {
            if (!requirePurchase || ruleset->getGenerationOptions()->mPack->hasPurchasedPack())
            {
                rules->push_back(ruleset);
            }
        }
        ++rule;
    }
}

void LevelRules::populateRulesets()
{
    std::vector<LevelRuleset*>::iterator rule = m_levelRules.begin();
    while (rule != m_levelRules.end())
    {
        LevelRuleset* ruleset = *rule;
        LevelRulesetGroup* group = NULL;
        std::vector<LevelRulesetGroup*>::iterator existing = m_ruleGroups.begin();
        const std::vector<LevelRulesetGroup*>::iterator end = m_ruleGroups.end();
        if (existing != end)
        {
            const ELevelRuleType type = (*rule)->getRuleType();
            do
            {
                LevelRuleset* const& front = (*existing)->rulesets.front();
                if (front->getRuleType() == type && front->getThemeId() == (*rule)->getThemeId())
                {
                    group = *existing;
                    break;
                }
                ++existing;
            } while (existing != end);
        }

        if (group == NULL)
        {
            group = new LevelRulesetGroup();
            m_ruleGroups.push_back(group);
        }

        group->rulesets.push_back(ruleset);
        ++rule;
    }
}

void LevelRules::getRulesByType(ELevelRuleType type, std::vector<LevelRulesetGroup*>* groups, bool requirePurchase)
{
    if (m_ruleGroups.empty())
    {
        populateRulesets();
    }

    groups->clear();
    std::vector<LevelRulesetGroup*>::iterator group = m_ruleGroups.begin();
    while (group != m_ruleGroups.end())
    {
        LevelRulesetGroup* current = *group;
        LevelRuleset* ruleset = current->rulesets.front();
        if (ruleset->getRuleType() == type)
        {
            bool available = true;
            if (requirePurchase)
            {
                available = ruleset->getGenerationOptions()->mPack->hasPurchasedPack() ||
                            gApp.mDLCManager.IsMiniGameTemporarilyFree(static_cast<int>(type));
            }
            if (available)
            {
                groups->push_back(current);
            }
        }
        ++group;
    }
}

LevelGenerationOptions* LevelRules::getLevelGenFromUniqueId(ELevelRuleType type, int uniqueId) const
{
    const short theme = LevelGenerationOptions::GetThemeFromUniqueId(uniqueId);
    std::vector<LevelRulesetGroup*>::const_iterator group = m_ruleGroups.begin();
    const std::vector<LevelRulesetGroup*>::const_iterator end = m_ruleGroups.end();
    while (group != end)
    {
        LevelRuleset* const& ruleset = (*group)->rulesets.front();
        if (ruleset->getRuleType() == type)
        {
            if (theme == LevelGenerationOptions::GetThemeFromUniqueId(ruleset->getGenerationOptions()->getUniqueId()))
            {
                return ruleset->getGenerationOptions();
            }
        }
        ++group;
    }
    return NULL;
}

bool LevelRules::LevelRulesetGroup::SortMapRules(const LevelRulesetGroup* first, const LevelRulesetGroup* second)
{
    const int firstTheme = first->rulesets.front()->getThemeId();
    const int secondTheme = second->rulesets.front()->getThemeId();
    int firstIndex = -1;
    int secondIndex = -1;

    for (int offset = 0; offset < 18; ++offset)
    {
        if (g_mapRuleOrder[offset + 9] == firstTheme)
        {
            firstIndex = offset;
        }
        if (g_mapRuleOrder[offset + 9] == secondTheme)
        {
            secondIndex = offset;
        }
    }

    if (firstIndex != -1 && secondIndex != -1)
    {
        return firstIndex < secondIndex;
    }
    return secondTheme < firstTheme;
}
