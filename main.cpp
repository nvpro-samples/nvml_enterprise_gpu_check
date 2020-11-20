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
        const bool is_enterprise_or_quadro = (brand_type == NVML_BRAND_QUADRO);

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