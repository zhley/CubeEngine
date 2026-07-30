#pragma once

#include <cstdint>

namespace Cube {

    struct Color {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 0.0f;
        constexpr Color() = default;
        constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
        constexpr Color(int r, int g, int b, int a) : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a / 255.0f) {}
        constexpr Color(uint32_t hex) : r((hex >> 24 & 0xFF) / 255.0f), g((hex >> 16 & 0xFF) / 255.0f), b((hex >> 8 & 0xFF) / 255.0f), a((hex & 0xFF) / 255.0f) {}
    };

    constexpr Color BLACK    = {0.0f, 0.0f, 0.0f, 1.0f};
    constexpr Color WHITE    = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr Color RED      = {1.0f, 0.0f, 0.0f, 1.0f};
    constexpr Color GREEN    = {0.0f, 1.0f, 0.0f, 1.0f};
    constexpr Color BLUE     = {0.0f, 0.0f, 1.0f, 1.0f};
    constexpr Color CYAN     = {0.0f, 1.0f, 1.0f, 1.0f};
    constexpr Color MAGENTA  = {1.0f, 0.0f, 1.0f, 1.0f};
    constexpr Color YELLOW   = {1.0f, 1.0f, 0.0f, 1.0f};
}