#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "ConfigLoader.hpp"
// #include ""

class Window
{
    public:
        Window();
        ~Window();
        void display();
        void update();
        void clear();
    private:
        sf::RenderWindow window;
        ConfigLoader windowLoader;
};
