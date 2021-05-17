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


#include "loadNVML.h"

// Include files for loading the DLL on Windows:
#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // #ifndef WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <cfgmgr32.h>  // For getting driver information in the registry
#include <delayimp.h>
#include <string>

// Attempt to load all imports; use SEH to catch errors
// (see https://github.com/MicrosoftDocs/cpp-docs/issues/1003)
bool TryLoadAllImports()
{
  __try
  {
    if(FAILED(__HrLoadAllImportsForDll("nvml.dll")))
    {
      return false;
    }
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
    return false;
  }
  return true;
}
#endif  // #ifdef _WIN32

bool LoadNVMLImports()
{
#ifdef _WIN32
  // Path to directory where nvml.dll would be if contained in DriverStore.
  std::string driverStoreNVMLPath = "";

  // Look for the OpenGL driver, which exists next to nvml.dll.
  static const char* deviceInstanceIdentifiersGUID = "{4d36e968-e325-11ce-bfc1-08002be10318}";
  const ULONG        flags                         = CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT;
  ULONG              deviceListSize                = 0;

  // Determine the size of the device instance ID buffer
  if(CM_Get_Device_ID_List_SizeA(&deviceListSize, deviceInstanceIdentifiersGUID, flags) != CR_SUCCESS)
  {
    return false;
  }

  // Get the list of device names, which is a sequence of zero-terminated
  // strings with an extra final null character.
  char* deviceNames = new char[deviceListSize];
  if(CM_Get_Device_ID_ListA(deviceInstanceIdentifiersGUID, deviceNames, deviceListSize, flags) != CR_SUCCESS)
  {
    delete[] deviceNames;
    return false;
  }

  // Iterate over device names; continue to the next device if we encounter errors.
  for(char* deviceName = deviceNames; *deviceName; deviceName += strlen(deviceName) + 1)
  {
    // Get the device instance handle given the device name.
    DEVINST devID = 0;
    if(CM_Locate_DevNodeA(&devID, deviceName, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS)
    {
      continue;
    }

    // Open this device's configuration information registry key.
    HKEY regKey = 0;
    if(CM_Open_DevNode_Key(devID, KEY_QUERY_VALUE, 0, RegDisposition_OpenExisting, &regKey, CM_REGISTRY_SOFTWARE) != CR_SUCCESS)
    {
      continue;
    }

    // Query the length and then the value of the "OpenGLDriverName" key.
    const char* valueName = "OpenGLDriverName";
    DWORD       valueSize = 0;
    if(RegQueryValueExA(regKey, valueName, NULL, NULL, NULL, &valueSize) != ERROR_SUCCESS)
    {
      RegCloseKey(regKey);
      continue;
    }

    char* regValue = new char[valueSize];
    if(RegQueryValueExA(regKey, valueName, NULL, NULL, reinterpret_cast<LPBYTE>(regValue), &valueSize) != ERROR_SUCCESS)
    {
      delete[] regValue;
      RegCloseKey(regKey);
      continue;
    }

    // regValue now contains the path to the OpenGL driver DLL!
    // Let's get its folder.
    char*      lastBackslashPos = strrchr(regValue, '\\');
    const bool willSetPath      = (lastBackslashPos != nullptr);
    if(willSetPath)
    {
      driverStoreNVMLPath = std::string(regValue, lastBackslashPos);
    }

    delete[] regValue;
    RegCloseKey(regKey);

    if(willSetPath)
    {
      break;
    }
  }

  delete[] deviceNames;

  // Add driverStoreNVMLPath as the first search path if non-empty.
  if(driverStoreNVMLPath.size() > 0)
  {
    BOOL directorySet = SetDllDirectoryA(driverStoreNVMLPath.c_str());
    if(directorySet == FALSE)
    {
      return false;  // SetDllDirectoryA failed
    }
  }

  bool succeeded = TryLoadAllImports();
  SetDllDirectoryA("");  // Restore search paths

  return succeeded;

#else   // For non-Windows systems:
  return true;
#endif  // #ifdef _WIN32
}