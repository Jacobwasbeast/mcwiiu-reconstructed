#ifndef SOURCE_VENDOR_TUTORIALPOPUPINFO_HPP
#define SOURCE_VENDOR_TUTORIALPOPUPINFO_HPP

#include <boost/shared_ptr.hpp>

class UIScene;
class ItemInstance;

class _TutorialPopupInfo
{
public:
    _TutorialPopupInfo();

    UIScene* scene;
    const wchar_t* title;
    const wchar_t* description;
    boost::shared_ptr<ItemInstance> icon;
    int iconItemId;
    bool showIcon;
    bool useDescription;
    bool showTitle;
    bool showExitScreenshot;
    int descriptionId;
    int reserved;
};

#endif
