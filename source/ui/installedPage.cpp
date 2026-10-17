#include <algorithm>
#include <cstdio>
#include <cstring>
#include <strings.h>
#include <filesystem>
#include <memory>
#include "ui/MainApplication.hpp"
#include "ui/installedPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    static std::string FormatSize(u64 bytes) {
        char buf[32];
        if (bytes == 0) return "archived";
        if (bytes >= (1ull << 30)) std::snprintf(buf, sizeof(buf), "%.2f GB", bytes / 1073741824.0);
        else std::snprintf(buf, sizeof(buf), "%.0f MB", bytes / 1048576.0);
        return buf;
    }

    installedPage::installedPage() : Layout::Layout() {
        if (inst::config::oledMode) {
            this->SetBackgroundColor(COLOR("#000000FF"));
        } else {
            this->SetBackgroundColor(COLOR("#0B0F1AFF"));
            if (std::filesystem::exists(inst::config::appDir + "/background.png")) this->SetBackgroundImage(inst::config::appDir + "/background.png");
            else this->SetBackgroundImage("romfs:/images/background.jpg");
        }
        const auto topColor = inst::config::oledMode ? COLOR("#000000FF") : COLOR("#141A2EFF");
        const auto infoColor = inst::config::oledMode ? COLOR("#000000FF") : COLOR("#141A2E80");
        this->topRect = Rectangle::New(0, 0, 1280, 74, topColor);
        this->infoRect = Rectangle::New(0, 75, 1280, 60, infoColor);
        this->botRect = Rectangle::New(0, 660, 1280, 60, infoColor);
        this->titleImage = Image::New(0, -8, "romfs:/images/logo.png");
        this->appVersionText = TextBlock::New(300, 29, "v" + inst::config::appVersion, 22);
        this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
        this->timeText = TextBlock::New(0, 18, "--:--", 22);
        this->timeText->SetColor(COLOR("#FFFFFFFF"));
        this->ipText = TextBlock::New(0, 26, "IP: --", 16);
        this->ipText->SetColor(COLOR("#FFFFFFFF"));
        this->sysLabelText = TextBlock::New(0, 6, "System Memory", 16);
        this->sysLabelText->SetColor(COLOR("#FFFFFFFF"));
        this->sysFreeText = TextBlock::New(0, 42, "Free --", 16);
        this->sysFreeText->SetColor(COLOR("#FFFFFFFF"));
        this->sdLabelText = TextBlock::New(0, 6, "microSD Card", 16);
        this->sdLabelText->SetColor(COLOR("#FFFFFFFF"));
        this->sdFreeText = TextBlock::New(0, 42, "Free --", 16);
        this->sdFreeText->SetColor(COLOR("#FFFFFFFF"));
        this->sysBarBack = Rectangle::New(0, 30, 180, 6, COLOR("#FFFFFF33"));
        this->sysBarFill = Rectangle::New(0, 30, 0, 6, COLOR("#7B61FFFF"));
        this->sdBarBack = Rectangle::New(0, 30, 180, 6, COLOR("#FFFFFF33"));
        this->sdBarFill = Rectangle::New(0, 30, 0, 6, COLOR("#7B61FFFF"));
        this->netIndicator = Rectangle::New(0, 0, 6, 6, COLOR("#FF3B30FF"), 3);
        this->wifiBar1 = Rectangle::New(0, 0, 4, 4, COLOR("#FFFFFF55"));
        this->wifiBar2 = Rectangle::New(0, 0, 4, 7, COLOR("#FFFFFF55"));
        this->wifiBar3 = Rectangle::New(0, 0, 4, 10, COLOR("#FFFFFF55"));
        this->batteryOutline = Rectangle::New(0, 0, 24, 12, COLOR("#FFFFFF66"));
        this->batteryFill = Rectangle::New(0, 0, 0, 10, COLOR("#4CD964FF"));
        this->batteryCap = Rectangle::New(0, 0, 3, 6, COLOR("#FFFFFF66"));
        this->pageInfoText = TextBlock::New(10, 89, "Installed games", 30);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 678, " Details      Refresh      Back", 20);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 136, 1280, COLOR("#FFFFFF00"), 50, 10);
        this->menu->SetOnFocusColor(inst::config::oledMode ? COLOR("#FFFFFF33") : COLOR("#7B61FF55"));
        this->menu->SetScrollbarColor(COLOR("#141A2E80"));
        for (auto e : std::initializer_list<pu::ui::elm::Element::Ref>{
                 this->topRect, this->infoRect, this->botRect, this->titleImage, this->appVersionText,
                 this->sysBarBack, this->sysBarFill, this->sdBarBack, this->sdBarFill,
                 this->sysLabelText, this->sysFreeText, this->sdLabelText, this->sdFreeText,
                 this->netIndicator, this->wifiBar1, this->wifiBar2, this->wifiBar3,
                 this->batteryOutline, this->batteryFill, this->batteryCap,
                 this->timeText, this->ipText, this->butText, this->pageInfoText, this->menu})
            this->Add(e);
    }

    void installedPage::open() {
        this->refresh();
        mainApp->LoadLayout(mainApp->installedpage);
    }

    // ponytail: reads every title's control data on open; cache by id if big libraries feel slow.
    void installedPage::refresh() {
        this->titles.clear();
        this->menu->ClearItems();
        if (R_SUCCEEDED(nsInitialize())) {
            auto control = std::make_unique<NsApplicationControlData>();
            NsApplicationRecord records[64];
            s32 offset = 0, count = 0;
            while (R_SUCCEEDED(nsListApplicationRecord(records, 64, offset, &count)) && count > 0) {
                for (s32 i = 0; i < count; i++) {
                    Title t;
                    t.id = records[i].application_id;
                    size_t read = 0;
                    NacpLanguageEntry* lang = nullptr;
                    if (R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, t.id, control.get(), sizeof(NsApplicationControlData), &read)) &&
                        R_SUCCEEDED(nacpGetLanguageEntry(&control->nacp, &lang)) && lang) {
                        t.name = lang->name;
                        t.version = control->nacp.display_version;
                    }
                    if (t.name.empty()) {
                        char hex[17];
                        std::snprintf(hex, sizeof(hex), "%016lX", t.id);
                        t.name = hex;
                    }
                    NsApplicationOccupiedSize occ{};
                    if (R_SUCCEEDED(nsCalculateApplicationOccupiedSize(t.id, &occ)))
                        for (auto& l : occ.layout) t.size += l.sizeApplication + l.sizePatch + l.sizeAddOnContent;
                    this->titles.push_back(t);
                }
                offset += count;
            }
            nsExit();
        }
        std::sort(this->titles.begin(), this->titles.end(), [](const Title& a, const Title& b) { return strcasecmp(a.name.c_str(), b.name.c_str()) < 0; });

        for (auto& t : this->titles) {
            std::string label = inst::util::shortenString(t.name, 56, true);
            if (!t.version.empty()) label += "   v" + t.version;
            label += "   " + FormatSize(t.size);
            auto item = pu::ui::elm::MenuItem::New(label);
            item->SetColor(COLOR("#FFFFFFFF"));
            item->SetIcon("romfs:/images/icons/apps.png");
            this->menu->AddItem(item);
        }
        this->pageInfoText->SetText("Installed games (" + std::to_string(this->titles.size()) + ")");
        if (!this->titles.empty()) this->menu->SetSelectedIndex(0);
    }

    void installedPage::showSelected() {
        const int idx = this->menu->GetSelectedIndex();
        if (idx < 0 || idx >= static_cast<int>(this->titles.size())) return;
        const Title t = this->titles[idx];
        char id[17];
        std::snprintf(id, sizeof(id), "%016lX", t.id);
        const std::string desc = "Title ID: " + std::string(id) +
            "\nVersion: " + (t.version.empty() ? "unknown" : t.version) +
            "\nSize: " + FormatSize(t.size);
        if (mainApp->CreateShowDialog(t.name, desc, {"Delete", "common.cancel"_lang}, true) != 0) return;
        if (mainApp->CreateShowDialog("Delete " + t.name + "?", "This removes the game, its update and DLC.\nSave data is kept.", {"Delete", "common.cancel"_lang}, false) != 0) return;

        Result rc = nsInitialize();
        if (R_SUCCEEDED(rc)) {
            rc = nsDeleteApplicationCompletely(t.id);
            nsExit();
        }
        if (R_FAILED(rc)) {
            char code[16];
            std::snprintf(code, sizeof(code), "0x%08X", rc);
            mainApp->CreateShowDialog("Delete failed", "Error " + std::string(code), {"common.ok"_lang}, true);
        }
        this->refresh();
        if (idx < static_cast<int>(this->titles.size())) this->menu->SetSelectedIndex(idx);
    }

    void installedPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        inst::util::playNavigationClickIfNeeded(Down);
        if (Down & HidNpadButton_B) mainApp->LoadLayout(mainApp->mainPage);
        else if (Down & HidNpadButton_A) this->showSelected();
        else if (Down & HidNpadButton_Y) this->refresh();
    }
}
