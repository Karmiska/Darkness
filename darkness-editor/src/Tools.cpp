#include "Tools.h"

engine::Key qtKeyToEngineKey(Qt::Key key)
{
    switch (key)
    {
    case Qt::Key_0: return engine::Key::Num0;
    case Qt::Key_1: return engine::Key::Num1;
    case Qt::Key_2: return engine::Key::Num2;
    case Qt::Key_3: return engine::Key::Num3;
    case Qt::Key_4: return engine::Key::Num4;
    case Qt::Key_5: return engine::Key::Num5;
    case Qt::Key_6: return engine::Key::Num6;
    case Qt::Key_7: return engine::Key::Num7;
    case Qt::Key_8: return engine::Key::Num8;
    case Qt::Key_9: return engine::Key::Num9;

    case Qt::Key_A: return engine::Key::A;
    case Qt::Key_B: return engine::Key::B;
    case Qt::Key_C: return engine::Key::C;
    case Qt::Key_D: return engine::Key::D;
    case Qt::Key_E: return engine::Key::E;
    case Qt::Key_F: return engine::Key::F;
    case Qt::Key_G: return engine::Key::G;
    case Qt::Key_H: return engine::Key::H;
    case Qt::Key_I: return engine::Key::I;
    case Qt::Key_J: return engine::Key::J;
    case Qt::Key_K: return engine::Key::K;
    case Qt::Key_L: return engine::Key::L;
    case Qt::Key_M: return engine::Key::M;
    case Qt::Key_N: return engine::Key::N;
    case Qt::Key_O: return engine::Key::O;
    case Qt::Key_P: return engine::Key::P;
    case Qt::Key_Q: return engine::Key::Q;
    case Qt::Key_R: return engine::Key::R;
    case Qt::Key_S: return engine::Key::S;
    case Qt::Key_T: return engine::Key::T;
    case Qt::Key_U: return engine::Key::U;
    case Qt::Key_V: return engine::Key::V;
    case Qt::Key_W: return engine::Key::W;
    case Qt::Key_X: return engine::Key::X;
    case Qt::Key_Y: return engine::Key::Y;
    case Qt::Key_Z: return engine::Key::Z;

    case Qt::Key_F1: return engine::Key::F1;
    case Qt::Key_F2: return engine::Key::F2;
    case Qt::Key_F3: return engine::Key::F3;
    case Qt::Key_F4: return engine::Key::F4;
    case Qt::Key_F5: return engine::Key::F5;
    case Qt::Key_F6: return engine::Key::F6;
    case Qt::Key_F7: return engine::Key::F7;
    case Qt::Key_F8: return engine::Key::F8;
    case Qt::Key_F9: return engine::Key::F9;
    case Qt::Key_F10: return engine::Key::F10;
    case Qt::Key_F11: return engine::Key::F11;
    case Qt::Key_F12: return engine::Key::F12;

    case Qt::Key_Print: return engine::Key::PrintScreen;
    case Qt::Key_ScrollLock: return engine::Key::ScrollLock;
    case Qt::Key_Pause: return engine::Key::Pause;

    case Qt::Key_Left: return engine::Key::ArrowLeft;
    case Qt::Key_Right: return engine::Key::ArrowRight;
    case Qt::Key_Up: return engine::Key::ArrowUp;
    case Qt::Key_Down: return engine::Key::ArrowDown;

    case Qt::Key_Delete: return engine::Key::Delete;
    default: return engine::Key::Unknown;
    }
}

#define EXTENDED_KEY_MASK   0x01000000

quint32 interpretKeyEvent(QKeyEvent* e)
{
    quint32 vk = e->nativeVirtualKey();

    quint32 mods = e->nativeModifiers();
    bool extended = (bool)(mods & EXTENDED_KEY_MASK);

    quint32 scancode = e->nativeScanCode();

    switch (vk)
    {
    case VK_CONTROL:
        vk = extended ? VK_RCONTROL : VK_LCONTROL;
        break;
    case VK_MENU:
        // VK_MENU = ALT virtual key
        vk = extended ? VK_RMENU : VK_LMENU;
        break;
    case VK_SHIFT:
        vk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
        break;
    default:
        break;
    };
    return vk;
}

