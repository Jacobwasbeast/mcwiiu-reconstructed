#ifndef PLATFORM_WIIU_FFLICONTEXTSTATE_HPP
#define PLATFORM_WIIU_FFLICONTEXTSTATE_HPP

class FFLiBufferAllocator;
struct _GX2ContextState;

class FFLiContextState
{
public:
    FFLiContextState();
    ~FFLiContextState();

    static unsigned int GetBufferSize();
    static void SetDefaultState();

    void Initialize(FFLiBufferAllocator* allocator);
    void Setup();
    void SetupEx(bool profilingEnabled);
    void Bind(bool setupIfNeeded);

private:
    bool m_isSetup;
    _GX2ContextState* m_contextState;
};

#endif
