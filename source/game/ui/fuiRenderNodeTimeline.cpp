#include "game/ui/fuiRenderNodeTimeline.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_DynamicHtmlText.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_Label_Blink.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/FJ_List_MenuButton.hpp"
#include "game/ui/FJ_TextInput.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiSymbol.hpp"
#include "game/ui/fuiTimeline.hpp"
#include "game/ui/fuiTimelineAction.hpp"
#include "game/ui/fuiTimelineEvent.hpp"
#include "game/ui/fuiTimelineFrame.hpp"

fuiRenderNodeTimeline::frameAction::frameAction(const frameAction& other)
    : frame(other.frame), type(other.type), value(other.value)
{
}

fuiRenderNodeTimeline::fuiRenderNodeTimeline(fuiRenderNode* stage, fuiRenderNode* parent,
                                             fuiObject* object, unsigned int symbolIndex,
                                             unsigned char depth, fuiRGBA* color,
                                             unsigned char clipDepth, bool visible, fuiFile* file)
    : fuiRenderNode(stage, parent, object, symbolIndex, eFuiObjectType_Timeline, depth, color,
                    clipDepth, visible, file), m_mask(0), m_currentFrame(-1),
      m_previousFrame(-1)
{
    m_flags |= 4;
    if (object != 0)
    {
        fuiTimeline* timeline = static_cast<fuiTimeline*>(object);
        if (timeline->bounds.xMin != 0.0f || timeline->bounds.xMax != 0.0f ||
            timeline->bounds.yMin != 0.0f || timeline->bounds.yMax != 0.0f)
            m_flags |= 0x20;
    }
}

fuiRenderNodeTimeline::~fuiRenderNodeTimeline()
{
    if (m_mask != 0)
        m_mask->shutdown();
}

void fuiRenderNodeTimeline::setPlaying(bool playing)
{
    if (playing)
    {
        m_flags |= 6;
        return;
    }
    m_flags &= ~4u;
    if ((m_flags & 0x10) == 0)
        m_previousFrame = m_currentFrame;
}

void fuiRenderNodeTimeline::setFrame(unsigned int frame)
{
    unsigned int count = 0;
    if (m_object != 0)
        count = static_cast<fuiTimeline*>(m_object)->timelineFrameCount;
    if (frame > count)
        frame = count;
    m_flags |= 0x12;
    m_previousFrame = static_cast<int>(frame) - 1;
}

void fuiRenderNodeTimeline::addFrameAction(int frame, eFrameActionType type,
                                           const std::string& value)
{
    frameAction action;
    action.frame = frame - 1;
    action.type = type;
    action.value = value;
    m_frameActions.push_back(action);
}

void fuiRenderNodeTimeline::expandBoundsFromChild(fuiRect& bounds)
{
    if (m_bounds.xMin > bounds.xMin) m_bounds.xMin = bounds.xMin;
    if (m_bounds.xMax < bounds.xMax) m_bounds.xMax = bounds.xMax;
    if (m_bounds.yMin > bounds.yMin) m_bounds.yMin = bounds.yMin;
    if (m_bounds.yMax < bounds.yMax) m_bounds.yMax = bounds.yMax;
}

void fuiRenderNodeTimeline::updateBoundsLocal()
{
    m_bounds.xMin = 0.0f;
    m_bounds.xMax = 0.0f;
    m_bounds.yMin = 0.0f;
    m_bounds.yMax = 0.0f;
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        fuiRect bounds;
        (*it)->getParentSpaceBounds(&bounds);
        expandBoundsFromChild(bounds);
    }
}

void fuiRenderNodeTimeline::progogateBoundsFromChild()
{
    updateBoundsLocal();
    if (m_parent != 0)
        m_parent->progogateBounds();
}

void fuiRenderNodeTimeline::shutdown()
{
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
        if (*it != 0) (*it)->shutdown();
}

int fuiRenderNodeTimeline::getReferencedObject(fuiObject* object)
{
    if (object == 0 || m_file == 0)
        return -1;
    return static_cast<int>(object->symbolIndex);
}

unsigned int fuiRenderNodeTimeline::createReferencedNode(fuiObject* object, unsigned int depth,
                                                         unsigned char renderFlags, fuiRGBA* color,
                                                         unsigned char clipDepth, bool visible)
{
    if (object == 0 || m_file == 0)
        return 0;
    int index = getReferencedObject(object);
    if (index < 0)
        return 0;
    if (index < 0)
        return 0;
    return static_cast<unsigned int>(m_file->createNodeFromSymbol(m_file->symbolData[index].name, this, m_file->index));
}

bool fuiRenderNodeTimeline::depthComparison(const fuiRenderNode* first, const fuiRenderNode* second)
{
    fuiRect firstBounds;
    fuiRect secondBounds;
    const_cast<fuiRenderNode*>(first)->getParentSpaceBounds(&firstBounds);
    const_cast<fuiRenderNode*>(second)->getParentSpaceBounds(&secondBounds);
    return firstBounds.xMin < secondBounds.xMin;
}

void fuiRenderNodeTimeline::depthSort()
{
    std::sort(m_children.begin(), m_children.end(), depthComparison);
}

void fuiRenderNodeTimeline::handleConstruction()
{
    FJ_FuiNode* created;
    if (static_cast<int>(m_object->symbolIndex) == -1)
    {
        created = FJ_Factory::createObject("", this, false, 0);
    }
    else
    {
        created = FJ_Factory::createObject(
            m_file->symbolData[static_cast<int>(m_object->symbolIndex)].name, this, false, 0);
    }

    fuiTimeline* timeline = static_cast<fuiTimeline*>(m_object);
    fuiTimelineAction* action = m_file->timelineActionData + timeline->timelineActionIndex;
    m_node = created;

    if (timeline->timelineActionCount == 0)
        return;

    unsigned int index = 0;
    do
    {
        unsigned char type = action->actionType;
        fuiRenderNode* target = 0;
        if (type > 0x1d)
        {
            if (type == 0x1e)
            {
                std::string value;
                value.assign("", 0);
                addFrameAction(action->frameIndex, eFrameActionType_Stop, value);
            }
            else if (type < 0x20)
            {
                std::string value;
                value.assign(action->actionValue, std::strlen(action->actionValue));
                addFrameAction(action->frameIndex, eFrameActionType_GotoAndStopLabel, value);
            }
            else if (type == 0x20)
            {
                std::string value;
                value.assign(action->actionValue, std::strlen(action->actionValue));
                addFrameAction(action->frameIndex, eFrameActionType_GotoAndPlay, value);
            }
            else if (type == 0x21)
            {
                std::string value;
                value.assign(action->actionValue, std::strlen(action->actionValue));
                addFrameAction(action->frameIndex, eFrameActionType_GotoAndStop, value);
            }
            else
            {
                target = findNode(action->targetName);
            }
        }
        else
        {
            target = findNode(action->targetName);
        }

        if (target != 0)
        {
            FJ_FuiNode* node = target->getFuiNode();
            if (node != 0)
            {
                switch (type)
                {
                case 4:
                {
                    FJ_Base* base = node->asFJ_Base();
                    if (base != 0)
                        base->setTabIndex(std::atoi(action->actionValue));
                    break;
                }
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                {
                    FJ_Label* label = node->asFJ_Label();
                    if (label == 0)
                    {
                        FJ_List_MenuButton* menuButton = node->asFJ_List_MenuButton();
                        if (menuButton == 0)
                        {
                            FJ_Label_Blink* blink = node->asFJ_Label_Blink();
                            if (blink != 0)
                                blink->setAction(type, action->actionValue);
                        }
                        else
                        {
                            menuButton->setAction(type, action->actionValue);
                        }
                    }
                    else
                    {
                        label->setAction(type, action->actionValue);
                    }
                    break;
                }
                case 10:
                case 0xb:
                case 0xc:
                case 0xd:
                case 0x1d:
                {
                    FJ_List* list = node->asFJ_List();
                    if (list != 0)
                        list->setAction(type, action->actionValue);
                    FJ_List2D* list2d = node->asFJ_List2D();
                    if (list2d != 0)
                        list2d->setAction(type, action->actionValue);
                    break;
                }
                case 0x13:
                {
                    if (node->isFJ_HtmlText())
                    {
                        FJ_HtmlText* html = node->asFJ_HtmlText();
                        html->m_bUseScrollRect = std::strcmp(action->actionValue, "true") == 0;
                    }
                    break;
                }
                case 0x14:
                {
                    if (node->isFJ_HtmlText())
                    {
                        FJ_HtmlText* html = node->asFJ_HtmlText();
                        html->m_bNoBackground = std::strcmp(action->actionValue, "true") == 0;
                    }
                    break;
                }
                case 0x15:
                {
                    if (node->isFJ_DynamicHtmlText())
                    {
                        FJ_DynamicHtmlText* dynamicText = node->asFJ_DynamicHtmlText();
                        dynamicText->m_iTopMargin = std::atoi(action->actionValue);
                    }
                    break;
                }
                case 0x16:
                {
                    if (node->isFJ_TextInput())
                    {
                        FJ_TextInput* textInput = node->asFJ_TextInput();
                        textInput->SetCharLimit(std::atoi(action->actionValue));
                    }
                    break;
                }
                case 0x17:
                {
                    if (node->isFJ_DynamicHtmlText())
                    {
                        FJ_DynamicHtmlText* dynamicText = node->asFJ_DynamicHtmlText();
                        dynamicText->m_iCharacterHeight = std::atoi(action->actionValue);
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }

        ++index;
        ++action;
    } while (index < timeline->timelineActionCount);
}

int fuiRenderNodeTimeline::getFrame(const char* name)
{
    if (m_object == 0 || m_file == 0 || name == 0)
        return 0;
    fuiTimeline* timeline = static_cast<fuiTimeline*>(m_object);
    for (unsigned int i = 0; i < timeline->timelineFrameCount; ++i)
        if (std::strcmp(m_file->frameData[timeline->timelineFrameIndex + i].name, name) == 0)
            return static_cast<int>(i + 1);
    return 0;
}

void fuiRenderNodeTimeline::setFrame(const char* name)
{
    int frame = getFrame(name);
    if (frame != 0)
        setFrame(static_cast<unsigned int>(frame));
}

void fuiRenderNodeTimeline::handleFrameTransition(int frame)
{
    for (std::vector<frameAction>::iterator it = m_frameActions.begin(); it != m_frameActions.end(); ++it)
    {
        if (it->frame != frame || m_node == 0)
            continue;
        if (it->type == eFrameActionType_Stop)
            setPlaying(false);
        else if (it->type == eFrameActionType_GotoAndStop || it->type == eFrameActionType_GotoAndStopLabel)
        {
            setFrame(it->value.c_str());
            setPlaying(false);
        }
        else if (it->type == eFrameActionType_GotoAndPlay)
        {
            setFrame(it->value.c_str());
            setPlaying(true);
        }
    }
}

void fuiRenderNodeTimeline::gather(bool force)
{
    if (m_object == 0 || m_file == 0)
        return;
    if ((m_flags & 2) != 0)
    {
        m_currentFrame = m_previousFrame;
        if (m_currentFrame < 0) m_currentFrame = 0;
        m_previousFrame = m_currentFrame;
        m_flags &= ~0x10u;
    }
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
        (*it)->gather(force);
    updateBoundsLocal();
    if ((m_flags & 1) != 0)
    {
        handleConstruction();
        m_flags &= ~1u;
    }
    if (m_currentFrame != m_previousFrame)
        handleFrameTransition(m_currentFrame);
    depthSort();
    if (force && (m_flags & 4) != 0 && static_cast<fuiTimeline*>(m_object)->timelineFrameCount != 0)
        m_previousFrame = (m_currentFrame + 1) % static_cast<fuiTimeline*>(m_object)->timelineFrameCount;
}

void fuiRenderNodeTimeline::preRender()
{
    if (!isVisible()) return;
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
        (*it)->preRender();
}

void fuiRenderNodeTimeline::render(fuiRect* bounds, fuiMatrix* matrix)
{
    if (!isVisible()) return;
    pushMatrix();
    pushColor();
    pushFilterFlags();
    setScrollRectViewport();
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
        if (*it != 0) (*it)->render(bounds, matrix);
    clearScrollRectViewport();
    popFilterFlags();
    popColor();
    popMatrix();
}

void fuiRenderNodeTimeline::handleAddedToStage()
{
    fuiRenderNode::handleAddedToStage();
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
        (*it)->handleAddedToStage();
}

fuiRenderNode* fuiRenderNodeTimeline::findNode(const char* name)
{
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        fuiRenderNode* result = (*it)->findNode(name);
        if (result != 0) return result;
    }
    return fuiRenderNode::findNode(name);
}

fuiRenderNode* fuiRenderNodeTimeline::getNodeFromPath(const char* path)
{
    fuiRenderNode* result = fuiRenderNode::getNodeFromPath(path);
    if (result != 0) return result;
    const char* separator = std::strchr(path, '.');
    if (separator == 0) return findNode(path);
    std::string first(path, separator - path);
    fuiRenderNode* child = findNode(first.c_str());
    return child == 0 ? 0 : child->getNodeFromPath(separator + 1);
}

std::string fuiRenderNodeTimeline::currentLabel()
{
    if (m_object == 0 || m_file == 0) return std::string();
    fuiTimeline* timeline = static_cast<fuiTimeline*>(m_object);
    if (m_currentFrame < 0 || static_cast<unsigned int>(m_currentFrame) >= timeline->timelineFrameCount)
        return std::string();
    return std::string(m_file->frameData[timeline->timelineFrameIndex + m_currentFrame].name);
}

int fuiRenderNodeTimeline::currentFrame()
{
    return m_currentFrame + 1;
}

void fuiRenderNodeTimeline::removeChild(fuiRenderNode* child, bool destroy)
{
    std::vector<fuiRenderNode*>::iterator it = std::find(m_children.begin(), m_children.end(), child);
    if (it == m_children.end()) return;
    m_removedChildren.push_back(fuiRenderNodeRemoved(0, 0));
    m_children.erase(it);
    if (destroy) child->shutdown();
}

void fuiRenderNodeTimeline::removeChildAt(int index, bool destroy)
{
    if (index < 0 || static_cast<unsigned int>(index) >= m_children.size()) return;
    removeChild(m_children[index], destroy);
}

void fuiRenderNodeTimeline::addMask(fuiRenderNode* mask)
{
    m_mask = mask;
}

void fuiRenderNodeTimeline::addChild(fuiRenderNode* child)
{
    if (child == 0) return;
    m_children.push_back(child);
    child->generateGlobalMatrix();
    m_flags |= 0x80;
    depthSort();
}

void fuiRenderNodeTimeline::addChildAt(fuiRenderNode* child, int index)
{
    if (child == 0) return;
    if (index < 0) index = 0;
    if (index > static_cast<int>(m_children.size())) index = static_cast<int>(m_children.size());
    m_children.insert(m_children.begin() + index, child);
    child->generateGlobalMatrix();
    m_flags |= 0x80;
    depthSort();
}

unsigned int fuiRenderNodeTimeline::getChildIndex(fuiRenderNode* child)
{
    std::vector<fuiRenderNode*>::iterator it = std::find(m_children.begin(), m_children.end(), child);
    return it == m_children.end() ? 0xffffffffu : static_cast<unsigned int>(it - m_children.begin());
}

void fuiRenderNodeTimeline::generateGlobalMatrix()
{
    fuiRenderNode::generateGlobalMatrix();
    updateBoundsTree();
}

void fuiRenderNodeTimeline::updateBoundsTree()
{
    updateBoundsLocal();
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
        (*it)->generateGlobalMatrix();
}
