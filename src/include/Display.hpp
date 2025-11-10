#pragma once
#include <SFML/Graphics.hpp>
#include "ConfigLoader.hpp"
// #include ""

class Display
{
    public:
        Display();
        ~Display();
        // 显示窗口
        void display();
        // 更新窗口内容
        void update();
        // 清空窗口内容
        void clear();
        sf::RenderWindow window;

    private:
        ConfigLoader windowLoader;
};
