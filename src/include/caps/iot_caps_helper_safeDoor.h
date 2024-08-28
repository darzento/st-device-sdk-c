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

#ifndef _IOT_CAPS_HELPER_SAFE_DOOR_
#define _IOT_CAPS_HELPER_SAFE_DOOR_

#include "iot_caps_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CAP_ENUM_SAFEDOOR_SAFE_VALUE_CLOSED,
    CAP_ENUM_SAFEDOOR_SAFE_VALUE_OPEN,
    CAP_ENUM_SAFEDOOR_SAFE_VALUE_MAX
};

const static struct iot_caps_safeDoor {
    const char *id;
    const struct safeDoor_attr_safe {
        const char *name;
        const unsigned char property;
        const unsigned char valueType;
        const char *values[CAP_ENUM_SAFEDOOR_SAFE_VALUE_MAX];
        const char *value_close;
        const char *value_open;
    } attr_safe;
} caps_helper_safeDoor = {
    .id = "stse.safeDoor",
    .attr_safe = {
        .name = "safeDoor",
        .property = ATTR_SET_VALUE_REQUIRED,
        .valueType = VALUE_TYPE_STRING,
        .values = {"close", "open"},
        .value_close = "close",
        .value_open = "open",
    },
};

#ifdef __cplusplus
}
#endif

#endif /* _IOT_CAPS_HERLPER_CONTACT_SENSOR_ */
