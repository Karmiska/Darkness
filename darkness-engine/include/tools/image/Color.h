#pragma once

namespace bmp
{
    class Color
    {
    private:
        unsigned int mColor;
    public:
        Color();
        Color(unsigned char red,
            unsigned char green,
            unsigned char blue,
            unsigned char alpha = 0);
        Color(unsigned int color);

        static Color from565rgb(unsigned short color);
        static Color from565bgr(unsigned short color);

        unsigned char red() const;
        unsigned char green() const;
        unsigned char blue() const;
        unsigned char alpha() const;

        void setRed(unsigned char value);
        void setGreen(unsigned char value);
        void setBlue(unsigned char value);
        void setAlpha(unsigned char value);

        unsigned short get565rgb() const;
        unsigned short get565bgr() const;
        unsigned int getRGBA() const;

        friend bool operator> (const Color& a, const Color& b);
        friend bool operator<= (const Color& a, const Color& b);

        friend bool operator< (const Color& a, const Color& b);
        friend bool operator>= (const Color& a, const Color& b);

        friend Color operator+(Color lhs, const Color& rhs);
        friend Color operator-(Color lhs, const Color& rhs);
        friend Color operator*(Color lhs, const Color& rhs);
        friend Color operator*(Color lhs, const float& rhs);

        Color& operator+=(const Color& rhs);
        Color& operator-=(const Color& rhs);
        Color& operator*=(const Color& rhs);
        Color& operator*=(const float& rhs);
    };
}
