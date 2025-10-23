#pragma once
#include <SFML/Graphics.hpp>
#include "ConfigLoader.hpp"
// #include ""

class Display
{
    public:
        Display();
        ~Display();
        void display();
        void update();
        void clear();
        sf::RenderWindow window;

    private:
        ConfigLoader windowLoader;
};
