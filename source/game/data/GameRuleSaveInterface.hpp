#ifndef GAME_DATA_GAMERULESAVEINTERFACE_HPP
#define GAME_DATA_GAMERULESAVEINTERFACE_HPP

#include <deque>
#include <string>

class ConsoleGameRules
{
public:
    enum EGameRuleType
    {
    };

    enum EGameRuleAttr
    {
    };
};

class GameRuleSaveInterface
{
public:
    virtual void startTag(ConsoleGameRules::EGameRuleType type);
    virtual void startAttributes(int attributeCount);
    virtual void saveAttribute(ConsoleGameRules::EGameRuleAttr attribute, const std::wstring& value);
    virtual void startChildren(int childCount);
    virtual void finishTag();

protected:
    struct Partials
    {
        Partials(ConsoleGameRules::EGameRuleType type, int depth)
            : mDepth(depth),
              mType(type),
              mAttributeCount(0),
              mAttributesSaved(0),
              mChildCount(0),
              mChildrenFinished(0),
              mAttributesStarted(false),
              mChildrenStarted(false)
        {
        }

        int mDepth;
        ConsoleGameRules::EGameRuleType mType;
        int mAttributeCount;
        int mAttributesSaved;
        int mChildCount;
        int mChildrenFinished;
        bool mAttributesStarted;
        bool mChildrenStarted;
    };

    std::deque<Partials> mPartials;
};

#endif
