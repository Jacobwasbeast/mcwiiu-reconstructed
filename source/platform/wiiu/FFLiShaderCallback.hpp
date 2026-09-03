#ifndef SOURCE_PLATFORM_WIIU_FFLISHADERCALLBACK_HPP
#define SOURCE_PLATFORM_WIIU_FFLISHADERCALLBACK_HPP

struct FFLShaderCallback;
struct _Mat44;
struct FFLDrawParam;

class FFLiShaderCallback
{
private:
    const FFLShaderCallback* m_callback;

public:
    FFLiShaderCallback();
    ~FFLiShaderCallback();
    bool IsExist() const;
    void Set(const FFLShaderCallback* callback);
    void CallSetMatrix(const _Mat44& matrix) const;
    void CallDraw(const FFLDrawParam& drawParam) const;
    void CallSetContextState() const;
};

#endif
