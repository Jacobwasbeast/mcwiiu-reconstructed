#include "game/ui/UIScene_QuadrantSignin.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_QuadrantSignin.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

namespace
{
int s_avatarImageCount = 0;

struct AvatarRequest
{
    UIScene_QuadrantSignin* scene;
    int quadrant;
};

FJ_QuadrantSignin* GetQuadrantDocument(fuiRenderNode* root)
{
    return static_cast<FJ_QuadrantSignin*>(root->getFuiNode()->asFJ_Document());
}
}

UIScene_QuadrantSignin::UIScene_QuadrantSignin(int playerId, void* data, UILayer* layer)
    : UIScene(playerId, layer), m_signInPending(false)
{
    initialiseMovie();
    m_signInInfo = *static_cast<SignInInfo*>(data);
    m_signInPending = false;
    _initQuadrants();
    if (g_pInput->IsCircleCrossSwapped())
    {
        GetQuadrantDocument(m_movie->getRootNode())->SetABSwap(true);
        gApp.SetDRCIconEnabled(false);
        layer->addComponent(playerId, EUIScene_MenuBackground, NULL);
    }
}

UIScene_QuadrantSignin::~UIScene_QuadrantSignin()
{
    m_layer->removeComponent(EUIScene_MenuBackground);
}

unsigned int UIScene_QuadrantSignin::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    static const char* controllers[4] = { "Controller1", "Controller2", "Controller3", "Controller4" };
    static const char* press[4] = { "PressLabel", "PressLabel", "PressLabel", "PressLabel" };
    static const char* gamerTags[4] = { "GamerTag", "GamerTag", "GamerTag", "GamerTag" };
    static const char* accountTypes[4] = { "AccountType", "AccountType", "AccountType", "AccountType" };
    static const char* playerNumbers[4] = { "PlayerNumber", "PlayerNumber", "PlayerNumber", "PlayerNumber" };
    static const char* connectLabels[4] = { "ConnectControllerLabel", "ConnectControllerLabel", "ConnectControllerLabel", "ConnectControllerLabel" };
    for (int i = 0; i < 4; ++i)
    {
        m_controllerControls[i].setupControl(this, root, std::string(controllers[i]));
        m_pressLabels[i].setupControl(this, m_controllerControls[i].getFuiRenderNode(), std::string(press[i]));
        m_gamerTagLabels[i].setupControl(this, m_controllerControls[i].getFuiRenderNode(), std::string(gamerTags[i]));
        m_accountTypeLabels[i].setupControl(this, m_controllerControls[i].getFuiRenderNode(), std::string(accountTypes[i]));
        m_playerNumberLabels[i].setupControl(this, m_controllerControls[i].getFuiRenderNode(), std::string(playerNumbers[i]));
        m_playerPictures[i].setupControl(this, m_controllerControls[i].getFuiRenderNode(), std::string("PlayerPic"));
        m_connectControllerLabels[i].setupControl(this, m_controllerControls[i].getFuiRenderNode(), std::string(connectLabels[i]));
        m_controls.push_back(&m_controllerControls[i]);
        m_controls.push_back(&m_pressLabels[i]);
        m_controls.push_back(&m_gamerTagLabels[i]);
        m_controls.push_back(&m_accountTypeLabels[i]);
        m_controls.push_back(&m_playerNumberLabels[i]);
        m_controls.push_back(&m_playerPictures[i]);
        m_controls.push_back(&m_connectControllerLabels[i]);
    }
    return 1;
}

void UIScene_QuadrantSignin::setControllerState(int controller, EControllerStatus status)
{
    if (m_controllerStates[controller] == status)
        return;
    m_controllerStates[controller] = status;
    if (g_pInput->IsPadDRCController(controller))
    {
        GetQuadrantDocument(m_movie->getRootNode())->SetControllerStatus(controller, status, 0);
        return;
    }
    int controllerType = g_pInput->IsPadClassicController(controller) ? 2 : 1;
    GetQuadrantDocument(m_movie->getRootNode())->SetControllerStatus(controller, status, controllerType);
}

void UIScene_QuadrantSignin::_initQuadrants()
{
    for (int i = 0; i < 4; ++i)
    {
        m_avatarPending[i] = false;
        m_pressLabels[i].init(UIString(0x14b60640));
        m_connectControllerLabels[i].init(UIString(L""));
        m_accountTypeLabels[i].init(UIString(L""));
        m_controllerStates[i] = eControllerStatus_Disconnected;
        if (gProfile->IsSignedIn(i))
        {
            setControllerState(i, eControllerStatus_SignedIn);
            m_gamerTagLabels[i].init(UIString(gProfile->GetMiiName(i)));
        }
        else if (g_pInput->IsPadConnected(i))
        {
            setControllerState(i, eControllerStatus_Connected);
            m_gamerTagLabels[i].init(UIString(L""));
        }
        else
        {
            setControllerState(i, eControllerStatus_Disconnected);
        }
    }
}

void UIScene_QuadrantSignin::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_CONTINUE, IDS_TOOLTIPS_CANCEL, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

bool UIScene_QuadrantSignin::hasFocus(int)
{
    return m_hasFocus;
}

bool UIScene_QuadrantSignin::hidesLowerScenes()
{
    return false;
}

int UIScene_QuadrantSignin::AvatarReturned(void* context, unsigned char* data, unsigned int size)
{
    AvatarRequest* request = static_cast<AvatarRequest*>(context);
    if (data == NULL && size != 0)
    {
        ++s_avatarImageCount;
        wchar_t name[32];
        std::swprintf(name, 32, L"quadrantImage_%05d", s_avatarImageCount);
        request->scene->registerSubstitutionTexture(std::wstring(name), data, size, false);
        request->scene->m_playerPictures[request->quadrant].setTextureName(std::wstring(name), true);
    }
    delete request;
    return 0;
}

void UIScene_QuadrantSignin::updateState()
{
    for (int i = 0; i < 4; ++i)
    {
        if (!gProfile->IsSignedIn(i) || !g_pInput->IsPadConnected(i) || gProfile->IsSigningIn(i))
        {
            if (!g_pInput->IsPadConnected(i))
            {
                setControllerState(i, eControllerStatus_Disconnected);
            }
            else
            {
                setControllerState(i, eControllerStatus_Connected);
                m_gamerTagLabels[i].setLabel(UIString(L""), false, false);
            }
            m_avatarPending[i] = false;
            continue;
        }

        setControllerState(i, eControllerStatus_SignedIn);
        m_gamerTagLabels[i].setLabel(UIString(gProfile->GetMiiName(i)), false, false);
        if (!m_avatarPending[i])
        {
            AvatarRequest* request = new AvatarRequest;
            request->scene = this;
            request->quadrant = i;
            if (gProfile->GetProfileAvatar(i, AvatarReturned, request))
                m_avatarPending[i] = true;
        }
    }
}

void UIScene_QuadrantSignin::tick()
{
    if (m_movie != NULL)
    {
        UIScene::tick();
        updateState();
    }
}

std::wstring UIScene_QuadrantSignin::getMoviePath()
{
    return L"QuadrantSignin";
}

int UIScene_QuadrantSignin::SignInReturned(void* context, bool, int, int)
{
    UIScene_QuadrantSignin* scene = static_cast<UIScene_QuadrantSignin*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (scene != NULL)
    {
        scene->m_signInPending = false;
        scene->updateState();
    }
    return 0;
}

void UIScene_QuadrantSignin::handleInput(int playerId, int action, bool repeat, bool released,
                                         bool pressed, bool& handled)
{
    handled = true;
    if (m_signInPending)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
        if (pressed)
            sendInputToMovie(action, repeat, released, pressed);
        break;
    case EControllerActions_MenuOk:
        if (!pressed)
            break;
        m_signInPending = true;
        if (gProfile->IsSignedIn(playerId))
        {
            gProfile->CancelProfileAvatarRequest();
            navigateBack();
            m_signInInfo.callback(m_signInInfo.context, true, m_playerId);
            break;
        }
        gProfile->RequestSignInUI(false, false, false, false, true, SignInReturned,
                                  reinterpret_cast<void*>(GetCallbackUniqueId()), playerId);
        break;
    case EControllerActions_MenuCancel:
        if (!pressed)
            break;
        if (playerId == gProfile->GetPrimaryPad())
        {
            m_signInPending = true;
            m_signInInfo.callback(m_signInInfo.context, false, playerId);
            gProfile->CancelProfileAvatarRequest();
            if (m_signInInfo.returnToHome)
            {
                gConsoleUIController.NavigateToHomeMenu(false);
                break;
            }
            gApp.SetDRCIconEnabled(true);
            navigateBack();
            break;
        }
        if (gProfile->IsSignedIn(playerId))
            gProfile->SignOutUser(playerId);
        break;
    }
}

void UIScene_QuadrantSignin::handleReload()
{
    _initQuadrants();
}

int UIScene_QuadrantSignin::getSceneType()
{
    return EUIScene_QuadrantSignin;
}
