#ifndef GAME_UI_FJ_INVENTORYMENU_HPP
#define GAME_UI_FJ_INVENTORYMENU_HPP

#include <string>

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class FJ_InventoryMenu : public FJ_AbstractContainerDocument
{
public:
    explicit FJ_InventoryMenu(fuiRenderNode* renderNode);
    virtual ~FJ_InventoryMenu();

    static FJ_AbstractContainerDocument* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void InitEffectsGroup();
    virtual void init(FJ_Event* event);
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void RepositionEffects();
    virtual void HideInventory();
    void SetClassicCrafting(bool classicCrafting);
    void AddEffect(int effect, const std::wstring& name, int duration);
    unsigned int UpdateEffects(int count, int* effects);
    void ShowArmorSlot(int slot, bool visible);

private:
    unsigned int m_layoutPrefix;
    FJ_FuiNode* m_armorSlots[4];
    FJ_FuiNode* m_effectNodes[27];
    unsigned char m_effectFlags[27];
    unsigned char m_effectState[29];
    int m_effectStartY;
    int m_effectEndY;
    int m_effectBottomY;
    unsigned char m_inventoryHidden;
};

#endif
