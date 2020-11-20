# nvml_enterprise_gpu_check

Shows how to detect if an NVIDIA GPU is an Enterprise or Quadro GPU, using [NVML](https://developer.nvidia.com/nvidia-management-library-nvml) (the library for which is installed with the CUDA Toolkit). This code can also be modified to detect if the GPU is a Tesla or GeForce GPU.

Windows-only applications can also use [NVAPI](https://developer.nvidia.com/nvapi) to detect if a GPU is an Enterprise or Quadro GPU using `NvAPI_GPU_GetQuadroStatus`.

## What Differentiates Enterprise GPUs?

NVIDIA Enterprise and Quadro GPUs such as the RTX A6000 or the Quadro RTX 8000 are designed for professional, scientific, and workstation applications, and are built for systems that need certified devices and drivers. They support special features such as Quadro Sync, NVIDIA Mosaic, Quadro View, and Quadro Virtual Data Center Workstation. They also include around 8x faster 64-bit floating-point performance, larger memory capacity, faster OpenGL interop, and more.

## Example Outputs

On a device with a Quadro RTX 8000 GPU:

```
1 device(s).
Device 0:
        Name: Quadro RTX 8000
        Is Professional/Quadro GPU: Yes
```

On a device with two Quadro GPUs, one a Quadro RTX 8000 and the other a Quadro P3000:

```
2 device(s).
Device 0:
        Name: Quadro RTX 8000
        Is Professional/Quadro GPU: Yes
Device 1:
        Name: Quadro P3000
        Is Professional/Quadro GPU: Yes
```

On a device with a GeForce GTX 1060 Max-Q GPU (which is not a Professional or Quadro GPU):

```
1 device(s).
Device 0:
        Name: GeForce GTX 1060 with Max-Q Design
        Is Professional/Quadro GPU: No
```

## How It Works

Determining which GPUs are Enterprise or Quadro GPUs takes five steps:

- Load the NVML shared library.
- Call `nvmlInit()` to initialize NVML.
- Call `nvmlDeviceGetCount` to get the number of NVIDIA GPUs. Then for each index from 0 to the count minus one:
  - Call `nvmlDeviceGetHandleByIndex` to get the `nvmlDevice_t` device from the index.
  - Call `nvmlDeviceGetBrand` with the device to get an `nvmlBrandType_t` brand. If this is `NVML_BRAND_QUADRO`, then the device is an Enterprise or Quadro GPU.

The last four steps are usually straightforward, but the first step can be a bit more complex on Windows. We describe how to load NVML below.

### Loading NVML

The NVML shared library depends on the version of the driver it was installed with, so applications should look for the NVML shared library installed with the driver (rather than redistributing the NVML shared library).

On Linux platforms, NVML is included in the standard library load path. As such, loading NVML is relatively easy; it's enough to link with `nvml.lib` and include `nvml.h` (see the included [`FindNVML.cmake`](FindNVML.cmake) script for a way to include NVML in a CMake build system). However, on Windows, the best place to look for NVML is usually in the place it was installed with the driver, in a directory within `C:\Windows\System32\DriverStore\FileRepository`.

We show one way to implement loading NVML correctly in [`loadNVML.cpp`](loadNVML.cpp) using the build configuration in [`CMakeLists.txt`](CMakeLists.txt). On Windows, we tell the compiler to [delay-load](https://docs.microsoft.com/en-us/cpp/build/reference/linker-support-for-delay-loaded-dlls) `nvml.dll`, which makes it so that Windows doesn't attempt to load the DLL using the [standard search order](https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order) when the application starts. We then call `LoadNVMLImports()` before `nvmlInit()`. On Windows, this function finds the correct DriverStore subdirectory by getting the location of the OpenGL driver from the registry. It then adds this directory to the top of the search order using `SetDllDirectoryA`, and then loads NVML's imports using `__HrLoadAllImportsForDll("nvml.dll")`.

## Build Instructions

This sample requires CMake, and a version of the CUDA Toolkit (for NVML). To build this sample, configure and generate the project using CMake on `CMakeLists.txt`. (This should automatically find NVML on your system.) Then build and run the program.

This sample doesn't depend on the [NVIDIA DesignWorks Samples'](https://github.com/nvpro-samples) [`shared_sources`](https://github.com/nvpro-samples/shared_sources) project — but if it is built along with `shared_sources` (for instance, using [`build_all`](https://github.com/nvpro-samples/build_all)), then it will automatically use the same build and install directories as the rest of the nvpro-samples.