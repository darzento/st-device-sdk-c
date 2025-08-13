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

#ifndef _IOT_CAPS_HELPER_BATTERY_LEVEL_
#define _IOT_CAPS_HELPER_BATTERY_LEVEL_

#include "iot_caps_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CAP_ENUM_BATTERYLEVEL_BATTERYLEVEL_VALUE_NORMAL,
    CAP_ENUM_BATTERYLEVEL_BATTERYLEVEL_VALUE_WARNING,
    CAP_ENUM_BATTERYLEVEL_BATTERYLEVEL_VALUE_CRITICAL,
    CAP_ENUM_BATTERYLEVEL_BATTERYLEVEL_VALUE_MAX,
};

const static struct iot_caps_batteryLevel {
    const char *id;
    const struct batteryLevel_attr_batteryLevel {
        const char *name;
        const unsigned char property;
        const unsigned char valueType;
        const char *values[CAP_ENUM_BATTERYLEVEL_BATTERYLEVEL_VALUE_MAX];
        const char *value_normal;
        const char *value_warning;
        const char *value_critical;
    } attr_batteryLevel;
} caps_helper_batteryLevel = {
    .id = "batteryLevel",
    .attr_batteryLevel = {
        .name = "battery",
        .property = ATTR_SET_VALUE_REQUIRED,
        .valueType = VALUE_TYPE_STRING,
        .values = {"normal", "warning", "critical"},
        .value_normal = "normal",
        .value_warning = "warning",
        .value_critical = "critical",
    },
};

#ifdef __cplusplus
}
#endif

#endif /* _IOT_CAPS_HELPER_BATTERY_LEVEL_ */
