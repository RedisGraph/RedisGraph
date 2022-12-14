/*
 * Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of NVIDIA CORPORATION nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Specializations for different atomic operations on different types
 */

#pragma once

// TODO: These should really be pre-compiled into the graphblascuda binary

template <typename T>
__device__ void atomic_add(T* ptr, T val);

template<> __device__ __inline__ void atomic_add<int>(int* ptr, int val) { atomicAdd(ptr, val); }
template<> __device__ __inline__ void atomic_add<int64_t>(int64_t* ptr, int64_t val) { atomicAdd((unsigned long long*)ptr, (unsigned long long)val); }
template<> __device__ __inline__ void atomic_add<float>(float* ptr, float val) { atomicAdd(ptr, val); }
template<> __device__ __inline__ void atomic_add<double>(double* ptr, double val) { atomicAdd(ptr, val); }


template <typename T>
__device__ void atomic_max(T* ptr, T val);

template<> __device__ __inline__ void atomic_max<int>(int* ptr, int val) { atomicMax(ptr, val); }
template<> __device__ __inline__ void atomic_max<int64_t>(int64_t* ptr, int64_t val) { atomicMax((unsigned long long*)ptr, (unsigned long long)val); }

template <typename T>
__device__ void atomic_min(T* ptr, T val);

template<> __device__ __inline__ void atomic_min<int>(int* ptr, int val) { atomicMin(ptr, val); }
template<> __device__ __inline__ void atomic_min<int64_t>(int64_t* ptr, int64_t val) { atomicMin((unsigned long long*)ptr, (unsigned long long)val); }


template <typename T>
__device__ void atomic_sub(T* ptr, T val);

template<> __device__ __inline__ void atomic_sub<int>(int* ptr, int val) { atomicSub(ptr, val); }
