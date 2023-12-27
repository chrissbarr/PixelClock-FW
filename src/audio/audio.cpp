/* Project Scope */
#include "audio/audio.h"
#ifdef PIXELCLOCK_DESKTOP
#include "audio/desktop.h"
#else
#include "audio/ESP32.h"
#endif



Audio& AudioSingleton::get() {
#ifdef PIXELCLOCK_DESKTOP
    static AudioDesktop instance;
#else
    static AudioESP32 instance;
#endif
    return instance;
}
