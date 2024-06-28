#  Copyright (c) 2020-2024, NVIDIA CORPORATION.  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  SPDX-FileCopyrightText: Copyright (c) 2020-2024 NVIDIA CORPORATION
#  SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.5)

get_filename_component(PROJNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJNAME})
message(STATUS "-------------------------------")
message(STATUS "Processing Project ${PROJNAME}:")

#####################################################################################
# If this is built as part of a larger nvpro-samples build, use the appropriate
# bin_${ARCH} directory.
# look for nvpro_core 1) as a sub-folder 2) at some other locations
# this cannot be put anywhere else since we still didn't find setup.cmake yet
#
if(NOT BASE_DIRECTORY)
  find_path(BASE_DIRECTORY
            NAMES nvpro_core/cmake/setup.cmake
            PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. REQUIRED
            DOC "Directory containing nvpro_core"
  )
endif()
if(EXISTS ${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
  include(${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
else()
  message(FATAL_ERROR "could not find base directory, please set BASE_DIRECTORY to folder containing nvpro_core")
endif()

_add_project_definitions(${PROJNAME})

# Find NVML.
_add_package_nvml()
if(NOT NVML_FOUND)
  message(WARNING "Project ${PROJNAME} NOT built: CUDA Toolkit (which includes NVML) not found. Please provide CUDAToolkit_ROOT to the CMake invocation.")
  return()
endif()

add_executable(${PROJNAME} main.cpp loadNVML.h loadNVML.cpp)
include_directories(${PROJNAME} "${NVML_INCLUDE_DIRS}") # target_include_directories was added in CMake 3.0.2
target_link_libraries(${PROJNAME} "${NVML_LIBRARIES}")

# On Windows platforms using MSVC, enable delay loading for nvml.dll.
# Please see the source code for more information as to what this does, or the
# documentation at https://docs.microsoft.com/en-us/cpp/build/reference/linker-support-for-delay-loaded-dlls
if(MSVC)
  set_target_properties(${PROJNAME} PROPERTIES LINK_FLAGS "/DELAYLOAD:nvml.dll")
  # Also link with Cfgmgr32.lib:
  target_link_libraries(${PROJNAME} "Cfgmgr32.lib" "delayimp.lib")
endif()

# Install commands:
install(TARGETS ${PROJNAME}
        CONFIGURATIONS Release
        DESTINATION bin_${ARCH}
)
install(TARGETS ${PROJNAME}
        CONFIGURATIONS Debug
        DESTINATION bin_${ARCH}_debug
)
