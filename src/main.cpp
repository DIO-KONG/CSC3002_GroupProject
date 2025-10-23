#include "engine/MainLoop.hpp"
#include "engine/Window.hpp"
#include "loader/ConfigLoader.hpp"

int main()
{
    // Load configuration
    loader::ConfigLoader configLoader;
    auto config = configLoader.loadConfig("config/engine.ini");

    // Create window
    Window window(unzip(config.windowConfig));

    // Start main loop
    MainLoop mainLoop(window, config);
    mainLoop.run();

    return 0;
}
