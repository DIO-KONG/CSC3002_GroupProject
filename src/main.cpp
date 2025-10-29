// #include "engine/MainLoop.hpp"
#include "Display.hpp"
// #include "loader/ConfigLoader.hpp"
#include "EventSys.hpp"

Display display;
EventSys eventSys;

int main()
{
    while (display.window.isOpen())
    {
        display.update();
        display.clear();
        display.display();
    }
}
