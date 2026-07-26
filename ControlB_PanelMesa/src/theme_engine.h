#pragma once

// ============================================================================
// Theme Engine - Dynamic theme switching based on time/activity
// Novel feature: auto-adapting UI themes
// ============================================================================

#include <cstdint>

struct ThemeColors {
    uint32_t background;
    uint32_t surface;
    uint32_t primary;
    uint32_t secondary;
    uint32_t text_primary;
    uint32_t text_secondary;
    uint32_t accent;
    uint32_t error;
    uint32_t success;
    uint32_t border;
};

enum class ThemeMode : uint8_t {
    DARK,
    LIGHT,
    AUTO,           // Based on time of day
    SCENE_BASED     // Based on active scene
};

class ThemeEngine {
public:
    static const ThemeColors DARK_THEME;
    static const ThemeColors LIGHT_THEME;
    static const ThemeColors MOVIE_THEME;   // Extra dim for movie mode
    static const ThemeColors NIGHT_THEME;   // Very dim warm tones

    ThemeEngine() : mode_(ThemeMode::AUTO), currentTheme_(&DARK_THEME) {}

    void setMode(ThemeMode mode) {
        mode_ = mode;
        updateTheme();
    }

    void setHour(int hour) {
        currentHour_ = hour;
        if (mode_ == ThemeMode::AUTO) {
            updateTheme();
        }
    }

    void setActiveScene(const char* scene) {
        activeScene_ = scene;
        if (mode_ == ThemeMode::SCENE_BASED) {
            updateTheme();
        }
    }

    const ThemeColors* getTheme() const { return currentTheme_; }
    ThemeMode getMode() const { return mode_; }
    bool isDark() const { return currentTheme_ == &DARK_THEME || 
                                  currentTheme_ == &MOVIE_THEME || 
                                  currentTheme_ == &NIGHT_THEME; }

    // Backlight level recommendation (0.0 - 1.0)
    float getRecommendedBrightness() const {
        if (currentTheme_ == &MOVIE_THEME) return 0.3f;
        if (currentTheme_ == &NIGHT_THEME) return 0.2f;
        if (currentTheme_ == &DARK_THEME) return 0.7f;
        return 1.0f;
    }

private:
    ThemeMode mode_;
    const ThemeColors* currentTheme_;
    int currentHour_ = 12;
    const char* activeScene_ = "";

    void updateTheme() {
        switch (mode_) {
            case ThemeMode::DARK:
                currentTheme_ = &DARK_THEME;
                break;
            case ThemeMode::LIGHT:
                currentTheme_ = &LIGHT_THEME;
                break;
            case ThemeMode::AUTO:
                if (currentHour_ >= 6 && currentHour_ < 18) {
                    currentTheme_ = &LIGHT_THEME;
                } else if (currentHour_ >= 22 || currentHour_ < 6) {
                    currentTheme_ = &NIGHT_THEME;
                } else {
                    currentTheme_ = &DARK_THEME;
                }
                break;
            case ThemeMode::SCENE_BASED:
                updateSceneTheme();
                break;
        }
    }

    void updateSceneTheme() {
        // Match scene name to appropriate theme
        if (strcmp(activeScene_, "Pelicula") == 0 || 
            strcmp(activeScene_, "Movie") == 0) {
            currentTheme_ = &MOVIE_THEME;
        } else if (strcmp(activeScene_, "Buenas Noches") == 0 ||
                   strcmp(activeScene_, "Night") == 0) {
            currentTheme_ = &NIGHT_THEME;
        } else {
            // Fall back to auto
            if (currentHour_ >= 6 && currentHour_ < 18) {
                currentTheme_ = &LIGHT_THEME;
            } else {
                currentTheme_ = &DARK_THEME;
            }
        }
    }
};

// Theme definitions
inline const ThemeColors ThemeEngine::DARK_THEME = {
    .background    = 0x121212,
    .surface       = 0x1E1E1E,
    .primary       = 0xBB86FC,
    .secondary     = 0x03DAC6,
    .text_primary  = 0xFFFFFF,
    .text_secondary = 0xB0B0B0,
    .accent        = 0xCF6679,
    .error         = 0xCF6679,
    .success       = 0x4CAF50,
    .border        = 0x333333
};

inline const ThemeColors ThemeEngine::LIGHT_THEME = {
    .background    = 0xFAFAFA,
    .surface       = 0xFFFFFF,
    .primary       = 0x6200EE,
    .secondary     = 0x03DAC6,
    .text_primary  = 0x212121,
    .text_secondary = 0x757575,
    .accent        = 0xB00020,
    .error         = 0xB00020,
    .success       = 0x4CAF50,
    .border        = 0xE0E0E0
};

inline const ThemeColors ThemeEngine::MOVIE_THEME = {
    .background    = 0x0A0A0A,
    .surface       = 0x141414,
    .primary       = 0x7B1FA2,
    .secondary     = 0x424242,
    .text_primary  = 0xCCCCCC,
    .text_secondary = 0x666666,
    .accent        = 0x9C27B0,
    .error         = 0xCF6679,
    .success       = 0x388E3C,
    .border        = 0x1A1A1A
};

inline const ThemeColors ThemeEngine::NIGHT_THEME = {
    .background    = 0x0D0D1A,
    .surface       = 0x141428,
    .primary       = 0xFF8A65,    // Warm orange
    .secondary     = 0x4E342E,
    .text_primary  = 0xFFCCBC,
    .text_secondary = 0x8D6E63,
    .accent        = 0xFFAB91,
    .error         = 0xEF5350,
    .success       = 0x66BB6A,
    .border        = 0x1A1A2E
};
