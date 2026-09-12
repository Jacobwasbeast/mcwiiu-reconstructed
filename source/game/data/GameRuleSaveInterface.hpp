#ifndef GAME_DATA_GAMERULESAVEINTERFACE_HPP
#define GAME_DATA_GAMERULESAVEINTERFACE_HPP

#include <deque>
#include <string>

class ConsoleGameRules
{
public:
    // Fixed underlying type, not invented enumerators. These enums are declared
    // empty, so their value range is [0,1] and every
    // static_cast<EGameRuleType>(0x11) in the tree is out of range -- which GCC
    // tolerates but clang rejects outright in a case label. 102 distinct values
    // are cast to EGameRuleType across source/, and naming all 102 would be
    // fabricating 102 facts. Declaring the underlying type asserts nothing about
    // names and makes the existing casts well-formed. The real enumerators still
    // need recovering from evidence.
    enum EGameRuleType : int
    {
    };

    enum EGameRuleAttr : int
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
