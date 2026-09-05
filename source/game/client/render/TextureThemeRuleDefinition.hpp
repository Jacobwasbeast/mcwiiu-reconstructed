#ifndef GAME_CLIENT_RENDER_TEXTURETHEMERULEDEFINITION_HPP
#define GAME_CLIENT_RENDER_TEXTURETHEMERULEDEFINITION_HPP

#include <string>
#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/client/render/TextureTheme.hpp"
#include "game/world/block/GenericMultiBlockThemeRuleDefinition.hpp"
#include "game/world/generation/ShaftsLayerThemeRuleDefinition.hpp"

class TextureThemeRuleDefinition : public GameRuleDefinition
{
public:
    class BlockStackGetter
    {
    public:
        virtual ~BlockStackGetter();
        virtual void getStack(TextureTheme::BlockStack& stack) = 0;
        std::vector<std::wstring> getColours() const;

    protected:
        std::wstring mColours;
    };

    class IndexedBlockDef : public GenericMultiBlockThemeRuleDefinition::BlockRuleDefinition,
                            public BlockStackGetter
    {
    public:
        virtual ~IndexedBlockDef();
        virtual void getStack(TextureTheme::BlockStack& stack);
    };

    class LayerTexture : public GameRuleDefinition
    {
    public:
        LayerTexture();
        virtual ~LayerTexture();
        virtual ConsoleGameRules::EGameRuleType getActionType();
        std::wstring mFilename;
        int mMinimumWidth;
    };

    class MaskedShaftsThemeRuleDefinition : public ShaftsLayerThemeRuleDefinition,
                                             public BlockStackGetter
    {
    public:
        virtual ~MaskedShaftsThemeRuleDefinition();
        virtual void getStack(TextureTheme::BlockStack& stack);

    private:
        int mMask;
    };

    TextureThemeRuleDefinition();
    virtual ~TextureThemeRuleDefinition();
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual TextureTheme* Generate() const;
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);

private:
    std::vector<LayerTexture*> mLayerTextures;
    std::vector<BlockStackGetter*> mBlockStackGetters;
};

#endif
