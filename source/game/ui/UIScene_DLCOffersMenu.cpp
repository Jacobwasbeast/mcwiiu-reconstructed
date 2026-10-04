#include "game/ui/UIScene_DLCOffersMenu.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/cCommerce.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/core/cProductInfo.hpp"
#include "game/ui/UIControl_List.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/StringConversion.hpp"
#include "game/client/render/Renderer.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_DLCOffersMenu::UIScene_DLCOffersMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_titleLabel(), m_priceLabel(), m_descriptionLabel(), m_banner(),
      m_timer(), m_displayUpdated(false), m_category(static_cast<int*>(data)[2]), m_selection(0),
      m_productCount(0), m_populatePending(true), m_products(NULL)
{
    initialiseMovie();
    gApp.SetLiveLinkRequired(true);
    m_isStandardDefinition = !gRenderer->IsHiDef() && !gRenderer->IsWidescreen();
    m_titleLabel.init(UIString(L10N::GetString(IDS_DOWNLOADABLE_CONTENT_OFFERS)));
    m_offerList.init(0);
    m_descriptionLabel.init(UIString(L" "));
    m_priceLabel.init(UIString(L" "));
    m_installable = false;
    m_alreadyInstalled = false;
    m_timer.setVisible(true);
    m_waitingForCatalog = true;
    m_navigateBack = false;

    switch (static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetDLCCatalogState())
    {
    case cCommerce::eDLCCatalogState_Retrieved:
        break;
    case cCommerce::eDLCCatalogState_LogInFailed:
        if (static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->AttemptLogIn() == 0)
            m_navigateBack = true;
        else
            static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->SetDLCCatalogState(cCommerce::eDLCCatalogState_Unknown);
        break;
    default:
        m_timer.setVisible(false);
        m_waitingForCatalog = false;
        break;
    }
    addTimer(0, 100);
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_DLCOffersMenu::~UIScene_DLCOffersMenu()
{
    gApp.SetLiveLinkRequired(false);
    static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->ClearProductInfo();
}

unsigned int UIScene_DLCOffersMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_offerList.setupControl(this, root, std::string("OffersList"));
    m_controls.push_back(&m_offerList);
    m_titleLabel.setupControl(this, root, std::string("OffersList_Title"));
    m_controls.push_back(&m_titleLabel);
    m_priceLabel.setupControl(this, root, std::string("PriceTag"));
    m_controls.push_back(&m_priceLabel);
    m_descriptionLabel.setupControl(this, root, std::string("HTMLSellText"));
    m_controls.push_back(&m_descriptionLabel);
    m_banner.setupControl(this, root, std::string("DLCIcon"));
    m_controls.push_back(&m_banner);
    m_timer.setupControl(this, root, std::string("Timer"));
    m_controls.push_back(&m_timer);
    return 1;
}

uint32_t UIScene_DLCOffersMenu::ExitDLCOffersMenu(void*, int, C4JStorage::EMessageResult)
{
    static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->HideStoreIcon();
    gConsoleUIController.NavigateToHomeMenu(false);
    return 0;
}

uint32_t UIScene_DLCOffersMenu::UpdateDisplay(cProductInfo*)
{
    if (static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetPurchasabilityUpdated() && static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetDLCCatalogListRetrieved() &&
        static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetDLCCatalogListInfoRetrieved() && m_productCount != 0)
    {
        static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->ClearProductInfo();
        static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetDLCProductInfo(&m_products, &m_productCount, m_category, gConsoleUIController.UsingBitmapFont());
        for (unsigned int i = 0; i < m_productCount; ++i)
            m_offerList.showTick(i, !m_products[i].CanPurchase());
    }

    if (!m_displayUpdated && static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetDLCCatalogListRetrieved() && m_productCount != 0)
    {
        m_descriptionLabel.setLabel(UIString(m_products[m_selection].GetDescription()), false, false);
        m_priceLabel.setLabel(UIString(m_products[m_selection].GetPriceString()), false, false);
        std::wstring banner = convUtf8ToWstring(m_products[m_selection].GetBanner());
        if (!hasRegisteredSubstitutionTexture(banner))
        {
            unsigned int imageSize = 0;
            void* imageData = NULL;
            bool freeImageData;
            static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance())
                ->DLC_RetrieveImageData(m_products[m_selection].GetBanner(), &imageData, &imageSize, &freeImageData);
            if (imageSize == 0)
            {
                m_banner.setTextureName(L"", true);
                updateTooltips();
            }
            else
            {
                registerSubstitutionTexture(banner, static_cast<unsigned char*>(imageData), imageSize, freeImageData);
                m_banner.setTextureName(banner, true);
                updateTooltips();
            }
        }
        else
        {
            m_banner.setTextureName(banner, true);
            updateTooltips();
        }
        return 1;
    }
    return 0;
}

void UIScene_DLCOffersMenu::tick()
{
    UIScene::tick();
    if (m_populatePending)
    {
        if (m_navigateBack)
        {
            navigateBack();
            return;
        }
        if (static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetDLCCatalogListRetrieved() && static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetDLCCatalogListInfoRetrieved() &&
            static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetDLCProductInfo(&m_products, &m_productCount, m_category, gConsoleUIController.UsingBitmapFont()))
        {
            for (unsigned int i = 0; i < m_productCount; ++i)
                m_offerList.addItem(std::string(m_products[i].GetName()), !m_products[i].CanPurchase(), i);
            m_populatePending = false;
            m_waitingForCatalog = false;
            m_offerList.setCurrentSelection(m_selection, false, true);
            m_timer.setVisible(false);
        }
    }
    if (!m_displayUpdated && static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->GetDLCCatalogListInfoRetrieved() && m_productCount != 0 &&
        m_offerList.hasFocus())
    {
        int focus = getControlChildFocus();
        if (focus >= 0 && UpdateDisplay(&m_products[focus]))
            m_displayUpdated = true;
    }
}

void UIScene_DLCOffersMenu::updateTooltips()
{
    int selectTooltip = IDS_TOOLTIPS_SELECT;
    if (m_installable)
    {
        selectTooltip = IDS_TOOLTIPS_INSTALL;
        if (m_alreadyInstalled)
            selectTooltip = IDS_TOOLTIPS_REINSTALL;
    }
    gConsoleUIController.SetTooltips(m_playerId, selectTooltip, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_DLCOffersMenu::getMoviePath()
{
    return std::wstring(L"DLCOffersMenu");
}

void UIScene_DLCOffersMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool&)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuUp:
        if (pressed && m_productCount != 0 && !m_timer.getVisible())
        {
            if (m_selection != 0)
                --m_selection;
            m_displayUpdated = false;
        }
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuDown:
        if (pressed && m_productCount != 0 && !m_timer.getVisible())
        {
            if (static_cast<unsigned int>(m_selection) < m_productCount - 1)
                ++m_selection;
            m_displayUpdated = false;
        }
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuCancel:
        if (pressed)
            navigateBack();
        break;
    }
}

void UIScene_DLCOffersMenu::handlePress(int controlId, int childId)
{
    if (controlId == 0)
        static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->PurchaseDLCByIndex(childId, m_playerId);
}

void UIScene_DLCOffersMenu::handleSelectionChanged(int)
{
}

void UIScene_DLCOffersMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0)
    {
        m_selection = childFocus;
        m_displayUpdated = false;
    }
}

void UIScene_DLCOffersMenu::handleTimerComplete(int)
{
}

void UIScene_DLCOffersMenu::HandleDLCLicenseChange()
{
    m_offerList.clearList();
    m_populatePending = true;
    m_displayUpdated = false;
}

int UIScene_DLCOffersMenu::getSceneType()
{
    return EUIScene_DLCOffersMenu;
}
