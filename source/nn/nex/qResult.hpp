#ifndef SOURCE_NN_NEX_QRESULT_HPP
#define SOURCE_NN_NEX_QRESULT_HPP
#include <cstdint>
namespace nn
{
namespace nex
{
class qResult
{
public:
    qResult();
    explicit qResult(const int&);
    explicit qResult(const bool&);
    bool Equals(const int&) const;
    bool Equals(const bool&) const;
    explicit operator bool() const;
    bool operator!() const;
    qResult& operator=(const qResult&);
    qResult& operator=(const bool&);
    int GetFacility() const;

private:
    int32_t m_code;
    const wchar_t* m_file;
    int32_t m_line;
};
}
}
#endif
