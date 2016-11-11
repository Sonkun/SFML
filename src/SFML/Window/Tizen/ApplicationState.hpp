////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2016 Jonathan De Wachter (dewachter.jonathan@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef SFML_APPLICATIONSTATE_HPP
#define SFML_APPLICATIONSTATE_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Tizen/EvasContext.hpp>
#include <SFML/Window/Tizen/LogStream.hpp>
#include <SFML/System/Mutex.hpp>
#include <Elementary.h>
#include <Evas_GL_GLES1_Helpers.h>

namespace sf
{
namespace priv
{
struct ApplicationState
{
    Evas_Object* window;
    Evas_Object* conform;

    Evas_GL* evasgl;
    Evas_GL_Surface* sharedSurface;
    Evas_GL_Context* sharedContext;

    EvasContext* context;
    LogStream logs;

    bool initialized;
    bool resizeHandled;

    Mutex mutex;

    void (*processMouseDownEvent)(void*, Evas*, void*);
    void (*processMouseUpEvent)  (void*, Evas*, void*);
    void (*processMouseMoveEvent)(void*, Evas*, void*);
    void (*processTouchDownEvent)(void*, Evas*, void*);
    void (*processTouchUpEvent)  (void*, Evas*, void*);
    void (*processTouchMoveEvent)(void*, Evas*, void*);
    void (*processKeyDownEvent)  (void*, Evas*, void*);
    void (*processKeyUpEvent)    (void*, Evas*, void*);

    void (*sendResizeEvent)(int, int);
    void (*sendPauseEvent)();
    void (*sendResumeEvent)();
    void (*sendTerminateEvent)();
};

SFML_SYSTEM_API ApplicationState* getApplicationState(ApplicationState* initializedState=NULL, bool reset=false);

} // namespace priv
} // namespace sf


#endif // SFML_APPLICATIONSTATE_HPP
