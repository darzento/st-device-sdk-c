/* ***************************************************************************
 *
 * Copyright 2019-2020 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

#ifndef _IOT_CAPS_HELPER_LOCK_ALARM_
#define _IOT_CAPS_HELPER_LOCK_ALARM_

#include "iot_caps_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CAP_ENUM_LOCKALARM_ALARM_VALUE_CLEAR,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_LOCKRESET,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_DAMAGED,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_FORCEDOPEN,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_UNABLELOCK,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_NOTCLOSE,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_HIGHTEMP,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_ATTEMPTSEXCEEDED,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_PHYSICALIMPACT,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_FAILEDOPENING,
    CAP_ENUM_LOCKALARM_ALARM_VALUE_MAX
};

const static struct iot_caps_lockAlarm {
    const char *id;
    const struct lockAlarm_attr_alarm {
        const char *name;
        const unsigned char property;
        const unsigned char valueType;
        const char *values[CAP_ENUM_LOCKALARM_ALARM_VALUE_MAX];
        const char *value_clear;
        const char *value_lock_reset;
        const char *value_damaged;
        const char *value_forced_open;
        const char *value_unable_lock;
        const char *value_not_close;
        const char *value_high_temp;
        const char *value_attempts_exceeded;
        const char *value_physical_impact;
        const char *value_failed_opening;
    } attr_alarm;
} caps_helper_lockAlarm = {
    .id = "lockAlarm",
    .attr_alarm = {
        .name = "alarm",
        .property = ATTR_SET_VALUE_REQUIRED,
        .valueType = VALUE_TYPE_STRING,
        .values = {"clear", "lockFactoryReset", "damaged", "forcedOpeningAttempt","unableToLockTheDoor","notClosedForALongTime","highTemperature","attemptsExceeded","physicalImpact","failedOpeningAttempt"},
        .value_clear = "clear",
        .value_lock_reset = "lockFactoryReset",
        .value_damaged = "damaged",
        .value_forced_open = "forcedOpeningAttempt",
        .value_unable_lock = "unableToLockTheDoor",
        .value_not_close = "notClosedForALongTime",
        .value_high_temp = "highTemperature",
        .value_attempts_exceeded = "attemptsExceeded",
        .value_physical_impact = "physicalImpact",
        .value_failed_opening = "failedOpeningAttempt",
    },
};

#ifdef __cplusplus
}
#endif

#endif /* _IOT_CAPS_HELPER_LOCK_ALARM_ */
