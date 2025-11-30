#include "Display.hpp"
#include "ConfigLoader.hpp"
#include "GameObj.hpp"

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
}

sf::View& Camera::getView()
{
    // 获取视图的实现
    return view;
}

void Camera::update()
{
    // Camera类的更新实现
    // 先简单向右平移视图作为示例
    view.move({10.f, 0.f});


    // if (auto target = targetObj.lock())
    // {
    //     // 假设BaseObj有一个getPosition()方法返回其位置
    //     view.setCenter(target->getPosition());
    // }
}

void Camera::setCenter(sf::Vector2f center)
{
    view.setCenter(center);
}

void Camera::setSize(sf::Vector2f size)
{
    view.setSize(size);
}

void Camera::setTarget(std::weak_ptr<Player> target)
{
    targetObj = target;
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

void Display::setCameraTarget(std::weak_ptr<Player> target)
{
    camera.setTarget(target);
}