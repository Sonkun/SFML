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
#include <SFML/Window/Tizen/EvasContext.hpp>
#include <SFML/Window/Tizen/EvasCheck.hpp>
#include <SFML/Window/Tizen/WindowImplTizen.hpp>
#include <SFML/Window/Tizen/ApplicationState.hpp>
#include <SFML/Window/Export.hpp>
#include <SFML/System/Lock.hpp>
#include <SFML/System/Err.hpp>

// Declare Evas GL API (this is where the symbol resides, in EvasContext.cpp)
SFML_WINDOW_API EVAS_GL_GLOBAL_GLES1_DEFINE();

namespace
{
    Evas_GL* evasgl = NULL;
}

namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
EvasContext::EvasContext(EvasContext* shared) :
m_context (NULL),
m_surface (NULL),
m_config  (NULL)
{    
    priv::ApplicationState* state = priv::getApplicationState();
    Lock(state->mutex);

    // Evas GL is shared from the main thread ( this causes one Evas GL error when getting config, but...)
    if (!evasgl)
        evasgl = state->evasgl;

    m_config = getBestConfig(VideoMode::getDesktopMode().bitsPerPixel, ContextSettings());
    m_config->color_format = EVAS_GL_NO_FBO;
    
    updateSettings();
    
    m_surface = evasCheck(evas_gl_pbuffer_surface_create(evasgl, m_config, 1, 1, NULL), evasgl);
    
	m_context = evasCheck(evas_gl_context_version_create(evasgl, state->sharedContext, EVAS_GL_GLES_1_X), evasgl);   
}


////////////////////////////////////////////////////////////
EvasContext::EvasContext(EvasContext* shared, const ContextSettings& settings, const WindowImpl* owner, unsigned int bitsPerPixel) :
m_context (NULL),
m_surface (NULL),
m_config  (NULL)
{    
    priv::ApplicationState* state = priv::getApplicationState();
    Lock(state->mutex);
    
    state->context = this;

    m_config = getBestConfig(bitsPerPixel, settings);
    updateSettings();

    m_context = evasCheck(evas_gl_context_version_create(evasgl, shared->m_context, EVAS_GL_GLES_1_X), evasgl);

    // Initially, the window is 1x1 (the surface is recreated later)
    createSurface(static_cast<Evas_Object*>(owner->getSystemHandle()), 1, 1);

    // The window with its Open GL context is initialized by then
    state->initialized = true;
}


////////////////////////////////////////////////////////////
EvasContext::EvasContext(EvasContext* shared, const ContextSettings& settings, unsigned int width, unsigned int height) :
m_context (NULL),
m_surface (NULL),
m_config  (NULL)
{
}


////////////////////////////////////////////////////////////
EvasContext::~EvasContext()
{
    // Deactivate the current context
    Evas_GL_Context* currentContext = evasCheck(evas_gl_current_context_get(evasgl), evasgl);

    if (currentContext == m_context)
    {
        evasCheck(evas_gl_make_current(evasgl, NULL, NULL), evasgl);
    }

    // Destroy context
    if (m_context != NULL)
    {
        evasCheck(evas_gl_context_destroy(evasgl, m_context), evasgl);
    }

    // Destroy surface
    if (m_surface)
    {
        evasCheck(evas_gl_surface_destroy(evasgl, m_surface), evasgl);
    }

    // Destroy config
    if (m_config)
    {
        evasCheck(evas_gl_config_free(m_config), evasgl);
    }
}


////////////////////////////////////////////////////////////
bool EvasContext::makeCurrent(bool current)
{    
    if (current)
        return m_surface != NULL && evasCheck(evas_gl_make_current(evasgl, m_surface, m_context), evasgl);

    return m_surface != NULL && evasCheck(evas_gl_make_current(evasgl, NULL, NULL), evasgl);
}


////////////////////////////////////////////////////////////
void EvasContext::display()
{
    // Transparently done by Evas
}


////////////////////////////////////////////////////////////
void EvasContext::setVerticalSyncEnabled(bool enabled)
{
    // Transparently done by Ecore and Evas
}


////////////////////////////////////////////////////////////
void EvasContext::createSurface(Evas_Object* window, int width, int height)
{
    m_surface = evasCheck(evas_gl_surface_create(evasgl, m_config, width, height), evasgl);

    setActive(true);
    
    Evas_Native_Surface ns;
    evasCheck(evas_gl_native_surface_get(evasgl, m_surface, &ns), evasgl);
    
    evasCheck(evas_object_image_native_surface_set(window, &ns), evasgl);
}


////////////////////////////////////////////////////////////
void EvasContext::destroySurface()
{
    // Leave the following line commented, otherwise it breaks rendering (bug in Tizen ?)
    //evasCheck(evas_gl_surface_destroy(evasgl, m_surface), evasgl);
    m_surface = NULL;
    
    // Ensure that this context is no longer active since our surface is now destroyed
    setActive(false);
}


////////////////////////////////////////////////////////////
Evas_GL_Config* EvasContext::getBestConfig(unsigned int bitsPerPixel, const ContextSettings& settings)
{
    // With Evas GL there's no such thing as available configurations. Instead, we
    // simply try to match as closely as possible the user's settings.
	Evas_GL_Config* config = evasCheck(evas_gl_config_new(), evasgl);

    // Set pixel depth value (must be either 24 or 32)
    if (bitsPerPixel == 24)
        config->color_format = EVAS_GL_RGB_888;
    else if (bitsPerPixel == 32)
        config->color_format = EVAS_GL_RGBA_8888;
    else
    {
        err() << "Pixel depth (bits per pixel) must be either 24 or 32 on Tizen; fall back on 24" << std::endl;
        config->color_format = EVAS_GL_RGB_888;
    }

    // Set depth bits value
    if (settings.depthBits == 0)
        config->depth_bits = EVAS_GL_DEPTH_NONE;
    else if (settings.depthBits == 8)
        config->depth_bits = EVAS_GL_DEPTH_BIT_8;
    else if (settings.depthBits == 16)
        config->depth_bits = EVAS_GL_DEPTH_BIT_16;
    else if (settings.depthBits == 24)
        config->depth_bits = EVAS_GL_DEPTH_BIT_24;
    else if (settings.depthBits == 32)
        config->depth_bits = EVAS_GL_DEPTH_BIT_32;
    else
    {
        err() << "Depth bits must be either 0, 8, 16, 24 or 32 on Tizen; fall back on 0" << std::endl;
        config->depth_bits = EVAS_GL_DEPTH_NONE;
    }
    
    // Set stencil bits value
    if (settings.stencilBits == 0)
        config->stencil_bits = EVAS_GL_STENCIL_NONE;
    else if (settings.stencilBits == 1)
        config->stencil_bits = EVAS_GL_STENCIL_BIT_1;
    else if (settings.stencilBits == 2)
        config->stencil_bits = EVAS_GL_STENCIL_BIT_2;
    else if (settings.stencilBits == 4)
        config->stencil_bits = EVAS_GL_STENCIL_BIT_4;
    else if (settings.stencilBits == 8)
        config->stencil_bits = EVAS_GL_STENCIL_BIT_8;
    else if (settings.stencilBits == 16)
        config->stencil_bits = EVAS_GL_STENCIL_BIT_16;
    else
    {
        err() << "Stencil bits must be either 0, 1, 2, 4, 8 or 16 on Tizen; fall back on 0" << std::endl;
        config->stencil_bits = EVAS_GL_STENCIL_NONE;
    }

    // Set the anti-aliasing level. Note that this isn't really accurate given
    // Evas GL express it relatively to the total hardware capability.
    if (settings.antialiasingLevel == 0)
        config->multisample_bits = EVAS_GL_MULTISAMPLE_NONE;
    else if (settings.antialiasingLevel == 2)
        config->multisample_bits = EVAS_GL_MULTISAMPLE_LOW;
    else if (settings.antialiasingLevel == 4)
        config->multisample_bits = EVAS_GL_MULTISAMPLE_MED;
    else if (settings.antialiasingLevel == 8)
        config->multisample_bits = EVAS_GL_MULTISAMPLE_HIGH;
    else
    {
        err() << "Anti-aliasing level must be either 0, 2, 4 or 8 on Tizen; fall back on 0" << std::endl;
        config->multisample_bits = EVAS_GL_MULTISAMPLE_NONE;
    }

	config->options_bits = EVAS_GL_OPTIONS_NONE;
    
    return config;
}


////////////////////////////////////////////////////////////
void EvasContext::updateSettings()
{
    if (m_config->depth_bits == EVAS_GL_DEPTH_NONE)
        m_settings.depthBits = 0;
    else if (m_config->depth_bits == EVAS_GL_DEPTH_BIT_8)
        m_settings.depthBits = 8;
    else if (m_config->depth_bits == EVAS_GL_DEPTH_BIT_16)
        m_settings.depthBits = 16;
    else if (m_config->depth_bits == EVAS_GL_DEPTH_BIT_24)
        m_settings.depthBits = 24;
    else if (m_config->depth_bits == EVAS_GL_DEPTH_BIT_32)
        m_settings.depthBits = 32;
        
    if (m_config->stencil_bits == EVAS_GL_STENCIL_NONE)
        m_settings.stencilBits = 0;
    else if (m_config->stencil_bits == EVAS_GL_STENCIL_BIT_1)
        m_settings.stencilBits = 1;
    else if (m_config->stencil_bits == EVAS_GL_STENCIL_BIT_2)
        m_settings.stencilBits = 2;
    else if (m_config->stencil_bits == EVAS_GL_STENCIL_BIT_4)
        m_settings.stencilBits = 4;
    else if (m_config->stencil_bits == EVAS_GL_STENCIL_BIT_8)
        m_settings.stencilBits = 8;
    else if (m_config->stencil_bits == EVAS_GL_STENCIL_BIT_16)
        m_settings.stencilBits = 16;

    if (m_config->multisample_bits == EVAS_GL_MULTISAMPLE_NONE)
        m_settings.antialiasingLevel = 0;
    else if (m_config->multisample_bits == EVAS_GL_MULTISAMPLE_LOW)
        m_settings.antialiasingLevel = 2;
    else if (m_config->multisample_bits == EVAS_GL_MULTISAMPLE_MED)
        m_settings.antialiasingLevel = 4;
    else if (m_config->multisample_bits == EVAS_GL_MULTISAMPLE_HIGH)
        m_settings.antialiasingLevel = 8;
        
    m_settings.majorVersion = 1;
    m_settings.minorVersion = 1;
    
    m_settings.attributeFlags = ContextSettings::Default;
}


} // namespace priv

} // namespace sf
