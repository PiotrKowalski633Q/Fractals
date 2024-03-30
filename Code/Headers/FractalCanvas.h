#ifndef FRACTALS_FRACTALCANVAS
#define FRACTALS_FRACTALCANVAS

#include <SFML/Graphics.hpp>
#include <CL/cl.hpp>

#include "OpenCLFunctions.h"

enum FractalType
{
    MANDELBROT,
    JULIA
};

struct OpenCLObject
{
    cl::Platform platform;
    cl::Device device;
    cl::Context context;
    cl::Program programMandelbrot;
    cl::Program programJulia;
    cl::CommandQueue commandQueue;
    cl::Kernel kernelMandelbrot;
    cl::Kernel kernelJulia;
    cl::Buffer deviceWidth, deviceHeight, deviceZoom, deviceOffsetX, deviceOffsetY, deviceMaxIterations, deviceColors, deviceColorCount, deviceOutputColors3DimArray;

    cl::NDRange localWorkGroupSize, globalWorkGroupSize;

    int* bufferOutputPixelColors;
};

class FractalCanvas
{
private:
    int mWidth, mHeight;
    sf::VertexArray mPixels;
    int* mColors;

    OpenCLObject mOpenCLObject;

    FractalType mCurrentFractalType;

public:
    FractalCanvas(int width, int height);

    ~FractalCanvas();

    void update(double zoom, double offsetX, double offsetY, int maxIterations);

    void draw(sf::RenderWindow &window);

    void switchFractalTypes();

private:
    void generateFractal(double zoom, double offsetX, double offsetY, int maxIterations);
};

#endif //FRACTALS_FRACTALCANVAS