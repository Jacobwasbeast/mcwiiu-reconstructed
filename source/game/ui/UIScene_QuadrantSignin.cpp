#include "game/ui/UIScene_QuadrantSignin.hpp"

#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_QuadrantSignin.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

namespace
{
unsigned int s_avatarName = 0;

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

UIScene_QuadrantSignin::UIScene_QuadrantSignin(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_signInPending(false), m_callback(NULL), m_callbackData(NULL),
      m_callbackUserIndex(0), m_returnToHome(false)
{
    initialiseMovie();
    int* values = static_cast<int*>(data);
    if (values != NULL)
    {
        m_callback = (SignInCallback)(unsigned int)values[0];
        m_callbackData = (void*)(unsigned int)values[1];
        m_callbackUserIndex = values[2];
    }
    _initQuadrants();
    if (g_pInput != NULL && g_pInput->IsDRCConnected() != 0)
    {
        GetQuadrantDocument(m_movie->getRootNode())->SetABSwap(true);
        gApp.SetDRCIconEnabled(false);
        layer->addComponent(type, static_cast<EUIScene>(0x48), NULL);
    }
}

UIScene_QuadrantSignin::~UIScene_QuadrantSignin()
{
    if (m_layer != NULL)
        m_layer->removeComponent(static_cast<EUIScene>(0x48));
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
        m_connectControllerLabels[i].setupControl(this, root, std::string(connectLabels[i]));
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
    int frame = 0;
    if (g_pInput != NULL && g_pInput->IsPadConnected(controller) == 0)
        frame = g_pInput->IsPadLocked(controller) != 0 ? 2 : 1;
    GetQuadrantDocument(m_movie->getRootNode())->SetControllerStatus(controller, status, frame);
}

void UIScene_QuadrantSignin::_initQuadrants()
{
    for (int i = 0; i < 4; ++i)
    {
        m_avatarPending[i] = 0;
        m_connectControllerLabels[i].init(UIString(0x14b60640));
        m_playerNumberLabels[i].init(UIString(L""));
        m_accountTypeLabels[i].init(UIString(L""));
        if (gProfile != NULL && gProfile->IsSignedIn(i))
        {
            setControllerState(i, eControllerStatus_SignedIn);
            m_gamerTagLabels[i].init(UIString(gProfile->GetGamertag(i)));
        }
        else if (g_pInput == NULL || g_pInput->IsPadConnected(i) == 0)
            setControllerState(i, eControllerStatus_Disconnected);
        else
        {
            setControllerState(i, eControllerStatus_Connected);
            m_gamerTagLabels[i].init(UIString(L""));
        }
    }
}

void UIScene_QuadrantSignin::updateTooltips()
{
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_controlFocus), 0x8ad40344,
                                     0xe21d8023, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, false, false);
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
    if (request != NULL && data == NULL && size != 0)
    {
        wchar_t name[32];
        ++s_avatarName;
        std::swprintf(name, 32, L"quadrantImage_%05u", s_avatarName);
        std::wstring texture(name);
        request->scene->registerSubstitutionTexture(texture, NULL, size, false);
        request->scene->m_playerPictures[request->quadrant].setTextureName(texture, true);
    }
    delete request;
    return 0;
}

void UIScene_QuadrantSignin::updateState()
{
    for (int i = 0; i < 4; ++i)
    {
        bool signedIn = gProfile != NULL && gProfile->IsSignedIn(i);
        bool connected = g_pInput != NULL && g_pInput->IsPadConnected(i) != 0;
        bool live = gProfile != NULL && gProfile->IsSignedInLive(i);
        if (!signedIn || !connected || live)
        {
            if (!connected)
            {
                setControllerState(i, eControllerStatus_Disconnected);
                m_avatarPending[i] = 0;
            }
            else if (!signedIn)
            {
                setControllerState(i, eControllerStatus_Connected);
                m_gamerTagLabels[i].setLabel(UIString(L""), false, false);
                m_avatarPending[i] = 0;
            }
            else
            {
                setControllerState(i, eControllerStatus_Connected);
                m_gamerTagLabels[i].setLabel(UIString(L""), false, false);
                m_avatarPending[i] = 0;
            }
        }
        else
        {
            setControllerState(i, eControllerStatus_SignedIn);
            m_gamerTagLabels[i].setLabel(UIString(gProfile->GetGamertag(i)), false, false);
            if (m_avatarPending[i] == 0)
            {
                AvatarRequest* request = new AvatarRequest;
                request->scene = this;
                request->quadrant = i;
                if (gProfile->GetProfileAvatar(i, AvatarReturned, request) != 0)
                    m_avatarPending[i] = 1;
            }
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
    return std::wstring(L"QuadrantSignin");
}

int UIScene_QuadrantSignin::SignInReturned(void* context, bool, int, int)
{
    UIScene* scene = gConsoleUIController.GetSceneFromCallbackId((unsigned int)context);
    if (scene != NULL)
    {
        UIScene_QuadrantSignin* quadrant = static_cast<UIScene_QuadrantSignin*>(scene);
        quadrant->m_signInPending = false;
        quadrant->updateState();
    }
    return 0;
}

void UIScene_QuadrantSignin::handleInput(int playerId, int action, bool repeat, bool released,
                                         bool pressed, bool& handled)
{
    handled = true;
    if (m_signInPending)
        return;
    gConsoleUIController.AnimateKeyPress(m_controlFocus, action, repeat, released, pressed);
    if (action > 3)
    {
        if (action < 6 && released)
            sendInputToMovie(action, repeat, released, pressed);
        else if (action == 0x14 && released)
        {
            m_signInPending = true;
            if (gProfile != NULL && gProfile->IsSignedIn(playerId))
            {
                gProfile->ResetProfileProcessState();
                navigateBack();
                if (m_callback != NULL)
                    m_callback(m_callbackData, true, m_controlFocus);
            }
            else if (gProfile != NULL)
            {
                unsigned int callbackId = GetCallbackUniqueId();
                gProfile->RequestSignInUI(false, false, false, false, true,
                                          SignInReturned, (void*)callbackId, playerId);
            }
            else
                m_signInPending = false;
        }
        else if (action == 0x15 && released && g_pInput != NULL &&
                 gProfile != NULL && playerId == gProfile->GetPrimaryPad())
        {
            m_signInPending = true;
            gProfile->ResetProfileProcessState();
            if (m_returnToHome)
                gConsoleUIController.NavigateToHomeMenu(false);
            else
            {
                gApp.SetDRCIconEnabled(true);
                navigateBack();
            }
        }
    }
}

void UIScene_QuadrantSignin::handleReload()
{
    _initQuadrants();
}

int UIScene_QuadrantSignin::getSceneType()
{
    return 0x4a;
}
