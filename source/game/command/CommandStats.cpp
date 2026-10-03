#include "game/command/CommandStats.hpp"
#include "game/data/CompoundTag.hpp"

CommandStats::CommandStats() : m_objectiveNames(0), m_entityNames(0) {}
CommandStats::~CommandStats() {}

void CommandStats::setCommandStats(MinecraftServer*, boost::shared_ptr<CommandSender> sender, CommandStatType type, int value)
{
    (void)sender;
    (void)type;
    (void)value;
}

void CommandStats::setCommandStats(boost::shared_ptr<CommandSender> sender, CommandStatType type, int value)
{
    (void)sender;
    (void)type;
    (void)value;
}

void CommandStats::unsetStatsSetting(CommandStats* stats, CommandStatType type)
{
    if (stats == 0 || type < SUCCESS_COUNT || type > QUERY_RESULT) return;
    if (stats->m_objectiveNames != 0) stats->m_objectiveNames[type].clear();
    if (stats->m_entityNames != 0) stats->m_entityNames[type].clear();
}

void CommandStats::assignStatsSetting(CommandStats* stats, CommandStatType type, std::wstring* objective, std::wstring* name)
{
    if (stats == 0 || type < SUCCESS_COUNT || type > QUERY_RESULT) return;
    if (stats->m_objectiveNames == 0) stats->m_objectiveNames = new std::wstring[5];
    if (stats->m_entityNames == 0) stats->m_entityNames = new std::wstring[5];
    if (objective != 0) stats->m_objectiveNames[type] = *objective;
    if (name != 0) stats->m_entityNames[type] = *name;
}

void CommandStats::read(CompoundTag* tag)
{
    static const wchar_t* prefixes[5] = { L"SuccessCount", L"AffectedBlocks", L"AffectedEntities", L"AffectedItems", L"QueryResult" };
    if (tag == 0) return;
    CompoundTag* statsTag = tag->getCompound(L"CommandStats");
    if (statsTag == 0) return;
    int i;
    for (i = 0; i < 5; ++i) {
        std::wstring nameKey = std::wstring(prefixes[i]) + L"Name";
        std::wstring objectiveKey = std::wstring(prefixes[i]) + L"Objective";
        if (statsTag->contains(nameKey) && statsTag->contains(objectiveKey)) {
            std::wstring name = statsTag->getString(nameKey);
            std::wstring objective = statsTag->getString(objectiveKey);
            assignStatsSetting(this, static_cast<CommandStatType>(i), &objective, &name);
        }
    }
}

void CommandStats::write(CompoundTag* tag)
{
    if (tag == 0) return;
    static const wchar_t* prefixes[5] = { L"SuccessCount", L"AffectedBlocks", L"AffectedEntities", L"AffectedItems", L"QueryResult" };
    CompoundTag* result = new CompoundTag();
    int i;
    for (i = 0; i < 5; ++i) {
        if (m_entityNames != 0 && m_objectiveNames != 0 && !m_entityNames[i].empty() && !m_objectiveNames[i].empty()) {
            result->putString(std::wstring(prefixes[i]) + L"Name", m_entityNames[i]);
            result->putString(std::wstring(prefixes[i]) + L"Objective", m_objectiveNames[i]);
        }
    }
    if (!result->isEmpty()) tag->put(L"CommandStats", result);
}

void CommandStats::cloneFrom(CommandStats* stats)
{
    if (stats == 0) return;
    int i;
    for (i = 0; i < 5; ++i) {
        if (stats->m_objectiveNames != 0 && stats->m_entityNames != 0)
            assignStatsSetting(this, static_cast<CommandStatType>(i), &stats->m_objectiveNames[i], &stats->m_entityNames[i]);
    }
}

bool CommandStats::hasStats(CommandStatType type)
{
    return type >= SUCCESS_COUNT && type <= QUERY_RESULT && m_objectiveNames != 0 && m_entityNames != 0 &&
           !m_objectiveNames[type].empty() && !m_entityNames[type].empty();
}
