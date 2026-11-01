#include "ui/theme.hpp"
#include <algorithm>
#include <filesystem>
#include <vector>
#include <SDL2/SDL_mixer.h>
#include "ui/MainApplication.hpp"
#include "util/config.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;
}

namespace inst::ui::theme {
    namespace {
        struct Palette {
            const char* name;
            const char* key;
            bool dark;
            u32 bg, panel, line, text, strong, muted, accent, green;
        };

        // Colours taken from Luma's five themes (Wii, Wii Black, Cafe, Pikmin, Clasico).
        const Palette kPalettes[] = {
            {"Light",   "light",   false, 0xEAECEF, 0xFFFFFF, 0xD5DAE0, 0x4A4F55, 0x33373C, 0x6B727A, 0x34BEED, 0x3FB950},
            {"Dark",    "dark",    true,  0x17191C, 0x23262A, 0x3A3F45, 0xD6DADE, 0xF1F3F5, 0x9AA1A8, 0x2A8CC4, 0x3FB950},
            {"Cafe",    "cafe",    false, 0xF3EFDF, 0xFFFDF6, 0xD8CFAE, 0x4A4A3C, 0x33332A, 0x7A7660, 0x6DBF45, 0x6DBF45},
            {"Space",   "space",   true,  0x0B1838, 0x13234A, 0x2F4F7A, 0xD8E4FF, 0xFFFFFF, 0x8FA3C7, 0x2F6BFF, 0x4CC35A},
            {"Classic", "classic", true,  0x05070D, 0x0F172A, 0x1E293B, 0xE0F2FF, 0xFFFFFF, 0x94A3B8, 0x7DD3FC, 0x22C55E},
        };
        constexpr int kCount = sizeof(kPalettes) / sizeof(kPalettes[0]);

        const Palette& Cur() {
            const int i = inst::config::lumaTheme;
            return kPalettes[(i >= 0 && i < kCount) ? i : 0];
        }

        pu::ui::Color Rgb(u32 rgb, u8 a = 0xFF) {
            return pu::ui::Color((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, a);
        }

        std::vector<std::string> gTracks;
        int gTrack = 0;
        Mix_Music* gMusic = nullptr;
        bool gPaused = false;
        bool gAudioReady = false;

        void PlayTrack(int index) {
            if (!gAudioReady || gTracks.empty())
                return;
            const int n = static_cast<int>(gTracks.size());
            gTrack = ((index % n) + n) % n;
            if (gMusic != nullptr) {
                Mix_HaltMusic();
                Mix_FreeMusic(gMusic);
            }
            gMusic = Mix_LoadMUS(gTracks[gTrack].c_str());
            if (gMusic != nullptr)
                Mix_PlayMusic(gMusic, 1); // MusicTick moves on to the next track
            gPaused = false;
        }
    }

    int Count() { return kCount; }
    const char* Name(int index) { return kPalettes[(index >= 0 && index < kCount) ? index : 0].name; }
    const char* CurrentName() { return Cur().name; }

    pu::ui::Color Map(const char* hex) {
        const std::string h = hex;
        if (inst::config::oledMode || h.size() != 9)
            return pu::ui::Color::FromHex(h);
        const std::string rgb = h.substr(1, 6);
        const u8 a = static_cast<u8>(std::stoul(h.substr(7, 2), nullptr, 16));
        const Palette& p = Cur();
        if (rgb == "0B0F1A") return Rgb(p.bg, a);
        if (rgb == "141A2E") return Rgb(p.panel);
        if (rgb == "7B61FF") return Rgb(p.accent, a);
        if (rgb == "E9FFF2") return Rgb(p.strong);
        if (rgb == "FFFFFF") {
            if (a == 0x00) return Rgb(0xFFFFFF, 0);
            if (a == 0xFF) return Rgb(p.strong);
            if (a == 0xCC) return Rgb(p.text);
            if (a >= 0x88) return Rgb(p.muted);
            if (a <= 0x33) return Rgb(p.line);
            return Rgb(p.accent, 0x55);
        }
        // Non-OLED list focus colours.
        if (rgb == "000000" && (a == 0x22 || a == 0x33 || a == 0x70))
            return Rgb(p.accent, a == 0x70 ? 0x55 : 0x33);
        return pu::ui::Color::FromHex(h);
    }

    pu::ui::Color Panel() { return Rgb(Cur().panel); }
    pu::ui::Color Line() { return Rgb(Cur().line); }
    pu::ui::Color Text() { return Rgb(Cur().strong); }
    pu::ui::Color Muted() { return Rgb(Cur().muted); }
    pu::ui::Color Accent() { return Rgb(Cur().accent); }
    pu::ui::Color Green() { return Rgb(Cur().green); }
    std::string Background() { return std::string("romfs:/images/theme_") + Cur().key + ".jpg"; }
    std::string Logo() { return Cur().dark ? "romfs:/images/logo.png" : "romfs:/images/logo_dark.png"; }

    void Pick(bool backToOptions) {
        std::vector<std::string> names;
        for (int t = 0; t < kCount; t++)
            names.push_back(kPalettes[t].name);
        const int picked = mainApp->CreateShowDialog("Theme", "Pick a look for ShopNX.", names, true);
        if (picked < 0 || picked >= kCount || picked == inst::config::lumaTheme)
            return;
        inst::config::lumaTheme = picked;
        inst::config::setConfig();

        using namespace std::placeholders;
        auto keepAlive = mainApp->GetCurrentLayout();
        mainApp->mainPage = MainPage::New();
        mainApp->instpage = instPage::New();
        mainApp->sdinstPage = sdInstPage::New();
        mainApp->netinstPage = netInstPage::New();
        mainApp->usbinstPage = usbInstPage::New();
        mainApp->hddinstPage = hddInstPage::New();
        mainApp->remoteinstPage = remoteInstPage::New();
        mainApp->optionspage = optionsPage::New();
        mainApp->installedpage = installedPage::New();
        mainApp->mainPage->SetOnInput(std::bind(&MainPage::onInput, mainApp->mainPage, _1, _2, _3, _4));
        mainApp->instpage->SetOnInput(std::bind(&instPage::onInput, mainApp->instpage, _1, _2, _3, _4));
        mainApp->sdinstPage->SetOnInput(std::bind(&sdInstPage::onInput, mainApp->sdinstPage, _1, _2, _3, _4));
        mainApp->netinstPage->SetOnInput(std::bind(&netInstPage::onInput, mainApp->netinstPage, _1, _2, _3, _4));
        mainApp->usbinstPage->SetOnInput(std::bind(&usbInstPage::onInput, mainApp->usbinstPage, _1, _2, _3, _4));
        mainApp->hddinstPage->SetOnInput(std::bind(&hddInstPage::onInput, mainApp->hddinstPage, _1, _2, _3, _4));
        mainApp->remoteinstPage->SetOnInput(std::bind(&remoteInstPage::onInput, mainApp->remoteinstPage, _1, _2, _3, _4));
        mainApp->optionspage->SetOnInput(std::bind(&optionsPage::onInput, mainApp->optionspage, _1, _2, _3, _4));
        mainApp->installedpage->SetOnInput(std::bind(&installedPage::onInput, mainApp->installedpage, _1, _2, _3, _4));
        if (backToOptions) mainApp->LoadLayout(mainApp->optionspage);
        else mainApp->LoadLayout(mainApp->mainPage);
    }

    Pill::Pill(int x, int y, int h, int fontSize) : x(x), y(y), h(h) {
        this->border = pu::ui::elm::Rectangle::New(x, y, h, h, Rgb(Cur().accent, 0x99), h / 2);
        this->fill = pu::ui::elm::Rectangle::New(x + 2, y + 2, h - 4, h - 4, Panel(), (h - 4) / 2);
        this->text = pu::ui::elm::TextBlock::New(x + h / 2, y, "", fontSize);
        this->text->SetColor(Text());
    }

    void Pill::SetText(const std::string& label) {
        this->text->SetText(label);
        const int w = this->text->GetWidth() + this->h;
        this->border->SetWidth(w);
        this->fill->SetWidth(w - 4);
        this->text->SetY(this->y + (this->h - this->text->GetHeight()) / 2);
    }

    int Pill::Right() const { return this->x + this->border->GetWidth(); }

    void Pill::AddTo(pu::ui::Layout* layout) {
        layout->Add(this->border);
        layout->Add(this->fill);
        layout->Add(this->text);
    }

    void MusicInit() {
        const std::filesystem::path dir = inst::config::appDir + "/music";
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        static const std::vector<std::string> exts = {".mp3", ".ogg", ".opus", ".wav", ".mod", ".xm", ".it", ".s3m"};
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (entry.is_regular_file(ec) && std::find(exts.begin(), exts.end(), ext) != exts.end())
                gTracks.push_back(entry.path().string());
        }
        if (gTracks.empty())
            return;
        std::sort(gTracks.begin(), gTracks.end());
        Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_OPUS | MIX_INIT_MOD);
        // Opened before the click sound so music gets 48 kHz; later Mix_OpenAudio calls only add a reference.
        if (Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 4096) != 0)
            return;
        gAudioReady = true;
        Mix_VolumeMusic(80);
        PlayTrack(0);
    }

    void MusicTick() {
        if (gAudioReady && !gPaused && gMusic != nullptr && !Mix_PlayingMusic())
            PlayTrack(gTrack + 1);
    }

    void MusicNext() { PlayTrack(gTrack + 1); }
    void MusicPrev() { PlayTrack(gTrack - 1); }

    void MusicToggle() {
        if (!gAudioReady || gMusic == nullptr)
            return;
        gPaused = !gPaused;
        if (gPaused) Mix_PauseMusic();
        else Mix_ResumeMusic();
    }

    std::string MusicLabel() {
        if (gTracks.empty())
            return "Music: put files in switch/ShopNX/music";
        if (!gAudioReady)
            return "Music: audio unavailable";
        std::string name = std::filesystem::path(gTracks[gTrack]).stem().string();
        if (name.size() > 30)
            name = name.substr(0, 28) + "..";
        return (gPaused ? "Paused: " : "Playing: ") + name;
    }
}
