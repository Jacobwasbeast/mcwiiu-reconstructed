#ifndef GAME_CLIENT_RENDER_PROGRESSRENDERER_HPP
#define GAME_CLIENT_RENDER_PROGRESSRENDERER_HPP

#include <string>

class Minecraft;

class ProgressRenderer
{
public:
    class TextField
    {
    public:
        TextField();
        TextField(const TextField& other);

        bool active;
        int id;
        std::wstring text;
    };

    ProgressRenderer(Minecraft* minecraft);
    virtual ~ProgressRenderer();

    void internalProgressStart(const TextField& field);
    void progressStart(int id);
    void progressStartNoAbort(int id);
    void progressStartNoAbort(const std::wstring& text);
    void progressStart(const std::wstring& text);
    void progressStage(int id);
    void progressStage(const std::wstring& text);
    void progressStagePercentage(int percentage);
    int getCurrentTitle();
    std::wstring getCurrentTitleString();
    std::wstring getCurrentStatusString();
    int getCurrentPercent();
    int getCurrentStatus();

private:
    TextField mStatus;
    Minecraft* mMinecraft;
    TextField mTitle;
    unsigned long long mStartTime;
    bool mNoAbort;
    int mPercent;
};

#endif
