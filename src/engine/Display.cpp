#include "Display.hpp"
#include "ConfigLoader.hpp"

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
}

void Display::clear()
{
    // Display类的清除逻辑实现
    window.clear();
}
