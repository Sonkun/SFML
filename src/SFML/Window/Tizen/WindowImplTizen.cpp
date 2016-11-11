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
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/WindowStyle.hpp> // important to be included first (conflict with None)
#include <SFML/Window/Tizen/WindowImplTizen.hpp>
#include <SFML/Window/Tizen/ApplicationState.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Lock.hpp>
#include <queue>


namespace
{
    std::queue<sf::Event> systemEvents;
    sf::Mutex mutex;
}


////////////////////////////////////////////////////////////
void imagePixelsCallback(void* data, Evas_Object* obj)
{
    // Do nothing
}

////////////////////////////////////////////////////////////
// Private data
////////////////////////////////////////////////////////////
namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
WindowImplTizen::WindowImplTizen(WindowHandle handle) :
m_image(NULL),
m_hasFocus(false),
m_size(0, 0)
{
}


////////////////////////////////////////////////////////////
WindowImplTizen::WindowImplTizen(VideoMode mode, const String& title, unsigned long style, const ContextSettings& settings)
: m_image(NULL)
, m_hasFocus(false)
{
    priv::ApplicationState* state = priv::getApplicationState();
    Lock lock(state->mutex);

    // todo: check that window is differetn from NULL
    m_image = evas_object_image_filled_add(evas_object_evas_get(state->conform));
    evas_object_image_pixels_get_callback_set(m_image, imagePixelsCallback, NULL); // SANS CEtte ligne, CA BUG SEVERE OMG
    elm_object_content_set(state->conform, m_image);

    state->sendResizeEvent    = sendResizeEvent;
    state->sendPauseEvent     = sendPauseEvent;
    state->sendResumeEvent    = sendResumeEvent;
    state->sendTerminateEvent = sendTerminateEvent;

    state->processMouseDownEvent = processMouseDownEvent;
    state->processMouseUpEvent   = processMouseUpEvent;
    state->processMouseMoveEvent = processMouseMoveEvent;
    state->processTouchDownEvent = processTouchDownEvent;
    state->processTouchUpEvent   = processTouchUpEvent;
    state->processTouchMoveEvent = processTouchMoveEvent;
    state->processKeyDownEvent   = processKeyDownEvent;
    state->processKeyUpEvent     = processKeyUpEvent;

    //// Mark the application state as initialized
    //state->initialized = true;
}


////////////////////////////////////////////////////////////
WindowImplTizen::~WindowImplTizen()
{
}


////////////////////////////////////////////////////////////
WindowHandle WindowImplTizen::getSystemHandle() const
{
    // To be implemented
    return m_image;
}


////////////////////////////////////////////////////////////
void WindowImplTizen::processEvents()
{
    Lock lock(mutex);

    Event event;

    while (!systemEvents.empty())
    {
        event = systemEvents.front();
        systemEvents.pop();

        if (event.type == Event::Resized)
        {
            m_size.x = event.size.width;
            m_size.y = event.size.height;

            // The window was resized, our strategy is to recreate the image
            // and the Evas GL surface attached to it.
            // Note: this step is a bit radical, I experimented tons of way
            // to recreate the surface but it worked randomely, the thing is
            // Evas objects seem to do EGL and OpenGL stuff internally and it
            // probably breaks the context. With a lack of understanding, I
            // couldnt really debug this, kept it for later (but I'm closed
            // because I could get things working once)
            priv::ApplicationState* state = priv::getApplicationState();
            Lock(state->mutex);

            // Step 1 - Detach the surface from image and destroy it
            evas_object_image_native_surface_set(m_image, NULL);
            state->context->destroySurface();

            // Step 2 - Recreate image
            m_image = evas_object_image_filled_add(evas_object_evas_get(state->conform));
            evas_object_resize(m_image, event.size.width, event.size.height);

            // Step 3 - Create surface and attach it to image
            state->context->createSurface(m_image, event.size.width, event.size.height);

            elm_object_content_set(state->conform, m_image);
            //evas_object_image_size_set(m_image, event.size.width, event.size.height);
            evas_object_image_pixels_get_callback_set(m_image, imagePixelsCallback, NULL); // SANS CEtte ligne, CA BUG SEVERE OMG
            evas_object_show(m_image);
            evas_object_image_pixels_dirty_set(m_image, EINA_TRUE);

            // Mark the operation as done
            state->resizeHandled = true;
        }
        else if (event.type == Event::LostFocus)
        {
            m_hasFocus = false;
        }
        else if (event.type == Event::GainedFocus)
        {
            m_hasFocus = true;
        }

        pushEvent(event);
    }
}


////////////////////////////////////////////////////////////
Vector2i WindowImplTizen::getPosition() const
{
    // Not applicable
    return Vector2i(0, 0);
}


////////////////////////////////////////////////////////////
void WindowImplTizen::setPosition(const Vector2i& position)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
Vector2u WindowImplTizen::getSize() const
{
    // To be implemented
    return m_size;
}


////////////////////////////////////////////////////////////
void WindowImplTizen::setSize(const Vector2u& size)
{
    // To be implemented - applicable ?
}


////////////////////////////////////////////////////////////
void WindowImplTizen::setTitle(const String& title)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplTizen::setIcon(unsigned int width, unsigned int height, const Uint8* pixels)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplTizen::setVisible(bool visible)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplTizen::setMouseCursorVisible(bool visible)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplTizen::setMouseCursorGrabbed(bool grabbed)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplTizen::setKeyRepeatEnabled(bool enabled)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplTizen::requestFocus()
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool WindowImplTizen::hasFocus() const
{
    return m_hasFocus;
}


////////////////////////////////////////////////////////////
void WindowImplTizen::sendResizeEvent(int width, int height)
{
    sf::Event event;

    event.size.width = width;
    event.size.height = height;
    event.type = Event::Resized;

    Lock lock(mutex);
    systemEvents.push(event);
}


////////////////////////////////////////////////////////////
void WindowImplTizen::sendPauseEvent()
{
    sf::Event event;
    event.type = Event::LostFocus;

    Lock lock(mutex);
    systemEvents.push(event);
}


////////////////////////////////////////////////////////////
void WindowImplTizen::sendResumeEvent()
{
    sf::Event event;
    event.type = Event::GainedFocus;

    Lock lock(mutex);
    systemEvents.push(event);
}


////////////////////////////////////////////////////////////
void WindowImplTizen::sendTerminateEvent()
{
    sf::Event event;
    event.type = Event::Closed;

    Lock lock(mutex);
    systemEvents.push(event);
}


////////////////////////////////////////////////////////////
void WindowImplTizen::processMouseDownEvent(void* data, Evas* evas, void* eventInfo)
{
    Evas_Event_Mouse_Down* eventData = static_cast<Evas_Event_Mouse_Down*>(eventInfo);

    sf::Event mouseButtonevent;
    mouseButtonevent.type = Event::MouseButtonPressed;
    //mouseButtonevent.mouseButton.button = eventData->button;
    mouseButtonevent.mouseButton.button = static_cast<Mouse::Button>(eventData->button);
    mouseButtonevent.mouseButton.x = eventData->output.x;
    mouseButtonevent.mouseButton.y = eventData->output.y;

    sf::Event touchEvent;
    touchEvent.type = Event::TouchBegan;
    touchEvent.touch.finger = 0;
    touchEvent.touch.x = eventData->output.x;
    touchEvent.touch.y = eventData->output.y;

    Lock lock(mutex);
    systemEvents.push(mouseButtonevent);
    systemEvents.push(touchEvent);
}


////////////////////////////////////////////////////////////
void WindowImplTizen::processMouseUpEvent(void* data, Evas* evas, void* eventInfo)
{
    Evas_Event_Mouse_Up* eventData = static_cast<Evas_Event_Mouse_Up*>(eventInfo);

    sf::Event mouseEvent;
    mouseEvent.type = Event::MouseButtonReleased;
    //mouseEvent.mouseButton.button = eventData->button;
    mouseEvent.mouseButton.button = static_cast<Mouse::Button>(eventData->button);
    mouseEvent.mouseButton.x = eventData->output.x;
    mouseEvent.mouseButton.y = eventData->output.y;

    sf::Event touchEvent;
    touchEvent.type = Event::TouchEnded;
    touchEvent.touch.finger = 0;
    touchEvent.touch.x = eventData->output.x;
    touchEvent.touch.y = eventData->output.y;

    Lock lock(mutex);
    systemEvents.push(mouseEvent);
    systemEvents.push(touchEvent);
}


////////////////////////////////////////////////////////////
void WindowImplTizen::processMouseMoveEvent(void* data, Evas* evas, void* eventInfo)
{
    Evas_Event_Mouse_Move* eventData = static_cast<Evas_Event_Mouse_Move*>(eventInfo);

    sf::Event mouseEvent;
    mouseEvent.type = Event::MouseMoved;
    mouseEvent.mouseMove.x = eventData->cur.output.x;
    mouseEvent.mouseMove.y = eventData->cur.output.y;

    sf::Event touchEvent;
    touchEvent.type = Event::TouchMoved;
    touchEvent.touch.finger = 0;
    touchEvent.touch.x = eventData->cur.output.x;
    touchEvent.touch.y = eventData->cur.output.y;

    Lock lock(mutex);
    systemEvents.push(mouseEvent);
    systemEvents.push(touchEvent);
}


////////////////////////////////////////////////////////////
void WindowImplTizen::processTouchDownEvent(void* data, Evas* evas, void* eventInfo)
{
    //Evas_Event_Multi_Up* data = static_cast<Evas_Event_Multi_Up*>(event_info);

    //sf::Event event;
    //event.type = Event::TouchBegan;
    ////event.mouseMove.button = data->button;
    ////event.mouseMove.x = data->output.x;
    ////event.mouseMove.y = data->output.y;

    //Lock lock(mutex);
    //systemEvents.push(event);
}


////////////////////////////////////////////////////////////
void WindowImplTizen::processTouchUpEvent(void* data, Evas* evas, void* eventInfo)
{
    err() << "processTouchUpEvent" << std::endl;
}


////////////////////////////////////////////////////////////
void WindowImplTizen::processTouchMoveEvent(void* data, Evas* evas, void* eventInfo)
{
    err() << "processTouchMoveEvent" << std::endl;
}


////////////////////////////////////////////////////////////
void WindowImplTizen::processKeyDownEvent(void* data, Evas* evas, void* eventInfo)
{
    err() << "processKeyDownEvent" << std::endl;
}


////////////////////////////////////////////////////////////
void WindowImplTizen::processKeyUpEvent(void* data, Evas* evas, void* eventInfo)
{
    err() << "processKeyUpEvent" << std::endl;
}

} // namespace priv
} // namespace sf
