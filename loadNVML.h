/*
 * Copyright (c) 2020-2021, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2020-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
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