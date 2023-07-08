#pragma once

#include "engine/primitives/Vector3.h"

namespace engine
{
    class Color4f
    {
        float m_colors[4];
    public:
        Color4f(float red, float green, float blue, float alpha)
            : m_colors{ red, green, blue, alpha }
        {}
        Color4f(const Color4f& color)
            : m_colors{ color.m_colors[0], 
                        color.m_colors[1],
                        color.m_colors[2],
                        color.m_colors[3] }
        {}
		Color4f(const Vector3f& color)
			: m_colors{ color.x, color.y, color.z, 1.0f }
		{}

        float& operator[](int color)
        {
            return m_colors[color];
        };
        const float* get() const
        {
            return &m_colors[0];
        }

        float red() const
        {
            return m_colors[0];
        }
        float green() const
        {
            return m_colors[1];
        }
        float blue() const
        {
            return m_colors[2];
        }
        float alpha() const
        {
            return m_colors[3];
        }
    };
}

// Just hiding this thing somewhere
// TODO: implement proper support
#if 1
namespace engine_565
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
        Color(const engine::Vector3<float>& col);

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
#endif