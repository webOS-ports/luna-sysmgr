/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
*      Copyright (c) 2023 Herman van Hazendonk <github.com@herrie.org>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */




#include "AmbientLightSensor.h"

#include "Common.h"
#include "HostBase.h"
#include "JSONUtils.h"
#include "Settings.h"
#include "SystemService.h"
#include "Time.h"

#include <json.h>
#include <glib.h>
#if defined(HAS_LUNA_PREF)
#include <lunaprefs.h>
#endif
#include <luna-service2/lunaservice.h>

#define AMBIENT_LIGHT_SENSOR_ID "com.palm.ambientLightSensor"

#define INTERVAL_SLOW  (1000)

#define INTERVAL_FAST (100)

#define ALS_CALIBRATION_TOKEN   "com.palm.properties.ALSCal"

AmbientLightSensor* AmbientLightSensor::m_instance = NULL;

/*! \page com_palm_ambient_light_sensor_control Service API com.palm.ambientLightSensor/control/
 *  Public methods:
 *  - \ref com_palm_ambient_light_sensor_control_status
 */
static LSMethod alsMethods[] = {
    {"status", AmbientLightSensor::controlStatus},
    {},
};

AmbientLightSensor::AmbientLightSensor ()
    : m_service(NULL)
    , m_alsEnabled(false)
    , m_alsIsOn(false)
    , m_alsRegion(ALS_REGION_UNDEFINED)
    , m_alsLastOff(0)
    , m_alsDisplayOn(false)
    , m_alsSubscriptions(0)
    , m_alsDisabled(0)
    , m_alsHiddOnline(false)
    , m_als (0)
{
    LSError lserror;
    LSErrorInit(&lserror);
    bool result;

    GMainLoop* mainLoop = HostBase::instance()->mainLoop();

    result = LSRegister(AMBIENT_LIGHT_SENSOR_ID, &m_service, &lserror);
    if (!result)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree(&lserror);
    }

    result = LSRegisterCategory (m_service, "/control", alsMethods, NULL, NULL, &lserror);
    if (!result)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree(&lserror);
    }

    result = LSCategorySetData (m_service, "/control", this, &lserror);
    if (!result)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree(&lserror);
    }

    result = LSSubscriptionSetCancelFunction(m_service, AmbientLightSensor::cancelSubscription, this, &lserror);
    if (!result)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree (&lserror);
    }

    result = LSGmainAttach(m_service, mainLoop, &lserror);
    if (!result)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree(&lserror);
    }

    result = LSRegisterServerStatusEx(m_service, "com.palm.hidd", AmbientLightSensor::hiddServiceNotification, this, NULL, &lserror);
    if (!result)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree (&lserror);
    }

    if (Settings::LunaSettings()->enableAls) {
    m_alsEnabled = true; 

    m_als = new QAmbientLightSensor();
    connect(m_als, SIGNAL(readingChanged()), this, SLOT(slotReadingChanged()));
    }
    else {
        g_warning ("%s: ALS is not enabled", __PRETTY_FUNCTION__); 

    }

    m_instance = this;

    g_debug ("%s started", __PRETTY_FUNCTION__);
}

AmbientLightSensor* AmbientLightSensor::instance (void)
{
    return AmbientLightSensor::m_instance;
}

AmbientLightSensor::~AmbientLightSensor()
{    
    LSError lserror;
    LSErrorInit(&lserror);
    bool result;

    result = LSUnregister(m_service, &lserror);
    if (!result)
    {
        g_message ("%s: failed at %s with message %s", __PRETTY_FUNCTION__, lserror.func, lserror.message);
        LSErrorFree(&lserror);
    }
    if (m_als)
        m_als->deleteLater();
}

void AmbientLightSensor::slotReadingChanged ()
{
    QAmbientLightReading *reading = m_als->reading();
    update(static_cast<int>(reading->lightLevel()));
    return;
}

int AmbientLightSensor::getCurrentRegion ()
{
    return m_alsRegion;
}

void AmbientLightSensor::setCurrentRegion (int newRegion)
{
    if (m_alsRegion != newRegion) {
        m_alsRegion = newRegion;
        Q_EMIT currentRegionChanged(m_alsRegion);
    }
}

bool AmbientLightSensor::start ()
{
    m_alsDisplayOn = true;
    return on ();
}

bool AmbientLightSensor::stop ()
{
    m_alsDisplayOn = false;
    return off ();
}

bool AmbientLightSensor::on ()
{
    if (Settings::LunaSettings()->hardwareType != Settings::HardwareTypeDevice)
        return true;

    LSError lserror;
    LSErrorInit(&lserror);
    bool result;

    // if display is off do not bother to enable the 
    // als sensor
    if (!m_alsDisplayOn)
        return true;

    // if it is already one do not bother to enable it
    if (m_alsIsOn)
        return true;

    // if we are not calibrated and there are no subscriptions
    // do not enable it
    if (!m_alsEnabled)
        return true;

    m_alsIsOn = true;

    int timeSinceLastReading = Time::curTimeMs() - m_alsLastOff;

    setCurrentRegion(ALS_REGION_INDOOR);

    if (NULL != m_als)
    {
        g_debug ("%s: ALS on!", __PRETTY_FUNCTION__);
        return m_als->start();
    }
    return true;
}

bool AmbientLightSensor::off ()
{
    LSError lserror;
    LSErrorInit(&lserror);
    bool result;

    if (!m_alsIsOn)
        return true;

    if (m_alsEnabled)
    {
        if (m_alsDisplayOn)
            return true;
    }
    else
    {
        if (m_alsDisplayOn && m_alsSubscriptions > 0)
            return true;
    }

    m_alsIsOn = false;

    m_alsLastOff = Time::curTimeMs();

    if (NULL != m_als)
    {
        g_debug ("%s: ALS off!", __PRETTY_FUNCTION__);
        m_als->stop();
    }
    return true;
}

bool AmbientLightSensor::update (int lightLevel)
{
    if (Settings::LunaSettings()->enableAls)
        return updateAls (lightLevel);
    else 
        return false;
}

bool sortIncr (int32_t alsVal1, int32_t alsVal2) 
{
    if (alsVal1 > alsVal2)
    return false;
    return true;
}

// this allows the als region to move directly to the current light condition.

bool AmbientLightSensor::updateAls(int lightLevel)
{
    if (Settings::LunaSettings()->hardwareType != Settings::HardwareTypeDevice)
        return false;

    LSError lserror;
    LSErrorInit(&lserror);
    bool result = true;

    int current = m_alsRegion;

    if (m_alsDisabled > 0) {
        setCurrentRegion(ALS_REGION_UNDEFINED);
        g_debug(
                "%s: reported light level of %d [region set to default by subscription]",
                __PRETTY_FUNCTION__, lightLevel);

        goto end;
    }

    if (!m_alsEnabled) {
        setCurrentRegion(ALS_REGION_UNDEFINED);
        g_debug("%s: reported light level of %d [device not calibrated]",
                __PRETTY_FUNCTION__, lightLevel);

        goto end;
    }

    if (lightLevel < 0) {
        g_warning("%s: invalid lightLevel %d", __PRETTY_FUNCTION__, lightLevel);
        return false;
    }

    if (m_alsRegion < ALS_REGION_UNDEFINED || m_alsRegion > ALS_REGION_SUNNY) {
        g_warning("%s: current region is invalid, resetting to indoor",
                __PRETTY_FUNCTION__);
        setCurrentRegion(ALS_REGION_INDOOR);
    }

    //FIXME: Is this really correct?
    setCurrentRegion(lightLevel);

end:

    if (m_alsSubscriptions > 0) {
        gchar *status = g_strdup_printf(
                "{\"returnValue\":true,\"region\":%i}",
                m_alsRegion);

        if (NULL != status)
            result = LSSubscriptionReply(m_service, "/control/status", status,
                    &lserror);
        if (!result) {
            LSErrorPrint(&lserror, stderr);
            LSErrorFree(&lserror);
        }
        g_free(status);
    }

    // if there was no change return false, no need to update anything
    return (m_alsRegion != current);
}

/*!
\page com_palm_ambient_light_sensor_control
\n
\section com_palm_ambient_light_sensor_control_status status

\e Public.

com.palm.ambientLightSensor/control/status

Get status and optionally enable or disable the ambient light sensor.

\subsection com_palm_ambient_light_sensor_control_status_syntax Syntax:
\code
{
    "subscribe": boolean,
    "disableALS": boolean
}
\endcode

\param subscribe Set to true to receive status updates.
\param disableALS If \e subscribe is set to true, set this to true to disable the ambient light sensor.

\subsection com_palm_ambient_light_sensor_control_status_returns_call Returns for a call:
\code
{
    "returnValue": boolean,
    "region": int,
    "disabled": boolean,
    "subscribed": boolean
}
\endcode

\param returnValue Indicates if the call was succesful.
\param region Current value of the ambient light sensor.
\param disabled True if ambient light sensor is disabled.
\param subscribed True if subscribed to receive status updates.

\subsection com_palm_ambient_light_sensor_control_status_returns_status Returns for status updates:
\code
{
    "returnValue": boolean,
    "region": int
}
\endcode

\param returnValue Indicates if the call was succesful.
\param region A value between 0-5 describing the amount of ambient light:
\li 0: Undefined, when the sensor is disabled.
\li 1: Dark
\li 2: Dim
\li 3: Indoor
\li 4: Outdoor
\li 5: Sunny

\subsection com_palm_ambient_light_sensor_control_status_examples Examples:
\code
luna-send -n 1 -f luna://com.palm.ambientLightSensor/control/status '{ "subscribe": true, "disableALS": false }'
\endcode

Example response for a succesful call:
\code
{
    "returnValue": true,
    "region": 1,
    "disabled": true,
    "subscribed": true
}
\endcode

Example status updates:
\code
{
    "returnValue": true,
    "region": 3
}
{
    "returnValue": true,
    "region": 2
}
{
    "returnValue": true,
    "region": 1
}
{
    "returnValue": true,
    "region": 3
}
\endcode
*/
bool AmbientLightSensor::controlStatus(LSHandle *sh, LSMessage *message, void *ctx)
{
    if (Settings::LunaSettings()->hardwareType != Settings::HardwareTypeDevice)
        return true;

    LSError lserror;
    LSErrorInit(&lserror);
    bool result = true;

    AmbientLightSensor *als = (AmbientLightSensor*)ctx;

    // {"subscribe":boolean, "disableALS" : boolean}
    VALIDATE_SCHEMA_AND_RETURN(sh,
                               message,
                               SCHEMA_2(REQUIRED(subscribe, boolean), REQUIRED(disableALS, boolean)));

    g_debug ("%s: received '%s;", __PRETTY_FUNCTION__, LSMessageGetPayload(message));

    bool subscribed = false;

    result = LSSubscriptionProcess (sh, message, &subscribed, &lserror);
    if(!result)
    {
        LSErrorFree (&lserror);
        result = true;
        subscribed = false;
    }

    if (subscribed)
    {
        als->m_alsSubscriptions++;
        als->on ();

        bool disable = false;
        const char* str = LSMessageGetPayload(message);
        if (str) {
            json_object* root = json_tokener_parse(str);
            if (root) {
                result = true;
                disable = json_object_get_boolean(json_object_object_get(root, "disableALS"));
                json_object_put(root);
            }
        }

        if (disable)
            als->m_alsDisabled++;
    }

    gchar *status = g_strdup_printf ("{\"returnValue\":true,\"current\":%i,\"disabled\":%s,\"subscribed\":%s}",
            als->m_alsRegion, als->m_alsDisabled > 0 ? "true" : "false",
            subscribed ? "true" : "false");

    if (NULL != status)
        result = LSMessageReply(sh, message, status, &lserror);
    if(!result)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree (&lserror);
    }

    g_free(status);
    return true;
}

bool AmbientLightSensor::cancelSubscription(LSHandle *sh, LSMessage *message, void *ctx)
{
    bool result = false;

    g_debug ("%s: category %s, method %s", __FUNCTION__, LSMessageGetCategory(message), LSMessageGetMethod(message));
    AmbientLightSensor *als = (AmbientLightSensor *)ctx;

    if (0 == strcmp (LSMessageGetMethod(message), "status") &&
        0 == strcmp (LSMessageGetCategory(message), "/control"))
    {
        als->m_alsSubscriptions--;
        if (als->m_alsSubscriptions == 0)
        {
            bool disable = false;
            const char* str = LSMessageGetPayload(message);
            if (str) {
                json_object* root = json_tokener_parse(str);
                if (root) {
                    result = true;
                    disable = json_object_get_boolean(json_object_object_get(root, "disableALS"));
                    json_object_put(root);
                }
            }
            if (result && disable)
                als->m_alsDisabled--;
            als->off ();
        }
    }
    return true;
}

bool AmbientLightSensor::hiddServiceNotification(LSHandle *sh, const char *serviceName, bool connected, void *ctx)
{
    LSError lserror;
    LSErrorInit(&lserror);

    AmbientLightSensor *als = (AmbientLightSensor *)ctx;

    als->m_alsHiddOnline = connected;

    g_debug ("%s: received conntection for '%s' and status is %s", __PRETTY_FUNCTION__, serviceName, connected ? "up" : "down");

    return true;
}

