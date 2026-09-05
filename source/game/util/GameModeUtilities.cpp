#include "game/util/GameModeUtilities.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Mth.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/WeighedTreasure.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/AddItemRuleDefinition.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/PlayerTeam.hpp"

namespace
{
struct ChestItemsInfoLess
{
    bool operator()(const MiniGameDef::ChestItemsInfo& left,
                    const MiniGameDef::ChestItemsInfo& right) const
    {
        return left.mKpScore < right.mKpScore;
    }
};
}

GameModeUtilities::Position::Position() : x(0.0), z(0.0), y(0.0) {}

GameModeUtilities::Position::Position(double xValue, double zValue)
    : x(xValue), z(zValue), y(Random().nextDouble() * 360.0) {}

void GameModeUtilities::Position::randomize(Random* random, double minX, double minZ,
                                            double maxX, double maxZ)
{
    x = Mth::nextDouble(random, minX, maxX);
    z = Mth::nextDouble(random, minZ, maxZ);
}

bool GameModeUtilities::Position::isSafeInRange(Level* level, int minY, int maxY)
{
    unsigned int ix = (unsigned int)x;
    unsigned int iz = (unsigned int)z;
    if (x < (double)(int)ix) --ix;
    if (z < (double)(int)iz) --iz;
    if (maxY > level->getHeight()) maxY = level->getHeight();
    if (minY < 2) minY = 2;
    while (minY < maxY)
    {
        BlockPos lower((int)ix, minY, (int)iz);
        BlockPos upper = lower.above();
        const BlockState* floor = level->getBlockState(lower.below());
        const BlockState* head = level->getBlockState(upper);
        if (floor != NULL && floor->getMaterial() != Material::AIR)
        {
            if (floor->getMaterial() == Material::AIR || head == NULL ||
                head->getMaterial() == Material::WATER)
                return false;
            return true;
        }
        minY += 2;
    }
    return false;
}

void GameModeUtilities::Position::randomize(Random* random, std::vector<long long>* positions)
{
    x = 0.0;
    z = 0.0;
    if (positions != NULL && !positions->empty())
    {
        long long packed = positions->at((unsigned int)Mth::nextInt(random, 0, (int)positions->size() - 1));
        BlockPos position = BlockPos::decompressBlockPos(packed);
        x = position.x;
        z = position.z;
    }
}

void GameModeUtilities::Position::set(double xValue, double zValue)
{
    x = xValue;
    z = zValue;
}

void GameModeUtilities::Position::dist(Position* other)
{
    std::sqrt((x - other->x) * (x - other->x) + (z - other->z) * (z - other->z));
}

int GameModeUtilities::Position::getSpawnY(Level* level)
{
    int ix = (int)x;
    int iz = (int)z;
    if (x < ix) --ix;
    if (z < iz) --iz;
    int y = 256;
    for (int i = 0; i < 64; ++i, y -= 4)
    {
        const BlockState* state = level->getBlockState(BlockPos(ix, y, iz));
        if (state != NULL && state->getMaterial() != Material::AIR)
            return state->getMaterial() == Material::WATER ? y + 50 : y + 1;
    }
    return 0;
}

bool GameModeUtilities::DistributeItemsSpread(const std::vector<boost::shared_ptr<Container> >* containers,
                                              const std::vector<not_null_ptr<ItemInstance> >* items)
{
    if (containers == NULL || items == NULL || containers->empty() || items->empty()) return false;
    Random random;
    std::vector<not_null_ptr<ItemInstance> > remaining(*items);
    unsigned int baseCount = (unsigned int)remaining.size() / (unsigned int)containers->size();
    unsigned int remainder = (unsigned int)remaining.size() % (unsigned int)containers->size();
    std::vector<unsigned int> counts(containers->size(), baseCount);
    for (unsigned int i = 0; i < remainder; ++i)
    {
        unsigned int index = (unsigned int)random.nextInt((int)containers->size());
        while (counts[index] != baseCount)
            index = (unsigned int)random.nextInt((int)containers->size());
        ++counts[index];
    }
    for (unsigned int i = 0; i < containers->size(); ++i)
    {
        for (unsigned int j = 0; j < counts[i]; ++j)
        {
            unsigned int index = (unsigned int)random.nextInt((int)remaining.size());
            AddItemRuleDefinition::addItemToContainer(containers->at(i), remaining.at(index), -1, true);
            remaining.erase(remaining.begin() + index);
        }
    }
    return true;
}

int GameModeUtilities::SelectRandomKit(const std::vector<MiniGameDef::ChestKitInfo>* kits,
                                       unsigned int* weights, Random* random)
{
    if (kits == NULL || kits->empty()) return -1;
    std::vector<unsigned int> localWeights;
    if (weights == NULL)
    {
        localWeights.resize(kits->size());
        for (unsigned int i = 0; i < kits->size(); ++i)
            localWeights[i] = (unsigned int)kits->at(i).mWeight;
        weights = &localWeights[0];
    }
    unsigned int total = 0;
    for (unsigned int i = 0; i < kits->size(); ++i)
        total += weights[i];
    if (total == 0) return 0;
    unsigned int value = (unsigned int)random->nextInt((int)total);
    for (unsigned int i = 0; i < kits->size(); ++i)
    {
        if (value < weights[i])
        {
            weights[i] >>= 1;
            return (int)i;
        }
        value -= weights[i];
    }
    return (int)kits->size() - 1;
}

void GameModeUtilities::GenerateContainerContentsGreedy(std::vector<not_null_ptr<ItemInstance> >* output,
                                                        const std::vector<MiniGameDef::ChestItemsInfo>* items,
                                                        Random* random, unsigned int playerCount, unsigned int score)
{
    if (output == NULL || items == NULL) return;
    std::vector<MiniGameDef::ChestItemsInfo> sorted(*items);
    std::sort(sorted.begin(), sorted.end(), ChestItemsInfoLess());
    unsigned int currentScore = score;
    unsigned int index = 0;
    while (index < sorted.size())
    {
        while (index < sorted.size() &&
               (unsigned int)sorted[index].mKpScore + currentScore > playerCount)
            ++index;
        if (index == sorted.size()) break;
        unsigned int end = index + 1;
        while (end < sorted.size() && sorted[end].mKpScore == sorted[index].mKpScore)
            ++end;
        unsigned int selected = index + (unsigned int)random->nextInt((int)(end - index));
        output->push_back(sorted[selected].mItem->copy());
        currentScore += (unsigned int)sorted[selected].mKpScore;
    }
}

void GameModeUtilities::GenerateContainerContentsRandom(std::vector<not_null_ptr<ItemInstance> >* output,
                                                        const std::vector<MiniGameDef::ChestItemsInfo>* items,
                                                        Random* random, unsigned int playerCount, unsigned int score)
{
    if (output == NULL || items == NULL || items->empty()) return;
    unsigned int total = score;
    while (total < playerCount)
    {
        const MiniGameDef::ChestItemsInfo& item = items->at((unsigned int)random->nextInt((int)items->size()));
        output->push_back(item.mItem->copy());
        total += (unsigned int)item.mKpScore;
    }
}

bool GameModeUtilities::DistributeItemsKitsAndKP(const std::vector<boost::shared_ptr<Container> >* containers,
                                                 const std::vector<MiniGameDef::ChestItemsInfo>* items,
                                                 const std::vector<MiniGameDef::ChestKitInfo>* kits,
                                                 unsigned int playerCount, bool useKits)
{
    if (containers == NULL || containers->empty()) return false;
    std::vector<not_null_ptr<ItemInstance> > contents;
    Random random;
    if (useKits && kits != NULL && !kits->empty())
    {
        int kit = SelectRandomKit(kits, NULL, &random);
        if (kit >= 0) contents = kits->at((unsigned int)kit).mItems;
    }
    if (contents.empty()) GenerateContainerContentsGreedy(&contents, items, &random, playerCount, 0);
    return DistributeItemsSpread(containers, &contents);
}

bool GameModeUtilities::DistributeItemsKitsFixed(const std::vector<boost::shared_ptr<Container> >* containers,
                                                 const std::vector<MiniGameDef::ChestKitInfo>* kits)
{
    if (containers == NULL || kits == NULL || kits->empty()) return false;
    for (unsigned int i = 0; i < containers->size(); ++i)
        for (unsigned int j = 0; j < kits->at(i % kits->size()).mItems.size(); ++j)
            AddItemRuleDefinition::addItemToContainer(containers->at(i), kits->at(i % kits->size()).mItems[j], -1, false);
    return true;
}

bool GameModeUtilities::DistributeItemsWeightedRandom(const std::vector<boost::shared_ptr<Container> >* containers,
                                                      arrayWithLength<WeighedTreasure*> treasures, unsigned int score)
{
    if (containers == NULL || treasures.data == NULL) return false;
    Random random;
    for (unsigned int i = 0; i < containers->size(); ++i)
        WeighedTreasure::addChestItems(&random, treasures, containers->at(i), (int)score, true);
    return true;
}

void GameModeUtilities::SpawnWithinArea(boost::shared_ptr<ServerPlayer> player, AABB* area)
{
    if (player.get() == NULL || area == NULL) return;
    area->correctMinMax();
}

void GameModeUtilities::DistributePlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                          std::vector<GameRuleDefinition*>* rules,
                                          arrayWithLength<unsigned char>* data)
{
    if (players == NULL || rules == NULL || data == NULL) return;
    if (data->length == 0) data->resize((unsigned int)players->size());
}

std::vector<GameModeUtilities::Position*>* GameModeUtilities::createInitialPositions(Random* random, int count,
                                                                                     double minX, double minZ,
                                                                                     double maxX, double maxZ)
{
    std::vector<Position*>* positions = new std::vector<Position*>();
    for (int i = 0; i < count; ++i)
    {
        Position* position = new Position();
        position->randomize(random, minX, minZ, maxX, maxZ);
        positions->push_back(position);
    }
    return positions;
}

int GameModeUtilities::getNumberOfTeams(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    return 0;
}

bool GameModeUtilities::spreadPositions(double distance, Level* level, Random* random, const AABB* area,
                                        std::vector<Position*>* positions, bool teams)
{
    if (positions == NULL || area == NULL) return false;
    for (unsigned int i = 0; i < positions->size(); ++i)
    {
        Position* position = positions->at(i);
        position->randomize(random, area->mMinX, area->mMinZ, area->mMaxX, area->mMaxZ);
        if (level != NULL && !position->isSafe(level)) return false;
        for (unsigned int j = 0; j < i; ++j)
            if (position->x - positions->at(j)->x < distance && position->z - positions->at(j)->z < distance) return false;
    }
    return true;
}

double GameModeUtilities::setPlayerPositions(std::vector<boost::shared_ptr<ServerPlayer> >* players, Level* level,
                                              std::vector<Position*>* positions, bool, TumbleGenerator*)
{
    if (players == NULL || positions == NULL || level == NULL) return 0.0;
    double minimum = 1.7976931348623157e+308;
    for (unsigned int i = 0; i < players->size() && i < positions->size(); ++i)
    {
        if (positions->at(i)->isSafe(level))
        {
            double distance = positions->at(i)->x * positions->at(i)->x + positions->at(i)->z * positions->at(i)->z;
            if (distance < minimum) minimum = distance;
        }
    }
    return minimum;
}

void GameModeUtilities::SpreadPlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players, const AABB* area,
                                      double distance, Level* level, bool teams, TumbleGenerator* generator)
{
    if (players == NULL || area == NULL) return;
    Random random;
    unsigned int count = teams ? (unsigned int)getNumberOfTeams(players) : (unsigned int)players->size();
    std::vector<Position*>* positions = createInitialPositions(&random, (int)count, area->mMinX, area->mMinZ, area->mMaxX, area->mMaxZ);
    spreadPositions(distance, level, &random, area, positions, teams);
    setPlayerPositions(players, level, positions, teams, generator);
    for (unsigned int i = 0; i < positions->size(); ++i)
        delete positions->at(i);
    delete positions;
}

void GameModeUtilities::DistributePlayersInTeams(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                                 Scoreboard* scoreboard)
{
    if (players == NULL || scoreboard == NULL) return;
    std::vector<PlayerTeam*>* teams = scoreboard->getPlayerTeams();
    if (teams == NULL || teams->empty()) return;
    for (unsigned int i = 0; i < players->size(); ++i)
    {
        PlayerTeam* team = teams->at(i % teams->size());
        if (team != NULL)
            team->getPlayerCount();
    }
}
