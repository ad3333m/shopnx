#pragma once
#include <pu/Plutonium>
#include <switch.h>

using namespace pu::ui::elm;
namespace inst::ui {
    // DBI-style "Browse installed games": list, details, delete.
    class installedPage : public pu::ui::Layout
    {
        public:
            installedPage();
            PU_SMART_CTOR(installedPage)
            void open();
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
            pu::ui::elm::Menu::Ref menu;
            TextBlock::Ref pageInfoText;
            Image::Ref titleImage;
            TextBlock::Ref appVersionText;
            TextBlock::Ref timeText;
            TextBlock::Ref ipText;
            TextBlock::Ref sysLabelText;
            TextBlock::Ref sysFreeText;
            TextBlock::Ref sdLabelText;
            TextBlock::Ref sdFreeText;
            Rectangle::Ref sysBarBack;
            Rectangle::Ref sysBarFill;
            Rectangle::Ref sdBarBack;
            Rectangle::Ref sdBarFill;
            Rectangle::Ref netIndicator;
            Rectangle::Ref wifiBar1;
            Rectangle::Ref wifiBar2;
            Rectangle::Ref wifiBar3;
            Rectangle::Ref batteryOutline;
            Rectangle::Ref batteryFill;
            Rectangle::Ref batteryCap;
        private:
            struct Title {
                u64 id = 0;
                std::string name;
                std::string version;
                u64 size = 0;
            };
            std::vector<Title> titles;
            TextBlock::Ref butText;
            Rectangle::Ref topRect;
            Rectangle::Ref infoRect;
            Rectangle::Ref botRect;
            void refresh();
            void showSelected();
    };
}
