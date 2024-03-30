#ifndef FRACTALS_OPENCLFUNCTIONS
#define FRACTALS_OPENCLFUNCTIONS

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>

#include <CL/cl.hpp>

class OpenCLFunctions
{
public:
    static std::vector<cl::Platform> getAllPlatforms();
    static std::vector<cl::Device> getAllDevicesOnPlatform(cl::Platform platform);
    static std::vector<cl::Device> getAllDevicesOnAllPlatforms();
    static cl::Program buildProgramFromFile(cl::Device& device, cl::Context& context, const std::string& programFilepath);

    static void allocateMemoryOnDevice(cl::Buffer& deviceMemory, size_t dataArraySize, cl::Context& context);

    static cl::Kernel createKernelForProgram(const std::string& kernelName, cl::Program& program, std::vector<cl::Buffer> arguments);

    static void sendDataToDevice(void* hostData, cl::Buffer deviceData, size_t dataArraySize, cl::CommandQueue& commandQueue);
    static void getDataFromDevice(void* hostData, cl::Buffer deviceData, size_t dataArraySize, cl::CommandQueue& commandQueue);

    static int findBestLocalWorkgroupSizePerDimension(cl::Kernel& kernel, cl::Device& device);
    static cl::NDRange findBestGlobalWorkgroupSize(int bestLocalWorkgroupSizeDimension, int actualWorkDimensionX, int actualWorkDimensionY);

    static void startKernel(cl::Kernel& kernel, cl::CommandQueue& commandQueue, cl::NDRange localWorkGroupSize, cl::NDRange globalWorkGroupSize);

};


#endif //FRACTALS_OPENCLFUNCTIONS
