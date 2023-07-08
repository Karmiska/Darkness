#pragma once

#include <QWidget>
#include <QKeyEvent>
#include "engine/Engine.h"

#define EXTENDED_KEY_MASK   0x01000000

engine::Key qtKeyToEngineKey(Qt::Key key);
quint32 interpretKeyEvent(QKeyEvent* e);
