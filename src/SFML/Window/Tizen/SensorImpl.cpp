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
#include <SFML/Window/SensorImpl.hpp>


namespace
{
    sf::Vector3f sensorData[sf::Sensor::Count];
}

namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
void SensorImpl::initialize()
{
    // Nothing to do
}


////////////////////////////////////////////////////////////
void SensorImpl::cleanup()
{
    // Nothing to do
}


////////////////////////////////////////////////////////////
bool SensorImpl::isAvailable(Sensor::Type sensor)
{
    bool supported;
    sensor_is_supported(SENSOR_ACCELEROMETER, &supported);

    return supported;
    //sensor_type_e sensors[Sensor::Count];
    
    //sensors[Accelerometer] = SENSOR_ACCELEROMETER;
    //sensors[Gyroscope] = SENSOR_GYROSCOPE;
    //sensors[Magnetometer] = SENSOR_MAGNETIC;
    //sensors[Gravity] = SENSOR_GRAVITY;
    //sensors[UserAcceleration] = SENSOR_LINEAR_ACCELERATION;
    //sensors[Orientation] = SENSOR_ORIENTATION; 
}


////////////////////////////////////////////////////////////
bool SensorImpl::open(Sensor::Type sensor)
{
    //// Retrieve the default sensor for this specific type (might be more than one)
    //int ret = sensor_get_default_sensor(sensors[sensor], &m_handle);

    //if (ret != 0)
    //{
        //sf::err() << "Failed to retrieve the default sensor (is available?)" << std::endl;
        //return true;
    //}

    //// Create a listener handling
    //ret = sensor_create_listener(m_handle, &m_listener);

    //if (ret != 0)
    //{
        //sf::err() << "Failed to create the listener" << std::endl;
        //return true;
    //}
    
    //// Register callback
    //sensor_listener_set_event_cb(m_listener, 100, processEvent, NULL);
    ////sensor_listener_set_attribute_int(listener, SENSOR_ATTRIBUTE_PAUSE_POLICY, SENSOR_PAUSE_NONE);

    //m_index = static_cast<unsigned int>(sensor);
    
    return false;
}


////////////////////////////////////////////////////////////
void SensorImpl::close()
{
    //sensor_destroy_listener(m_listener);
}


////////////////////////////////////////////////////////////
Vector3f SensorImpl::update()
{
    //return sensorData[m_index];
    return sf::Vector3f(0, 0, 0);
}


////////////////////////////////////////////////////////////
void SensorImpl::setEnabled(bool enabled)
{
    //if (enabled)
        //sensor_listener_start(m_listener);
    //else
        //sensor_listener_stop(m_listener);
}


////////////////////////////////////////////////////////////
void processEvent(sensor_h sensor, sensor_event_s *event, void *user_data)
{
    ///* If a callback function is used to listen to different sensor types, it can check the sensor type */
    //sensor_type_e type;
    //sensor_get_type(sensor, &type);

    //sf::Sensor::Type index;
    //sf::Vector3f data;
    
    //switch (type)
    //{
        //case SENSOR_ACCELEROMETER:
            //index = Sensor::Accelerometer;
            //data.x = event->values[0];
            //data.y = event->values[1];
            //data.z = event->values[2];
            //break;
            
        //case SENSOR_GYROSCOPE:
            //index = Sensor::Gyroscope;
            //data.x = event->values[0];
            //data.y = event->values[1];
            //data.z = event->values[2];
            //break;
            
        //case SENSOR_MAGNETIC:
            //index = Sensor::Magnetometer;
            //data.x = event->values[0];
            //data.y = event->values[1];
            //data.z = event->values[2];
            //break;
            
        //case SENSOR_GRAVITY:
            //index = Sensor::Gravity;
            //data.x = event->values[0];
            //data.y = event->values[1];
            //data.z = event->values[2];
            //break;
            
        //case SENSOR_LINEAR_ACCELERATION:
            //index = Sensor::UserAcceleration;
            //data.x = event->values[0];
            //data.y = event->values[1];
            //data.z = event->values[2];
            //break;
            
        //case SENSOR_ORIENTATION:
            //index = Sensor::Orientation;
            //data.x = event->values[0];
            //data.y = event->values[1];
            //data.z = event->values[2];
            //break; 
    //}

    ////// An unknown sensor event has been detected, we don't know how to process it
    ////if (type == Sensor::Count)
        ////continue;

    //sensorData[index] = data;
}

} // namespace priv

} // namespace sf
