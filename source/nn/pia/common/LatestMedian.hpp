#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_LATEST_MEDIAN_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_LATEST_MEDIAN_HPP

namespace nn { namespace pia { namespace common {

template <int Capacity>
class LatestMedian
{
public:
    int GetMedian() const;

    unsigned int m_count;
    unsigned int m_index;
    int m_values[9];
};

template <int Capacity>
int LatestMedian<Capacity>::GetMedian() const
{
    unsigned int count = m_count;
    if (count == 0)
        return 0;
    if (count > 9)
        count = 9;

    int values[9];
    for (unsigned int index = 0; index < count; ++index)
    {
        const unsigned int source = (m_index + m_count - index - 1) % 9;
        values[index] = m_values[source];
    }
    for (unsigned int index = 1; index < count; ++index)
    {
        const int value = values[index];
        unsigned int position = index;
        while (position != 0 && value < values[position - 1])
        {
            values[position] = values[position - 1];
            --position;
        }
        values[position] = value;
    }

    const unsigned int middle = count / 2;
    if ((count & 1) != 0)
        return values[middle];
    return (values[middle - 1] + values[middle]) / 2;
}

}}}

#endif
