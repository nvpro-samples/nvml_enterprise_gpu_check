# Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.5) 

get_filename_component(PROJNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
Project(${PROJNAME})
Message(STATUS "-------------------------------")
Message(STATUS "Processing Project ${PROJNAME}:")

#####################################################################################
# If this is built as part of a larger nvpro-samples build, use the appropriate
# bin_${ARCH} directory.
# look for shared_sources 1) as a sub-folder 2) at some other locations
# this cannot be put anywhere else since we still didn't find setup.cmake yet
#
if(NOT BASE_DIRECTORY)

  find_path(BASE_DIRECTORY
    NAMES shared_sources/cmake/setup.cmake
    PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. 
    REQUIRED
    DOC "Directory containing shared_sources"
    )
endif()
if(EXISTS ${BASE_DIRECTORY}/shared_sources/cmake/setup.cmake)
  include(${BASE_DIRECTORY}/shared_sources/cmake/setup.cmake)
else()
  message(FATAL_ERROR "could not find base directory, please set BASE_DIRECTORY to folder containing shared_sources")
endif()
  
_add_project_definitions(${PROJNAME})

# Find NVML.
_add_package_NVML()

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
install(TARGETS ${PROJNAME} CONFIGURATIONS Release DESTINATION bin_${ARCH})
install(TARGETS ${PROJNAME} CONFIGURATIONS Debug DESTINATION bin_${ARCH}_debug)