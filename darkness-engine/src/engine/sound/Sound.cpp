#include "engine/sound/Sound.h"

#if 0
#include "Framework.h"

namespace engine
{
    Sound::Sound()
    {
        // Initialize Framework
        ALFWInit();

        ALFWprintf("PlayStatic Test Application\n");

        if (!ALFWInitOpenAL())
        {
            ALFWprintf("Failed to initialize OpenAL\n");
            ALFWShutdown();
            //return 0;
        }

        // Generate an AL Buffer
        ALuint      uiBuffer;
        ALuint      uiSource;
        ALint       iState;

        alGenBuffers(1, &uiBuffer);
    }

    Sound::~Sound()
    {
        ALFWShutdownOpenAL();
        ALFWShutdown();
    }
}
#endif