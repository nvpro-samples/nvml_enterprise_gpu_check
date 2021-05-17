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


//-----------------------------------------------------------------------------
// nvml_enterprise_gpu_check
//
// Prints whether the current GPU is an Enterprise/Quadro GPU,
// using nvmlDeviceGetBrand. (This is more robust than
// finding a string in the GPU's name.)
//
// Contact nbickford@nvidia.com (Neil Bickford) for feedback.

#include <iostream>
#include <nvml.h>
#include <sstream>

#include "loadNVML.h"

// Checks that each NVML call succeeded; prints out a diagnostic message and
// throws an error if it failed.
#define CheckNVMLErrors(val) CheckNVML((val), #val, __FILE__, __LINE__)
inline void CheckNVML(nvmlReturn_t result, char const* const func, const char* const file, int const line)
{
  if(result == NVML_SUCCESS)
  {
    return;
  }
  else if(result == NVML_ERROR_NOT_SUPPORTED)
  {
    std::cout << func << " returned NVML_ERROR_NOT_SUPPORTED at " << file << " : " << line << "\n";
  }
  else
  {
    const char* error_string = nvmlErrorString(result);

    std::stringstream str;
    str << "NVML error! Result = " << static_cast<int>(result) << " (" << error_string << ") at " << file << " : "
        << line << " '" << func << "'";
    throw std::runtime_error(str.str());
  }
}

int main()
{
  // Make sure the NVML library is loaded.
  if(!LoadNVMLImports())
  {
    std::cout << "Failed to find or load an NVML DLL! Is an NVIDIA driver installed?\n";
    return 1;
  }

  // Initialize NVML. If this fails, no other NVML functions will be valid, so
  // print a custom error message without depending on nvmlErrorString.
  {
    const nvmlReturn_t result = nvmlInit();
    switch(result)
    {
      case NVML_SUCCESS:
        break;
      case NVML_ERROR_DRIVER_NOT_LOADED:
        std::cout << "nvmlInit() failed: Driver not loaded.\n";
        return 1;
      case NVML_ERROR_NO_PERMISSION:
        std::cout << "nvmlInit() failed: NVML does not have permission to talk to the driver.\n";
        return 1;
      default:
        std::cout << "nvmlInit() failed: Error code was " << result << "\n";
        return 1;
    }
  }

  try
  {
    // Systems might have multiple devices (GPUs).
    // We'll iterate over each device.

    // Get the number of devices:
    unsigned int device_count = 0;
    CheckNVMLErrors(nvmlDeviceGetCount(&device_count));
    std::cout << device_count << " device(s).\n";

    // Loop over each device:
    for(unsigned int device_index = 0; device_index < device_count; ++device_index)
    {
      std::cout << "Device " << device_index << ":\n";

      // Get the device handle:
      nvmlDevice_t device;
      CheckNVMLErrors(nvmlDeviceGetHandleByIndex(device_index, &device));

      // Print the device name:
      {
        char device_name[NVML_DEVICE_NAME_BUFFER_SIZE];
        CheckNVMLErrors(nvmlDeviceGetName(device, device_name, NVML_DEVICE_NAME_BUFFER_SIZE));
        device_name[NVML_DEVICE_NAME_BUFFER_SIZE - 1] = '\0';
        std::cout << "\tName: " << device_name << "\n";
      }

      // Get the brand of GPU. Set is_enterprise_or_quadro to true only if the GPU is an Enterprise or Quadro GPU.
      {
        nvmlBrandType_t brand_type = NVML_BRAND_UNKNOWN;
        CheckNVMLErrors(nvmlDeviceGetBrand(device, &brand_type));
        const bool is_enterprise_or_quadro = (brand_type == NVML_BRAND_QUADRO) || (brand_type == NVML_BRAND_NVIDIA_VAPPS)
                                             || (brand_type == NVML_BRAND_QUADRO_RTX);

        std::cout << "\tIs Enterprise/Quadro GPU: " << (is_enterprise_or_quadro ? "Yes" : "No") << "\n";
      }
    }
  }
  catch(std::runtime_error err)
  {
    std::cout << err.what();
    return 1;
  }

  // Shut down NVML - to be called after all NVML work is done.
  CheckNVMLErrors(nvmlShutdown());

  return 0;
}