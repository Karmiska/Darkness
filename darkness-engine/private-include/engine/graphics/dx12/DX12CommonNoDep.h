#pragma once

#ifndef _DURANGO
#define DARKNESS_IID_PPV_ARGS IID_PPV_ARGS
#else
#define DARKNESS_IID_PPV_ARGS(ppType) __uuidof(**(ppType)), (static_cast<IGraphicsUnknown *>(*(ppType)),reinterpret_cast<void**>(ppType))
#endif
