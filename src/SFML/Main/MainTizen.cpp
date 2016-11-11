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


////////////////////////////////////////////////////////////
// Tizen specific: SFML needs to hook the main function, to
// launch the application (event loop), and then call the
// user main from inside it.
//
// Our strategy is to rename the user main to 'sfmlMain' with
// a macro (see Main.hpp), and call this modified main ourselves.
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Tizen/ApplicationState.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Lock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Err.hpp>
#include <app.h>
#include <system_settings.h>
#include <Elementary.h>
#include <Evas_GL_GLES1_Helpers.h>
#include <cassert>

EVAS_GL_GLOBAL_GLES1_DECLARE();

extern int sfmlMain(int argc, char *argv[]);

////////////////////////////////////////////////////////////
void onWindowResized(void* data, Evas* event, Evas_Object* window, void* event_info);

////////////////////////////////////////////////////////////
void userMain()
{
    // Cancel out the user main arguments
    sfmlMain(0, NULL);
}

////////////////////////////////////////////////////////////
bool onAppCreated(void* data)
{
    // Retrieve the application state, no need to lock, the user main
    // isn't launched yet
    sf::priv::ApplicationState* state = static_cast<sf::priv::ApplicationState*>(data);

    // Indicate the underlying backend to use
    elm_config_accel_preference_set("opengl"); // Since 2.3.1, investigate "opengl:depth24:stencil8:msaa_high"

    // Create and setup the window
    state->window = elm_win_util_standard_add("SFML", "UI Template");

    elm_win_conformant_set(state->window, EINA_TRUE);
    elm_win_indicator_mode_set(state->window, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(state->window, ELM_WIN_INDICATOR_TRANSPARENT);

    evas_object_show(state->window);

    // We can now register our event callbacks (later, investigate AXIS_UPDATE, etc)
    evas_object_event_callback_add(state->window, EVAS_CALLBACK_RESIZE, onWindowResized, state);

    // Create a conform attached to the window and in which the image resides
    state->conform = elm_conformant_add(state->window);
    evas_object_size_hint_weight_set(state->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(state->window, state->conform);

    evas_object_show(state->conform);

    // Get the initial window size (should be 1x1)
    Evas_Coord width, height;
    evas_object_geometry_get(state->window, NULL, NULL, &width, &height);

    if (!(width == 1 && height == 1))
        sf::err() << "Expected initial window to be of size 1x1" << std::endl;

    // Initialize Evas GL and create shared context and surface (apparently,
    // that must be done in the main thread)
    state->evasgl = evas_gl_new(evas_object_evas_get(state->window));

    Evas_GL_Config* config = evas_gl_config_new();

    config->color_format     = EVAS_GL_RGBA_8888;
    config->depth_bits       = EVAS_GL_DEPTH_NONE;
    config->stencil_bits     = EVAS_GL_STENCIL_NONE;
    config->multisample_bits = EVAS_GL_MULTISAMPLE_NONE;
    config->options_bits     = EVAS_GL_OPTIONS_NONE;

    state->sharedSurface = evas_gl_surface_create(state->evasgl, config, width, height);
    state->sharedContext = evas_gl_context_version_create(state->evasgl, NULL, EVAS_GL_GLES_1_X);

    evas_gl_make_current(state->evasgl, state->sharedSurface, state->sharedContext);
    EVAS_GL_GLOBAL_GLES1_USE(state->evasgl, state->sharedContext);

    evas_gl_config_free(config);

    // Detect early Evas GL errors before launching main thread
    int errorCode = evas_gl_error_get(state->evasgl);

    if (errorCode != EVAS_GL_SUCCESS)
        sf::err() << "Ran into early Evas GL errors; expect errors in subsequent code" << std::endl;

    // Launch the user main in a thread
    sf::Thread* thread = new sf::Thread(userMain);
    thread->launch();

    // Wait until the user window grabs the underlying window and the
    // Evas context is created
    state->mutex.lock();

    while (!state->initialized)
    {
        state->mutex.unlock();
        sf::sleep(sf::seconds(0.001));
        state->mutex.lock();
    }

    state->mutex.unlock();

    return true;
}

////////////////////////////////////////////////////////////
void onAppPaused(void* data)
{
    // The application is now invisible, send sf::Event::LostFocus to
    // the user main loop
    sf::priv::ApplicationState* state = static_cast<sf::priv::ApplicationState*>(data);
    sf::Lock lock(state->mutex);

    assert(state->sendPauseEvent);
    state->sendPauseEvent();
}

////////////////////////////////////////////////////////////
void onAppResumed(void* data)
{
    // The application is now visible, send sf::Event::GainedFocus the
    // user main loop
    sf::priv::ApplicationState* state = static_cast<sf::priv::ApplicationState*>(data);
    sf::Lock lock(state->mutex);

    assert(state->sendResumeEvent);
    state->sendResumeEvent();
}

////////////////////////////////////////////////////////////
void onAppTerminated(void* data)
{
    // The application is requested to terminate, send sf::Event::Closed
    // to the user main loop
    sf::priv::ApplicationState* state = static_cast<sf::priv::ApplicationState*>(data);
    sf::Lock lock(state->mutex);

    assert(state->sendTerminateEvent);
    state->sendTerminateEvent();
}

////////////////////////////////////////////////////////////
void onWindowResized(void* data, Evas* event, Evas_Object* window, void* event_info)
{
    // The application was resized, send sf::Event::Resized to the user
    // main loop
    sf::priv::ApplicationState* state = static_cast<sf::priv::ApplicationState*>(data);

    // Retrieve new window size
    Evas_Coord width, height;
    evas_object_geometry_get(window, NULL, NULL, &width, &height);

    // Send resize event and wait until it's handled (the Evas GL surface
    // is recreated)
    state->mutex.lock();

    state->resizeHandled = false;
    state->sendResizeEvent(width, height);

    while (!state->resizeHandled)
    {
        state->mutex.unlock();
        sf::sleep(sf::seconds(0.001));
        state->mutex.lock();
    }

    state->mutex.unlock();
}

////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // Create the application state (will bridge the main loop and
    // the user main loop)
    sf::priv::ApplicationState* state = new sf::priv::ApplicationState;

    // Initialize the application state values
    state->window  = NULL;
    state->conform = NULL;
    state->evasgl = NULL;
    state->sharedContext = NULL;
    state->sharedSurface = NULL;
    state->initialized = false;
    state->resizeHandled = false;
    state->context = NULL;

    // Share the application state across SFML modules
    sf::priv::getApplicationState(state, true);

    // Redirect error messages to system logs
    sf::err().rdbuf(&state->logs);

    // Register life cycle event callbacks
    ui_app_lifecycle_callback_s eventCallback = {0,};

    eventCallback.create      = onAppCreated;
    eventCallback.terminate   = onAppTerminated;
    eventCallback.pause       = onAppPaused;
    eventCallback.resume      = onAppResumed;
    eventCallback.app_control = NULL;

    // Run the main loop
    return ui_app_main(argc, argv, &eventCallback, state);
}
