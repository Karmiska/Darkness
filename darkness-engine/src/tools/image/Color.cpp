#include "tools/image/Color.h"

namespace bmp
{
    Color::Color()
        : mColor(0)
    {
    }

    Color::Color(unsigned char red,
        unsigned char green,
        unsigned char blue,
        unsigned char alpha)
        : mColor(red << 24 | green << 16 | blue << 8 | alpha)
    {
    }

    Color::Color(unsigned int color)
        : mColor(color)
    {
    }

    Color Color::from565rgb(unsigned short color)
    {
        return Color(
            static_cast<unsigned char>(static_cast<unsigned int>(color & 0xF800) >> 8),
            static_cast<unsigned char>(static_cast<unsigned int>(color & 0x07E0) >> 3),
            static_cast<unsigned char>(static_cast<unsigned int>(color & 0x001F) << 3)
        );
    }

    Color Color::from565bgr(unsigned short color)
    {
        return Color(
            static_cast<unsigned char>(static_cast<unsigned int>(color & 0x001F) << 3),
            static_cast<unsigned char>(static_cast<unsigned int>(color & 0x07E0) >> 3),
            static_cast<unsigned char>(static_cast<unsigned int>(color & 0xF800) >> 8)
        );
    }

    unsigned char Color::red() const
    {
        return (unsigned char)((mColor & 0xFF000000) >> 24);
    }
    unsigned char Color::green() const
    {
        return (unsigned char)((mColor & 0x00FF0000) >> 16);
    }

    unsigned char Color::blue() const
    {
        return (unsigned char)((mColor & 0x0000FF00) >> 8);
    }

    unsigned char Color::alpha() const
    {
        return (unsigned char)(mColor & 0x000000FF);
    }

    void Color::setRed(unsigned char value)
    {
        mColor = (mColor & 0x00FFFFFF) | (((unsigned int)value << 24) & 0xFF000000);
    }

    void Color::setGreen(unsigned char value)
    {
        mColor = (mColor & 0xFF00FFFF) | (((unsigned int)value << 16) & 0x00FF0000);
    }

    void Color::setBlue(unsigned char value)
    {
        mColor = (mColor & 0xFFFF00FF) | (((unsigned int)value << 8) & 0x0000FF00);
    }

    void Color::setAlpha(unsigned char value)
    {
        mColor = (mColor & 0xFFFFFF00) | ((unsigned int)value & 0x000000FF);
    }

    unsigned short Color::get565rgb() const
    {
        return (((unsigned char)(((double)red() / 255.0)*31.0)) & 0x001F) |
            ((((unsigned char)(((double)green() / 255.0)*63.0)) << 5) & 0x07E0) |
            ((((unsigned char)(((double)blue() / 255.0)*31.0)) << 11) & 0xF800);
    }

    unsigned short Color::get565bgr() const
    {
        return (((unsigned char)(((double)blue() / 255.0)*31.0)) & 0x001F) |
            ((((unsigned char)(((double)green() / 255.0)*63.0)) << 5) & 0x07E0) |
            ((((unsigned char)(((double)red() / 255.0)*31.0)) << 11) & 0xF800);
    }

    unsigned int Color::getRGBA() const
    {
        return mColor;
    }

    bool operator> (const Color& a, const Color& b)
    {
        return a.mColor > b.mColor;
    }

    bool operator<= (const Color& a, const Color& b)
    {
        return a.mColor <= b.mColor;
    }

    bool operator< (const Color& a, const Color& b)
    {
        return a.mColor < b.mColor;
    }

    bool operator>= (const Color& a, const Color& b)
    {
        return a.mColor >= b.mColor;
    }

    Color operator+(Color lhs, const Color& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    Color operator-(Color lhs, const Color& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    Color operator*(Color lhs, const Color& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    Color operator*(Color lhs, const float& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    Color& Color::operator+=(const Color& rhs)
    {
        mColor = Color(
            red() + rhs.red(),
            green() + rhs.green(),
            blue() + rhs.blue(),
            alpha() + rhs.alpha()
        ).mColor;
        return *this;
    }

    Color& Color::operator-=(const Color& rhs)
    {
        mColor = Color(
            red() - rhs.red(),
            green() - rhs.green(),
            blue() - rhs.blue(),
            alpha() - rhs.alpha()
        ).mColor;
        return *this;
    }

    Color& Color::operator*=(const Color& rhs)
    {
        mColor = Color(
            red() * rhs.red(),
            green() * rhs.green(),
            blue() * rhs.blue(),
            alpha() * rhs.alpha()
        ).mColor;
        return *this;
    }

    Color& Color::operator*=(const float& rhs)
    {
        mColor = Color(
            (unsigned char)(red() * rhs),
            (unsigned char)(green() * rhs),
            (unsigned char)(blue() * rhs),
            (unsigned char)(alpha() * rhs)
        ).mColor;
        return *this;
    }
}
