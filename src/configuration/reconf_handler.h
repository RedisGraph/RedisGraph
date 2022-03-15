/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "config.h"

// handler function invoked when configuration changes
// might be called at any time after config_init
void reconf_handler(Config_Option_Field type);

