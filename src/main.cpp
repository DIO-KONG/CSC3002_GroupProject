// #include "engine/MainLoop.hpp"
#include "Display.hpp"
// #include "loader/ConfigLoader.hpp"
Display display;

int main()
{
    while (display.window.isOpen())
    {
        display.update();
        display.clear();
        display.display();
    }
}
