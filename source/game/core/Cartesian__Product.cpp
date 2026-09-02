#include "game/core/Cartesian__Product.hpp"

namespace Cartesian
{

Product::~Product()
{
}

Product::ProductIterator::ProductIterator(std::vector<const std::vector<Boxed*>*>* iterables)
{
    mIndex = -2;
    mIterables = iterables;
    mIterators = arrayWithLength<std::vector<Boxed*>::const_iterator>(iterables->size(), true);

    for (unsigned int i = 0; i < mIterables->size(); i++)
    {
        mIterators.data[i] = mIterables->at(i)->begin();
    }

    mResults = arrayWithLength<Boxed*>(mIterators.length, true);
}

Product::ProductIterator::~ProductIterator()
{
    delete[] mResults.data;
    delete[] mIterators.data;
}

bool Product::ProductIterator::hasNext()
{
    if (mIndex == -2)
    {
        mIndex = 0;

        for (unsigned int i = 0; i < mIterators.length; i++)
        {
            if (mIterators.data[i] == mIterables->at(i)->end())
            {
                mIndex = -1;
                break;
            }
        }

        return true;
    }

    if (mIndex >= (int)mIterators.length)
    {
        for (mIndex = (int)mIterators.length - 1; mIndex >= 0; mIndex--)
        {
            if (mIterators.data[mIndex] != mIterables->at(mIndex)->end())
            {
                break;
            }

            if (mIndex == 0)
            {
                mIndex = -1;
                return false;
            }

            mIterators.data[mIndex] = mIterables->at(mIndex)->begin();

            if (mIterators.data[mIndex] == mIterables->at(mIndex)->end())
            {
                mIndex = -1;
                return false;
            }
        }
    }

    return mIndex >= 0;
}

arrayWithLength<Boxed*> Product::ProductIterator::next()
{
    int i;

    for (i = mIndex; i < (int)mIterators.length; i++)
    {
        mResults.data[i] = *mIterators.data[i]++;
    }

    mIndex = i;

    return mResults;
}

}
