/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/


#pragma once

#include "config.h"

// handler function which being called when config param changed
// and executes the appropriate logic needed by the change.
// might be called at any time after config_init finished
void reconf_handler(Config_Option_Field type);
