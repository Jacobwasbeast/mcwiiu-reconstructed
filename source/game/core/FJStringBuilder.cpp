#include "game/core/FJStringBuilder.hpp"

FJStringBuilder::FJStringBuilder()
{
    mCore.reset();
}

FJStringBuilder::FJStringBuilder(StringBuilderCore* core)
{
    boost::shared_ptr<StringBuilderCore> temp(core);
    mCore.swap(temp);
}

std::wstring FJStringBuilder::operator()()
{
    return mCore->getString();
}
