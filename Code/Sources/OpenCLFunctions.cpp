#include "../Headers/OpenCLFunctions.h"

std::vector<cl::Platform> OpenCLFunctions::getAllPlatforms()
{
    std::vector<cl::Platform> allPlatforms;
    cl::Platform::get(&allPlatforms);

    if (allPlatforms.size()==0)
    {
        std::cout<<" No platforms found. Check OpenCL installation!" << std::endl;
        exit(1);
    }

    return allPlatforms;
}

std::vector<cl::Device> OpenCLFunctions::getAllDevicesOnPlatform(cl::Platform platform)
{
    std::vector<cl::Device> allDevicesOnPlatform;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &allDevicesOnPlatform);

    if(allDevicesOnPlatform.size()==0)
    {
        std::cout<<" No devices found. Check OpenCL installation!" << std::endl;
        exit(1);
    }

    return allDevicesOnPlatform;
}

std::vector<cl::Device> OpenCLFunctions::getAllDevicesOnAllPlatforms()
{
    std::vector<cl::Device> allDevicesOnSinglePlatform;
    std::vector<cl::Device> allDevicesOnAllPlatforms;
    std::vector<cl::Platform> allPlatforms = getAllPlatforms();

    for (int i=0; i<allPlatforms.size(); i++)
    {
        allDevicesOnSinglePlatform.clear();
        allPlatforms[i].getDevices(CL_DEVICE_TYPE_ALL, &allDevicesOnSinglePlatform);
        allDevicesOnAllPlatforms.insert(allDevicesOnAllPlatforms.end(), allDevicesOnSinglePlatform.begin(), allDevicesOnSinglePlatform.begin());
    }

    if(allDevicesOnAllPlatforms.size()==0)
    {
        std::cout<<" No devices found. Check OpenCL installation!" << std::endl;
        exit(1);
    }

    return allDevicesOnAllPlatforms;
}

cl::Program OpenCLFunctions::buildProgramFromFile(cl::Device& device, cl::Context& context, const std::string& programFilepath)
{
    cl::Program::Sources sources;

    std::ifstream file(programFilepath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    sources.push_back({content.c_str(), content.length()});

    cl::Program program(context, sources);
    if (program.build({device}) != CL_SUCCESS)
    {
        std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        exit(1);
    }

    return program;
}

void OpenCLFunctions::allocateMemoryOnDevice(cl::Buffer& deviceMemory, size_t dataArraySize, cl::Context& context)
{
    int error = 0;
    deviceMemory = cl::Buffer(context, CL_MEM_READ_WRITE, dataArraySize, nullptr, &error);
    if (error)
    {
        std::cout << "OpenCL Buffer allocation failed with error code " << error << std::endl;
    }
}

cl::Kernel OpenCLFunctions::createKernelForProgram(const std::string& kernelName, cl::Program& program, std::vector<cl::Buffer> arguments)
{
    int error = 0;
    cl::Kernel kernel = cl::Kernel(program, kernelName.c_str(), &error);
    if(error < 0)
    {
        std::cout << "OpenCL Kernel creation failed with error code: " << error << std::endl;
        exit(1);
    }

    for (int i=0; i<arguments.size(); i++)
    {
        error = kernel.setArg(i, arguments[i]);
        if(error < 0)
        {
            std::cout << "OpenCL Kernel argument setting failed with error code: " << error << std::endl;
            exit(1);
        }
    }

    return kernel;
}

void OpenCLFunctions::sendDataToDevice(void* hostData, cl::Buffer deviceData, size_t dataArraySize, cl::CommandQueue& commandQueue)
{
    commandQueue.enqueueWriteBuffer(deviceData, true, 0u, dataArraySize, hostData);
}

void OpenCLFunctions::getDataFromDevice(void* hostData, cl::Buffer deviceData, size_t dataArraySize, cl::CommandQueue& commandQueue)
{
    commandQueue.enqueueReadBuffer(deviceData, true, 0u, dataArraySize, hostData);
}

int OpenCLFunctions::findBestLocalWorkgroupSizePerDimension(cl::Kernel& kernel, cl::Device& device)
{
    std::array<size_t, 1> kernel_work_group_size;
    kernel.getWorkGroupInfo<std::array<size_t, 1>>(device, CL_KERNEL_WORK_GROUP_SIZE, &kernel_work_group_size);

    int workGroupSizeOneDimension = 1;
    while ((workGroupSizeOneDimension*2)*(workGroupSizeOneDimension*2)<kernel_work_group_size[0])
    {
        workGroupSizeOneDimension *=2;
    }
    return workGroupSizeOneDimension;
}

cl::NDRange OpenCLFunctions::findBestGlobalWorkgroupSize(int bestLocalWorkgroupSizeDimension, int actualWorkDimensionX, int actualWorkDimensionY)
{
    int bestGlobalWorkgroupSizeX = (int)ceil((double)actualWorkDimensionX/bestLocalWorkgroupSizeDimension)*bestLocalWorkgroupSizeDimension;
    int bestGlobalWorkgroupSizeY = (int)ceil((double)actualWorkDimensionY/bestLocalWorkgroupSizeDimension)*bestLocalWorkgroupSizeDimension;
    return cl::NDRange(bestGlobalWorkgroupSizeX, bestGlobalWorkgroupSizeY);
}

void OpenCLFunctions::startKernel(cl::Kernel& kernel, cl::CommandQueue& commandQueue, cl::NDRange localWorkGroupSize, cl::NDRange globalWorkGroupSize)
{
    int err = commandQueue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkGroupSize, localWorkGroupSize);
    if(err < 0)
    {
        std::cout << "Couldn't enqueue the kernel, error code: " << err << std::endl;
        exit(1);
    }
}