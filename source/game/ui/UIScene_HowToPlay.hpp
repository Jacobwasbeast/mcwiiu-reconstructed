#ifndef GAME_UI_UISCENE_HOWTOPLAY_HPP
#define GAME_UI_UISCENE_HOWTOPLAY_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_DynamicLabel.hpp"
#include "game/ui/UIControl_EnchantmentBook.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/item/not_null_ptr.hpp"

class Block;
class Item;
class ItemInstance;
class Potion;
class UILayer;
struct fuiRect;

enum EHowToPlayPage
{
    EHowToPlayPage_0 = 0,
    EHowToPlayPage_1 = 1,
    EHowToPlayPage_2 = 2,
    EHowToPlayPage_3 = 3
};

class UIScene_HowToPlay : public UIScene
{
public:
    class sCustomItems
    {
    public:
        sCustomItems(Item* item, int auxValue, int count);
        sCustomItems(Block* block, int auxValue, int count);
        sCustomItems(int blockId, int auxValue, int count);
        sCustomItems(Item* item, Potion* potion, int auxValue, int count);
        not_null_ptr<ItemInstance> GetItemInstance() const;

        Block* m_block;
        Item* m_item;
        Potion* m_potion;
        int m_auxValue;
        int m_count;
    };

    UIScene_HowToPlay(int playerId, void* data, UILayer* layer);
    virtual ~UIScene_HowToPlay();

    void StartPage(EHowToPlayPage page);
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    void customDrawEnchantmentFont(fuiRect* rect, int variant);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual int getSceneType();
    virtual unsigned int getSubSceneType() const;

private:
    int m_page;
    int m_pageCount;
    UIControl m_container;
    UIControl_DynamicLabel m_dynamicHtmlText;
    UIControl_Label m_labels[30];
    UIControl_EnchantmentBook m_enchantmentBook;
    UIControl_EnchantmentBook m_enchantmentBooks[3];
};

#endif
