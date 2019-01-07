/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _ALGORITHMS_H_
#define _ALGORITHMS_H_

// Function pointer to algorithm.
typedef int (*fpAlgorithmRun)(int argc, const char *argv[]);

typedef struct {
    const char *name;           // Name of algorithm.
    fpAlgorithmRun handler;     // Algorithm entry point.
} AlgorithmCtx;

// Register algorithm.
int Algorithms_Register(const char *name, fpAlgorithmRun fp);

// Get algorithm run function pointer.
fpAlgorithmRun Algorithms_Get(const char *name);

#endif
