#pragma once
#include <string>
#include <pu/Plutonium>

// Luma-style themes (Wii, Wii Black, Cafe, Pikmin, Classic) and the header music player.
namespace inst::ui::theme {
    int Count();
    const char* Name(int index);
    const char* CurrentName();

    // Maps ShopNX's original navy/violet palette (still written as COLOR("#...") in every page)
    // to the active theme, so pages only swap their COLOR macro.
    pu::ui::Color Map(const char* hex);
    pu::ui::Color Panel();
    pu::ui::Color Line();
    pu::ui::Color Text();
    pu::ui::Color Muted();
    pu::ui::Color Accent();
    pu::ui::Color Green();
    std::string Background();
    std::string Logo();
    // Light themes get blue-tinted copies of the white icons.
    std::string Icon(const std::string& file);
    // Curved bar along the bottom edge, behind the button hints.
    pu::ui::elm::Image::Ref BottomBar();

    // Hand pointer, always on screen: either stick, touch or controller motion moves it; hovering focuses grid tiles.
    void PointerTick();
    bool PointerHover(int& x, int& y); // true on frames the pointer moved
    void AttachPointer();              // adds the hand on top of every page

    // Theme dialog; a new pick is saved and every page is rebuilt in it.
    void Pick(bool backToOptions);

    // Rounded pill like Luma's header buttons: border, fill and centred label.
    struct Pill {
        pu::ui::elm::Rectangle::Ref border;
        pu::ui::elm::Rectangle::Ref fill;
        pu::ui::elm::TextBlock::Ref text;
        int x = 0, y = 0, h = 0;
        Pill(int x, int y, int h, int fontSize);
        void SetText(const std::string& label);
        int Right() const;
        void AddTo(pu::ui::Layout* layout);
    };

    // Plays sdmc:/switch/ShopNX/music (mp3/ogg/opus/wav/mod) in a loop.
    void MusicInit();
    void MusicTick();
    void MusicNext();
    void MusicPrev();
    void MusicToggle();
    std::string MusicLabel();
}
