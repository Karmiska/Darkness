#pragma once

#include "containers/unordered_map.h"

namespace engine
{
    enum class MouseButton
    {
        Left,
        Right,
        Center
    };

    enum class Key
    {
        Unknown,

        Num0,
        Num1,
        Num2,
        Num3,
        Num4, 
        Num5, 
        Num6, 
        Num7, 
        Num8, 
        Num9,

        A,
        B,
        C,
        D, 
        E, 
        F, 
        G, 
        H, 
        I, 
        J, 
        K, 
        L, 
        M, 
        N, 
        O, 
        P, 
        Q, 
        R, 
        S, 
        T, 
        U, 
        V, 
        W, 
        X, 
        Y, 
        Z,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        PrintScreen,
        ScrollLock,
        Pause,

        ArrowLeft,
        ArrowRight,
        ArrowUp,
        ArrowDown,

        PageDown,
        PageUp,

        Insert,
        Home,
        End,
        Delete,
        Escape,

        Section,            // §
        Tilde,                // ~
        Enter,
        Space,
        Tab,
        Backspace,
        
        Exclamation,        // !
        DoubleQuote,        // "
        Hash,               // #
        Weird,              // ¤        TODO
        Percent,            // %
        Ampersand,          // &
        ForwardSlash,       // /
        BackSlash,          // 
        ParenthesisOpen,    // (
        ParenthesisClose,   // )
        BracketOpen,        // [
        BracketClose,       // ]
        BraceOpen,          // {
        BraceClose,         // }
        Equal,              // =
        QuestionMark,       // ?
        BackQuote,          // `
        Asterisk,           // *
        Apostrophe,         // '
        Comma,              // ,
        Period,             // .
        Pipe,               // |
        Hyphen,             // -
        Plus                // +
    };

    enum class KeyModifier
    {
        ShiftLeft,
        ShiftRight,
        AltLeft,
        AltRight,
        CtrlLeft,
        CtrlRight
    };

    class ModifierState
    {
    public:
        ModifierState()
        {
            m_modifier[KeyModifier::ShiftLeft] = false;
            m_modifier[KeyModifier::ShiftRight] = false;
            m_modifier[KeyModifier::AltLeft] = false;
            m_modifier[KeyModifier::AltRight] = false;
            m_modifier[KeyModifier::CtrlLeft] = false;
            m_modifier[KeyModifier::CtrlRight] = false;
        }

        bool& operator[](const KeyModifier& mod)
        {
            return m_modifier[mod];
        }

        const bool operator[](const KeyModifier& mod) const
        {
            auto modres = m_modifier.find(mod);
            if (modres != m_modifier.end())
                return modres->second;
            return false;
        }

        bool operator==(const ModifierState& mod) const
        {
            const ModifierState& thismod = *this;
            return 
                ((thismod[KeyModifier::ShiftLeft] == mod[KeyModifier::ShiftLeft]) &&
                (thismod[KeyModifier::ShiftRight] == mod[KeyModifier::ShiftRight]) &&
                (thismod[KeyModifier::AltLeft] == mod[KeyModifier::AltLeft]) &&
                (thismod[KeyModifier::AltRight] == mod[KeyModifier::AltRight]) &&
                (thismod[KeyModifier::CtrlLeft] == mod[KeyModifier::CtrlLeft]) &&
                (thismod[KeyModifier::CtrlRight] == mod[KeyModifier::CtrlRight]));
        }

        bool operator!=(const ModifierState& mod) const
        {
            return !this->operator==(mod);
        }

    private:
        engine::unordered_map<KeyModifier, bool> m_modifier;
    };
}
