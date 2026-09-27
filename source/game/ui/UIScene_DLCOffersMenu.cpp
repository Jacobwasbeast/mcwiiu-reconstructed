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

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_DLCOffersMenu::UIScene_DLCOffersMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_catalogReady(false), m_purchasabilityChanged(false),
      m_shouldNavigateBack(false), m_unknownCallbackState(false), m_offerList(),
      m_titleLabel(), m_priceLabel(), m_descriptionLabel(), m_banner(), m_timer(),
      m_category(static_cast<int*>(data)[2]), m_selection(0), m_productCount(0),
      m_initialised(true), m_products(NULL)
{
    initialiseMovie();
    g_pApp->SetLiveLinkRequired(true);
    m_titleLabel.init(UIString(IDS_DOWNLOADABLE_CONTENT_OFFERS));
    m_offerList.init(0);
    m_descriptionLabel.init(UIString(L" "));
    m_priceLabel.init(UIString(L" "));
    m_banner.setVisible(true);
    m_catalogReady = true;
    m_shouldNavigateBack = false;

    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    if (commerce->GetDLCCatalogState() != 1)
    {
        if (commerce->GetDLCCatalogState() == 2)
        {
            if (commerce->ProductListAvailable() == 0)
            {
                m_shouldNavigateBack = true;
            }
        }
        else
        {
            m_catalogReady = false;
            m_timer.setVisible(false);
        }
    }
    addTimer(0, 100);
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_DLCOffersMenu::~UIScene_DLCOffersMenu()
{
    g_pApp->SetLiveLinkRequired(false);
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    commerce->ClearProductInfo();
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
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    commerce->ClearProductInfo();
    gConsoleUIController.NavigateToHomeMenu(false);
    return 0;
}

uint32_t UIScene_DLCOffersMenu::UpdateDisplay(cProductInfo* product)
{
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    if (commerce->GetPurchasabilityUpdated() != 0)
    {
        if (commerce->ProductListAvailable() != 0 && m_productCount != 0)
        {
            commerce->ClearProductInfo();
            commerce->GetDLCProductInfo(&m_products, &m_productCount,
                                        static_cast<unsigned int>(m_category),
                                        gConsoleUIController.UsingBitmapFont());
            unsigned int index;
            for (index = 0; index < m_productCount; ++index)
                m_offerList.showTick(static_cast<int>(index), !m_products[index].CanPurchase());
        }
    }

    if (!m_purchasabilityChanged && product != NULL && commerce->ProductListAvailable() != 0)
    {
        m_descriptionLabel.setLabel(UIString(product->GetDescription()), false, false);
        m_priceLabel.setLabel(UIString(product->GetPriceString()), false, false);
        std::wstring banner = convUtf8ToWstring(product->GetBanner());
        if (!hasRegisteredSubstitutionTexture(banner))
        {
            void* imageData = NULL;
            unsigned int imageSize = 0;
            bool freeImageData = false;
            cDLCInfo* dlcInfo = static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance());
            dlcInfo->DLC_RetrieveImageData(product->GetBanner(), &imageData, &imageSize,
                                           &freeImageData);
            if (imageData == NULL)
            {
                m_banner.setTextureName(std::wstring(L"DLCOff"), true);
                gConsoleUIController.RefreshTooltips(m_playerId);
            }
            else
            {
                registerSubstitutionTexture(banner, static_cast<unsigned char*>(imageData),
                                            imageSize, freeImageData);
                m_banner.setTextureName(banner, true);
                gConsoleUIController.RefreshTooltips(m_playerId);
            }
        }
        else
        {
            m_banner.setTextureName(banner, true);
            gConsoleUIController.RefreshTooltips(m_playerId);
        }
        return 1;
    }
    return 0;
}

void UIScene_DLCOffersMenu::tick()
{
    UIScene::tick();
    if (m_catalogReady)
    {
        if (m_shouldNavigateBack)
        {
            navigateBack();
            return;
        }
        cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
        if (commerce->ProductListAvailable() != 0 &&
            commerce->GetDLCCatalogListInfoRetrieved() != 0)
        {
            if (commerce->GetDLCProductInfo(&m_products, &m_productCount,
                                            static_cast<unsigned int>(m_category),
                                            gConsoleUIController.UsingBitmapFont()) != 0)
            {
                unsigned int index;
                for (index = 0; index < m_productCount; ++index)
                {
                    m_offerList.addItem(std::string(m_products[index].GetName()),
                                        !m_products[index].CanPurchase(), static_cast<int>(index));
                }
                m_catalogReady = false;
                m_initialised = false;
                m_offerList.setCurrentSelection(m_selection, false, true);
                m_timer.setVisible(false);
            }
        }
    }
    if (!m_purchasabilityChanged)
    {
        cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
        if (commerce->ProductListAvailable() != 0 && m_productCount != 0 &&
            m_offerList.hasFocus())
        {
            int focus = static_cast<int>(getControlChildFocus());
            if (focus >= 0 && UpdateDisplay(&m_products[focus]) != 0)
                m_purchasabilityChanged = true;
        }
    }
}

void UIScene_DLCOffersMenu::updateTooltips()
{
    int tooltip = IDS_TOOLTIPS_SELECT;
    if (m_unknownCallbackState && m_shouldNavigateBack)
        tooltip = IDS_TOOLTIPS_REINSTALL;
    else if (m_unknownCallbackState)
        tooltip = IDS_TOOLTIPS_INSTALL;
    gConsoleUIController.SetTooltips(m_playerId, tooltip, IDS_TOOLTIPS_BACK, -1, -1, -1, -1,
                                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_DLCOffersMenu::getMoviePath()
{
    return std::wstring(L"DLCOffersMenu");
}

void UIScene_DLCOffersMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                        bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    if (action >= EControllerActions_MenuOtherStickUp && action != EControllerActions_MenuOk &&
        action != EControllerActions_MenuCancel)
        return;
    switch (action)
    {
    case EControllerActions_MenuUp:
        if (pressed && m_productCount != 0 && !m_timer.getVisible() && m_selection != 0)
        {
            --m_selection;
            m_purchasabilityChanged = false;
        }
        break;
    case EControllerActions_MenuDown:
        if (pressed && m_productCount != 0 && !m_timer.getVisible() &&
            static_cast<unsigned int>(m_selection) < m_productCount - 1)
        {
            ++m_selection;
            m_purchasabilityChanged = false;
        }
        break;
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            navigateBack();
            handled = true;
            return;
        }
        break;
    }
    sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_DLCOffersMenu::handlePress(int controlId, int childId)
{
    if (controlId == 0)
    {
        cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
        commerce->PurchaseDLCByIndex(static_cast<unsigned int>(childId), m_playerId);
    }
}

void UIScene_DLCOffersMenu::handleSelectionChanged(int)
{
}

void UIScene_DLCOffersMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0)
    {
        m_selection = childFocus;
        m_purchasabilityChanged = false;
    }
}

void UIScene_DLCOffersMenu::handleTimerComplete(int)
{
}

void UIScene_DLCOffersMenu::HandleDLCLicenseChange()
{
    m_offerList.clearList();
    m_catalogReady = true;
    m_purchasabilityChanged = false;
}

int UIScene_DLCOffersMenu::getSceneType()
{
    return EUIScene_DLCOffersMenu;
}
