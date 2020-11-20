/* Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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

#pragma once

// Loads imports from NVML on Windows (returns true on Linux, since NVML is
// already loaded). Returns true if and only if it succeeded.
//
// This is necessary because the DLL depends on the driver version, and the
// preferred place to find nvml.dll is in the Windows DriverStore folder,
// followed by the standard Windows DLL search paths.
//
// In this sample, we use delay-loading (enabled in CMakeLists.txt) to
// postpone loading the DLL until we need it. Usually, this loads the DLL
// the first time a DLL function is called, but since we need to include the
// DriverStore folder, we use this function to specify a search path and
// to load it. (Note that one can also call LoadLibrary/GetProcAddress manually
// - see NVMLImpl.cpp in the OptiX Advanced Samples at
// https://github.com/NVIDIA/OptiX_Apps/blob/master/apps/nvlink_shared/src/NVMLImpl.cpp
// - or write their own helper function, documented at
// https://docs.microsoft.com/en-us/cpp/build/reference/understanding-the-helper-function.
//
bool LoadNVMLImports();