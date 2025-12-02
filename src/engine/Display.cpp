#include "Display.hpp"
#include "ConfigLoader.hpp"
#include <cmath>

Camera::~Camera()
{
    // Camera类的析构函数实现
}

Camera::Camera()
{
    // Camera类的构造函数实现   
}

void Camera::init(sf::Vector2f center, sf::Vector2f size)
{
    // Camera类的初始化实现
    view.setCenter(center);
    view.setSize(size);

    // 初始化最小X值为屏幕一半
    minX = size.x / 2.f;
    maxX = 3500.f; // 假设最大X值为3500，可以根据实际场景调整
    followoffsetX = size.x / 10.f; // 跟随偏移设为屏幕宽度的十分之一（玩家位于屏幕左边2/5位置）
}

sf::View& Camera::getView()
{
    // 获取视图的实现
    return view;
}

void Camera::update()
{
    // Debug
    // printf("Camera update called. FollowPoint: (%.2f, %.2f), Camera Center: (%.2f, %.2f), followoffsetX: %.2f\n",
        //    followPoint.x, followPoint.y,
        //    view.getCenter().x, view.getCenter().y,
        //    followoffsetX);

    float targetX = followPoint.x + followoffsetX;
    
    if (targetX < minX)
        targetX = minX;
    if (targetX > maxX)
        targetX = maxX;

    // Camera类的更新实现 如果FollowPoint与摄像机中心距离超过followoffsetX，则更新摄像机位置
    view.setCenter({targetX, view.getCenter().y});
}

void Camera::setCenter(sf::Vector2f center)
{
    view.setCenter(center);
}

void Camera::setSize(sf::Vector2f size)
{
    view.setSize(size);
}

Display::Display()
{
    // Display类的构造函数实现
    windowLoader.setBaseDir("");
    windowLoader.loadConfig("config\\engine.ini", "Display");
    int width = std::get<int>(windowLoader.getValue("DisplayWidth"));
    int height = std::get<int>(windowLoader.getValue("DisplayHeight"));
    int frameLimit = std::get<int>(windowLoader.getValue("FrameLimit"));
    std::string title = std::get<std::string>(windowLoader.getValue("WindowTitle"));
    window = sf::RenderWindow(sf::VideoMode({static_cast<unsigned int>(width), static_cast<unsigned int>(height)}), title);
    window.setFramerateLimit(frameLimit);
    // 初始化相机
    sf::Vector2f center(static_cast<float>(width) / 2.f, static_cast<float>(height) / 2.f);
    sf::Vector2f size(static_cast<float>(width), static_cast<float>(height));
    camera.init(center, size);
}

Display::~Display()
{
    // Display类的析构函数实现
    // 确保窗口正确关闭
    if (window.isOpen()) {
        window.close();
    }
    // sf::RenderWindow会自动处理资源释放
}

void Display::display()
{
    // Display类的显示逻辑实现
    window.display();
}

void Display::update()
{
    // Display类的更新逻辑实现
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }
    }
    // 更新相机
    camera.update();
    // 应用相机视图到窗口
    window.setView(camera.getView());
}

void Display::clear()
{
    // Display类的清除逻辑实现
    window.clear();
}