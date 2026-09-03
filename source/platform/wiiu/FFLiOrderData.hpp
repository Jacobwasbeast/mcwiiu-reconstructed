#ifndef PLATFORM_WIIU_FFLIORDERDATA_HPP
#define PLATFORM_WIIU_FFLIORDERDATA_HPP

class FFLiOrderData
{
public:
    void Init();
    void Clear();
    bool IsLinkedNext() const;
    bool IsLinkedPrev() const;
    bool IsUnlinked() const;
    unsigned short NextIndex() const;
    unsigned short PrevIndex() const;
    void SetNextIndex(unsigned short index);
    void SetPrevIndex(unsigned short index);
    void SwapEndian();

private:
    unsigned short mNextIndex;
    unsigned short mPrevIndex;
};

#endif
