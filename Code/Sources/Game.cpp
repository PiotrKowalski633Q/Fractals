#include "../Headers/Game.h"

Game::Game()
:mWindow(sf::RenderWindow( sf::VideoMode( GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 32 ), "BoidsWithSFML", sf::Style::Fullscreen )),
mDeltaTime(0),
mRenderingFrameTimer(0),
mExpectedRenderingFps(30),
mFractalCanvas(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)),
mDoesFractalCanvasNeedUpdate(true),
mZoom(1),
mOffsetX(0),
mOffsetY(0),
mIsZoomingIn(false),
mIsZoomingOut(false),
mIsMovingRight(false),
mIsMovingLeft(false),
mIsMovingDown(false),
mIsMovingUp(false)
{

}

void Game::gameLoop()//main loop, it will continuously poll events, read them and terminate only if the window closes
{
    mFrameBegin = std::chrono::steady_clock::now();

    while (mWindow.isOpen())
    {
        mFrameEnd = std::chrono::steady_clock::now();
        mDeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(mFrameEnd - mFrameBegin).count();
        mRenderingFrameTimer += mDeltaTime;
        mFrameBegin = std::chrono::steady_clock::now();

        processInput();

        if (mDoesFractalCanvasNeedUpdate)
        {
            update();
        }

        if (1000000.0/mRenderingFrameTimer <= mExpectedRenderingFps)
        {
            draw();
            mRenderingFrameTimer -= 1000000.0/mExpectedRenderingFps;
        }
        else
        {
            //this way the loop will always advance at least 1/10 of the way towards next draw call when not drawing and prevent extremely low deltaTime from causing precision problems
            std::this_thread::sleep_for(std::chrono::microseconds((int)(1000000/mExpectedRenderingFps/10)));
        }
    }
}

void Game::processInput()
{
    sf::Event event;
    while (mWindow.pollEvent( event ))
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        {
            mWindow.close();
            break;
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::LShift)
        {
            mIsZoomingIn = true;
        }
        else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::LShift)
        {
            mIsZoomingIn = false;
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
        {
            mIsZoomingOut = true;
        }
        else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Space)
        {
            mIsZoomingOut = false;
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D)
        {
            mIsMovingRight = true;
        }
        else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::D)
        {
            mIsMovingRight = false;
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::A)
        {
            mIsMovingLeft = true;
        }
        else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::A)
        {
            mIsMovingLeft = false;
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S)
        {
            mIsMovingDown = true;
        }
        else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::S)
        {
            mIsMovingDown = false;
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::W)
        {
            mIsMovingUp = true;
        }
        else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::W)
        {
            mIsMovingUp = false;
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Q)
        {
            mFractalCanvas.switchFractalTypes();
            mDoesFractalCanvasNeedUpdate = true;
        }
    }

    if (mIsZoomingIn)
    {
        mZoom *= 1-(0.04)*(mDeltaTime/100000);
        mDoesFractalCanvasNeedUpdate = true;
    }
    else if (mIsZoomingOut)
    {
        mZoom *= 1+(0.04)*(mDeltaTime/100000);
        mDoesFractalCanvasNeedUpdate = true;
    }

    if (mIsMovingRight)
    {
        mOffsetX += 0.01*mZoom*(mDeltaTime/100000);
        mDoesFractalCanvasNeedUpdate = true;
    }
    else if (mIsMovingLeft)
    {
        mOffsetX -= 0.01*mZoom*(mDeltaTime/100000);
        mDoesFractalCanvasNeedUpdate = true;
    }

    if (mIsMovingDown)
    {
        mOffsetY += 0.01*mZoom*(mDeltaTime/100000);
        mDoesFractalCanvasNeedUpdate = true;
    }
    else if (mIsMovingUp)
    {
        mOffsetY -= 0.01*mZoom*(mDeltaTime/100000);
        mDoesFractalCanvasNeedUpdate = true;
    }
}

void Game::update()
{
    mFractalCanvas.update(mZoom, mOffsetX, mOffsetY, 300);
    mDoesFractalCanvasNeedUpdate = false;
}

void Game::draw()
{
    mWindow.clear();
    mFractalCanvas.draw(mWindow);
    mWindow.display();
}

void Game::run()
{

    sf::Music music;
    music.openFromFile("Resources/Music/just-relax-11157.mp3");
    music.setLoop(true);
    music.play();

    gameLoop();
}