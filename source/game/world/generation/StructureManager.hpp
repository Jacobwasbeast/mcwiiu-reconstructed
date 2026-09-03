#ifndef GAME_WORLD_GENERATION_STRUCTUREMANAGER_HPP
#define GAME_WORLD_GENERATION_STRUCTUREMANAGER_HPP

#include <string>

#include <boost/unordered_map.hpp>

class ByteArrayInputStream;
class DataFixerUpper;
class StructureTemplate;

enum eMinecraftStructureTemplate
{
    eMinecraftStructureTemplate_Igloo = 0,
    eMinecraftStructureTemplate_EndCity,
    eMinecraftStructureTemplate_Shipwreck,
    eMinecraftStructureTemplate_OceanRuin,
    eMinecraftStructureTemplate_Village,
    eMinecraftStructureTemplate_PillagerOutpost,
    eMinecraftStructureTemplate_WoodlandMansion,
    eMinecraftStructureTemplate_Monument,
    eMinecraftStructureTemplate_BuriedTreasure,
    eMinecraftStructureTemplate_DesertPyramid,
    eMinecraftStructureTemplate_JungleTemple,
    eMinecraftStructureTemplate_SwampHut
};

class StructureManager : public boost::unordered_map<int, const StructureTemplate*>
{
private:
    DataFixerUpper* fixerUpper;

public:
    explicit StructureManager(DataFixerUpper* fixer);
    ~StructureManager();
    static void staticCtor();
    std::wstring fjGetStructureTemplatePath(eMinecraftStructureTemplate type) const;
    void readStructure(eMinecraftStructureTemplate type, ByteArrayInputStream* input);
    bool load(eMinecraftStructureTemplate type);
    const StructureTemplate* get(eMinecraftStructureTemplate type);
    const StructureTemplate* getOrCreate(eMinecraftStructureTemplate type);
};

#endif
