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

#ifndef _IOT_CAPS_HELPER_SAFE_STATE_
#define _IOT_CAPS_HELPER_SAFE_STATE_

#include "iot_caps_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CAP_ENUM_SAFESTATE_STATE_VALUE_FINGER_REGISTERED,
    CAP_ENUM_SAFESTATE_STATE_VALUE_FINGER_DELETED,
    CAP_ENUM_SAFESTATE_STATE_VALUE_FINGER_INVALID,
    CAP_ENUM_SAFESTATE_STATE_VALUE_PW_REGISTERED,
    CAP_ENUM_SAFESTATE_STATE_VALUE_PW_DELETED,
    CAP_ENUM_SAFESTATE_STATE_VALUE_PW_INVALID,
    CAP_ENUM_SAFESTATE_STATE_VALUE_DUAL_SET,
    CAP_ENUM_SAFESTATE_STATE_VALUE_DUAL_UNSET,
    CAP_ENUM_SAFESTATE_STATE_VALUE_MULTI_SET,
    CAP_ENUM_SAFESTATE_STATE_VALUE_MULTI_UNSET,
    CAP_ENUM_SAFESTATE_STATE_VALUE_DOOR_OPEN_ERR,
    CAP_ENUM_SAFESTATE_STATE_VALUE_DOOR_CLOSE_ERR,
    CAP_ENUM_SAFESTATE_STATE_VALUE_DOOR_STILL_OPEN,
    CAP_ENUM_SAFESTATE_STATE_VALUE_DOOR_OPEN,
    CAP_ENUM_SAFESTATE_STATE_VALUE_DOOR_CLOSED,
    CAP_ENUM_SAFESTATE_STATE_VALUE_PARALYSIS,
    CAP_ENUM_SAFESTATE_STATE_VALUE_PARALYSIS_CLEAR,
    CAP_ENUM_SAFESTATE_STATE_VALUE_MAX
};

const static struct iot_caps_safeState {
    const char *id;
    const struct safeState_attr_safe {
        const char *name;
        const unsigned char property;
        const unsigned char valueType;
        const char *values[CAP_ENUM_SAFESTATE_STATE_VALUE_MAX];
        const char *value_finger_registered;
        const char *value_finger_deleted;
        const char *value_finger_invalid;
        const char *value_pw_registered;
        const char *value_pw_deleted;
        const char *value_pw_invalid;
        const char *value_dual_set;
        const char *value_dual_unset;
        const char *value_multi_set;
        const char *value_multi_unset;
        const char *value_open_err;
        const char *value_close_err;
        const char *value_still_open;
        const char *value_open;
        const char *value_closed;
        const char *value_paralysis;
        const char *value_paralysis_clear;
    } attr_safe;
} caps_helper_safeState = {
    .id = "stse.safeState",
    .attr_safe = {
        .name = "safeState",
        .property = ATTR_SET_VALUE_REQUIRED,
        .valueType = VALUE_TYPE_STRING,
        .values = {"fingerprintRegistered", \
        "fingreprintDeleted", \
        "unregisteredFingerprint", \
        "passwordRegistered", \
        "passwordChanged", \
        "unregisteredPassword", \
        "dualcodeSet", \
        "dualcodeUnset", \
        "multiuserSet", \
        "multiuserUnset", \
        "doorOpenErr", \
        "doorCloseErr", \
        "doorStillOpen", \
        "doorOpen", \
        "doorClosed", \
        "temporaryLock", \
        "temporaryLockClear"},
        .value_finger_registered = "fingerprintRegistered",
        .value_finger_deleted = "fingreprintDeleted",
        .value_finger_invalid = "unregisteredFingerprint",
        .value_pw_registered = "passwordRegistered",
        .value_pw_deleted = "passwordChanged",
        .value_pw_invalid = "unregisteredPassword",
        .value_dual_set = "dualcodeSet",
        .value_dual_unset = "dualcodeUnset",
        .value_multi_set = "multiuserSet",
        .value_multi_unset = "multiuserUnset",
        .value_open_err = "doorOpenErr",
        .value_close_err = "doorCloseErr",
        .value_still_open = "doorStillOpen",
        .value_open = "doorOpen",
        .value_closed = "doorClosed",
        .value_paralysis = "temporaryLock",
        .value_paralysis_clear = "temporaryLockClear"
    },
};

#ifdef __cplusplus
}
#endif

#endif /* _IOT_CAPS_HELPER_SAFE_STATE_ */
