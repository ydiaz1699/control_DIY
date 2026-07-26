#pragma once

// ============================================================================
// Screensaver - Clock + Weather widget for idle mode
// Novel feature: intelligent screensaver showing useful info
// ============================================================================

#include <cstdint>
#include <cstring>

struct WeatherData {
    float temperature;
    float humidity;
    char condition[32];     // "sunny", "cloudy", "rainy", etc.
    char icon[8];           // Material icon codepoint
    float tempMin;
    float tempMax;
};

struct HomeStatus {
    int lightsOn;           // Number of lights on
    int doorsOpen;          // Number of doors/windows open
    bool alarmArmed;
    bool mediaPlaying;
    char mediaTitle[64];
    char mediaArtist[64];
};

class Screensaver {
public:
    // Layout configuration for 480x480 screen
    static constexpr int SCREEN_W = 480;
    static constexpr int SCREEN_H = 480;
    static constexpr int CLOCK_Y = 100;
    static constexpr int DATE_Y = 180;
    static constexpr int WEATHER_Y = 240;
    static constexpr int STATUS_Y = 340;
    static constexpr int MEDIA_Y = 400;

    Screensaver() {
        memset(&weather_, 0, sizeof(weather_));
        memset(&homeStatus_, 0, sizeof(homeStatus_));
        strcpy(weather_.condition, "clear");
        strcpy(weather_.icon, "\ue2bd");  // Material icon: wb_sunny
    }

    // Update data
    void setTime(int hour, int minute) {
        hour_ = hour;
        minute_ = minute;
    }

    void setDate(int day, int month, int year, const char* dayName) {
        day_ = day;
        month_ = month;
        year_ = year;
        strncpy(dayName_, dayName, sizeof(dayName_) - 1);
    }

    void setWeather(const WeatherData& data) {
        weather_ = data;
    }

    void setHomeStatus(const HomeStatus& status) {
        homeStatus_ = status;
    }

    // Getters for ESPHome display lambda
    int getHour() const { return hour_; }
    int getMinute() const { return minute_; }
    int getDay() const { return day_; }
    int getMonth() const { return month_; }
    const char* getDayName() const { return dayName_; }
    const WeatherData& getWeather() const { return weather_; }
    const HomeStatus& getHomeStatus() const { return homeStatus_; }

    // Should the clock colon blink?
    bool shouldBlinkColon() const {
        return (millis() / 500) % 2 == 0;
    }

    // Get weather icon based on condition
    static const char* getWeatherIcon(const char* condition) {
        if (strcmp(condition, "sunny") == 0 || strcmp(condition, "clear-night") == 0)
            return "\ue2bd";  // wb_sunny
        if (strcmp(condition, "cloudy") == 0 || strcmp(condition, "partlycloudy") == 0)
            return "\ue2cc";  // cloud
        if (strcmp(condition, "rainy") == 0)
            return "\uf176";  // rainy
        if (strcmp(condition, "snowy") == 0)
            return "\ue2cd";  // ac_unit
        if (strcmp(condition, "fog") == 0)
            return "\ue818";  // foggy
        return "\ue2bd";  // default: sunny
    }

    // Animation offset (slow drift for visual interest)
    int getAnimationOffset() const {
        return (int)(sin(millis() / 5000.0) * 3);
    }

private:
    int hour_ = 12;
    int minute_ = 0;
    int day_ = 1;
    int month_ = 1;
    int year_ = 2025;
    char dayName_[12] = "Monday";
    WeatherData weather_;
    HomeStatus homeStatus_;

    static unsigned long millis() {
        // Will be provided by Arduino framework at runtime
        extern unsigned long millis();
        return ::millis();
    }

    static double sin(double x) {
        // Simple sine approximation for animation
        x = x - (int)(x / 6.2832) * 6.2832;
        if (x < 0) x += 6.2832;
        double x2 = x * x;
        return x * (1.0 - x2 / 6.0 * (1.0 - x2 / 20.0));
    }
};
