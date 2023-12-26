/* Project Scope */
#include "audio.h"
#ifdef PIXELCLOCK_DESKTOP
#include "audioDesktop.h"
#else
#include "audioESP32.h"
#endif



Audio& AudioSingleton::get() {
#ifdef PIXELCLOCK_DESKTOP
    static AudioDesktop instance;
#else
    static AudioESP32 instance;
#endif
    return instance;
}
