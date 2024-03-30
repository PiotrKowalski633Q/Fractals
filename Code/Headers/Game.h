#ifndef FRACTALS_GAME
#define FRACTALS_GAME

#include <chrono>
#include <wtypes.h>
#include <thread>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "FractalCanvas.h"

class Game
{
    sf::RenderWindow mWindow;

    std::chrono::steady_clock::time_point mFrameBegin;
    std::chrono::steady_clock::time_point mFrameEnd;
    double mDeltaTime;
    double mRenderingFrameTimer;
    double mExpectedRenderingFps;

    FractalCanvas mFractalCanvas;
    bool mDoesFractalCanvasNeedUpdate;

    double mZoom;
    double mOffsetX, mOffsetY;

    bool mIsZoomingIn, mIsZoomingOut, mIsMovingRight, mIsMovingLeft, mIsMovingDown, mIsMovingUp;

public:
    Game();

private:
    void gameLoop();

    void processInput();
    void update();
    void draw();

public:
    void run();
};

#endif //FRACTALS_GAME
