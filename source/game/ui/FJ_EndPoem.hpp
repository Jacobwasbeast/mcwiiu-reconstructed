#ifndef GAME_UI_FJ_ENDPOEM_HPP
#define GAME_UI_FJ_ENDPOEM_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_EndPoem : public FJ_Document
{
private:
    FJ_FuiNode* tilePrototype;
    bool scrollDirection;
    bool waitingForLabel;
    int tileCount;
    int tileColumns;
    int tileRows;
    int currentLabel;
    int scrollState;
    std::vector<FJ_FuiNode*> textNodes;
    std::vector<int> labelIndices;
    bool hasFinished;

public:
    explicit FJ_EndPoem(fuiRenderNode* renderNode);
    virtual ~FJ_EndPoem();
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void RequestNextLabel(int label);
    void UpdateTileBackground();
    void UpdateScrolling(FJ_Event* event);
    static void _UpdateScrolling(FJ_FuiNode* node, FJ_Event* event);
    virtual void keyUpHandler(FJ_Event* event);
    static void _keyUpHandler(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    unsigned int SetNextLabel(const std::wstring& label, int index, bool append);
    virtual void keyDownHandler(FJ_Event* event);
};

#endif
