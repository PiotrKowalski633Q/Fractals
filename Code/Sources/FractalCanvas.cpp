#include "../Headers/FractalCanvas.h"

FractalCanvas::FractalCanvas(int width, int height)
:mWidth(width),
mHeight(height),
mPixels(sf::Points, width*height),
mColors(new int[12] {0,0,0,
                     300071,832371,271111,
                     543261,322251,1767611,
                     289091,428881,855541}),
mCurrentFractalType(FractalType::MANDELBROT)
{
    //all the drawable "pixel" objects must have a set position on the screen
    for (int i=0; i<mWidth; i++)
    {
        for (int j=0; j<mHeight; j++)
        {
            mPixels[i*mHeight+j].position = sf::Vector2f(i, j);
        }
    }

    //sets default platform and device to use for kernel calculations, allDevicesOnDefaultPlatform[0] is CPU, allDevicesOnDefaultPlatform[1] is GPU
    std::vector<cl::Platform> allPlatforms = OpenCLFunctions::getAllPlatforms();
    mOpenCLObject.platform = allPlatforms[0];
    std::vector<cl::Device> allDevicesOnDefaultPlatform = OpenCLFunctions::getAllDevicesOnPlatform(mOpenCLObject.platform);
    //GPU should generally be used whenever available, but not every PC has a separate GPU
    if (allDevicesOnDefaultPlatform.size() < 2)
    {
        mOpenCLObject.device = allDevicesOnDefaultPlatform[0];
    }
    else
    {
        mOpenCLObject.device = allDevicesOnDefaultPlatform[1];
    }

    //sets OpenCL context and begins allocating memory on the device using OpenCL buffer objects
    mOpenCLObject.context = cl::Context({mOpenCLObject.device});
    OpenCLFunctions::allocateMemoryOnDevice(mOpenCLObject.deviceWidth, 1*sizeof(int), mOpenCLObject.context);
    OpenCLFunctions::allocateMemoryOnDevice(mOpenCLObject.deviceHeight, 1*sizeof(int), mOpenCLObject.context);
    OpenCLFunctions::allocateMemoryOnDevice(mOpenCLObject.deviceZoom, 1*sizeof(double), mOpenCLObject.context);
    OpenCLFunctions::allocateMemoryOnDevice(mOpenCLObject.deviceOffsetX, 1*sizeof(double), mOpenCLObject.context);
    OpenCLFunctions::allocateMemoryOnDevice(mOpenCLObject.deviceOffsetY, 1*sizeof(double), mOpenCLObject.context);
    OpenCLFunctions::allocateMemoryOnDevice(mOpenCLObject.deviceMaxIterations, 1*sizeof(int), mOpenCLObject.context);
    OpenCLFunctions::allocateMemoryOnDevice(mOpenCLObject.deviceColors, 12*sizeof(int), mOpenCLObject.context);
    OpenCLFunctions::allocateMemoryOnDevice(mOpenCLObject.deviceColorCount, 1*sizeof(int), mOpenCLObject.context);
    OpenCLFunctions::allocateMemoryOnDevice(mOpenCLObject.deviceOutputColors3DimArray, mWidth*mHeight*3*sizeof(int), mOpenCLObject.context);

    //sets remaining OpenCL objects including two kernels with two separate programs that will be used during fractal generation
    mOpenCLObject.programMandelbrot = OpenCLFunctions::buildProgramFromFile(mOpenCLObject.device, mOpenCLObject.context, "Resources/Kernels/mandelbrot.txt");
    mOpenCLObject.programJulia = OpenCLFunctions::buildProgramFromFile(mOpenCLObject.device, mOpenCLObject.context, "Resources/Kernels/julia.txt");
    mOpenCLObject.commandQueue = cl::CommandQueue(mOpenCLObject.context, mOpenCLObject.device);
    mOpenCLObject.bufferOutputPixelColors = new int[mWidth*mHeight*3];
    mOpenCLObject.kernelMandelbrot = OpenCLFunctions::createKernelForProgram("mandelbrot", mOpenCLObject.programMandelbrot, {mOpenCLObject.deviceWidth, mOpenCLObject.deviceHeight, mOpenCLObject.deviceZoom, mOpenCLObject.deviceOffsetX, mOpenCLObject.deviceOffsetY, mOpenCLObject.deviceMaxIterations, mOpenCLObject.deviceColors, mOpenCLObject.deviceColorCount, mOpenCLObject.deviceOutputColors3DimArray});
    mOpenCLObject.kernelJulia = OpenCLFunctions::createKernelForProgram("julia", mOpenCLObject.programJulia, {mOpenCLObject.deviceWidth, mOpenCLObject.deviceHeight, mOpenCLObject.deviceZoom, mOpenCLObject.deviceOffsetX, mOpenCLObject.deviceOffsetY, mOpenCLObject.deviceMaxIterations, mOpenCLObject.deviceColors, mOpenCLObject.deviceColorCount, mOpenCLObject.deviceOutputColors3DimArray});

    //local and global work sizes can only be decided after kernels are created
    int bestLocalWorkgroupSizePerDimensionMandelbrot = OpenCLFunctions::findBestLocalWorkgroupSizePerDimension(mOpenCLObject.kernelMandelbrot, mOpenCLObject.device);
    int bestLocalWorkgroupSizePerDimensionJulia = OpenCLFunctions::findBestLocalWorkgroupSizePerDimension(mOpenCLObject.kernelMandelbrot, mOpenCLObject.device);
    int bestLocalWorkgroupSizePerDimension = std::min(bestLocalWorkgroupSizePerDimensionMandelbrot, bestLocalWorkgroupSizePerDimensionJulia);
    mOpenCLObject.localWorkGroupSize = cl::NDRange(bestLocalWorkgroupSizePerDimension, bestLocalWorkgroupSizePerDimension);
    mOpenCLObject.globalWorkGroupSize = OpenCLFunctions::findBestGlobalWorkgroupSize(bestLocalWorkgroupSizePerDimension, mWidth, mHeight);

    //finally send to the device all the data that is already known and will not change during fractal generation
    int arrayFormWidth[1] = {mWidth};
    int arrayFormHeight[1] = {mHeight};
    int arrayFormColorCount[1] = {4};
    OpenCLFunctions::sendDataToDevice((void*)arrayFormWidth, mOpenCLObject.deviceWidth, 1*sizeof(int), mOpenCLObject.commandQueue);
    OpenCLFunctions::sendDataToDevice((void*)arrayFormHeight, mOpenCLObject.deviceHeight, 1*sizeof(int), mOpenCLObject.commandQueue);
    OpenCLFunctions::sendDataToDevice((void*)mColors, mOpenCLObject.deviceColors, 12*sizeof(int), mOpenCLObject.commandQueue);
    OpenCLFunctions::sendDataToDevice((void*)arrayFormColorCount, mOpenCLObject.deviceColorCount, 1*sizeof(int), mOpenCLObject.commandQueue);
}

FractalCanvas::~FractalCanvas()
{
    delete[] mColors;
}

void FractalCanvas::update(double zoom, double offsetX, double offsetY, int maxIterations)
{
    generateFractal(zoom, offsetX, offsetY, maxIterations);
}

void FractalCanvas::draw(sf::RenderWindow &window)
{
    window.draw(mPixels);
}

void FractalCanvas::switchFractalTypes()
{
    if (mCurrentFractalType == FractalType::MANDELBROT)
    {
        mCurrentFractalType = FractalType::JULIA;
    }
    else if (mCurrentFractalType == FractalType::JULIA)
    {
        mCurrentFractalType = FractalType::MANDELBROT;
    }
}

void FractalCanvas::generateFractal(double zoom, double offsetX, double offsetY, int maxIterations)
{
    //OpenCL only likes arrays, so convert all passable variables to arrays
    double arrayFormZoom[1] = {zoom};
    double arrayFormOffsetX[1] = {offsetX};
    double arrayFormOffsetY[1] = {offsetY};
    int arrayFormMaxIterations[1] = {maxIterations};

    //sends all the necessary (changing between frames) data to kernels
    OpenCLFunctions::sendDataToDevice((void*)arrayFormZoom, mOpenCLObject.deviceZoom, 1*sizeof(double), mOpenCLObject.commandQueue);
    OpenCLFunctions::sendDataToDevice((void*)arrayFormOffsetX, mOpenCLObject.deviceOffsetX, 1*sizeof(double), mOpenCLObject.commandQueue);
    OpenCLFunctions::sendDataToDevice((void*)arrayFormOffsetY, mOpenCLObject.deviceOffsetY, 1*sizeof(double), mOpenCLObject.commandQueue);
    OpenCLFunctions::sendDataToDevice((void*)arrayFormMaxIterations, mOpenCLObject.deviceMaxIterations, 1*sizeof(int), mOpenCLObject.commandQueue);

    //begins calculating pixel colors for every pixel on the screen using the chosen fractal algorithm
    if (mCurrentFractalType == FractalType::MANDELBROT)
    {
        OpenCLFunctions::startKernel(mOpenCLObject.kernelMandelbrot, mOpenCLObject.commandQueue, mOpenCLObject.localWorkGroupSize, mOpenCLObject.globalWorkGroupSize);
    }
    else if (mCurrentFractalType == FractalType::JULIA)
    {
        OpenCLFunctions::startKernel(mOpenCLObject.kernelJulia, mOpenCLObject.commandQueue, mOpenCLObject.localWorkGroupSize, mOpenCLObject.globalWorkGroupSize);
    }

    //waits for kernels to finish all their actions...
    mOpenCLObject.commandQueue.finish();

    //...before retrieving results...
    OpenCLFunctions::getDataFromDevice((void*)mOpenCLObject.bufferOutputPixelColors, mOpenCLObject.deviceOutputColors3DimArray, mWidth*mHeight*3*sizeof(int), mOpenCLObject.commandQueue);

    //...and using them to set colors for all pixels on the screen
    for (int i=0; i<mWidth; i++)
    {
        for (int j=0; j<mHeight; j++)
        {
            mPixels[i*mHeight+j].color.r = mOpenCLObject.bufferOutputPixelColors[(i*mHeight+j)*3+0];
            mPixels[i*mHeight+j].color.g = mOpenCLObject.bufferOutputPixelColors[(i*mHeight+j)*3+1];
            mPixels[i*mHeight+j].color.b = mOpenCLObject.bufferOutputPixelColors[(i*mHeight+j)*3+2];
        }
    }
}
