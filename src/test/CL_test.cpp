// 检测ConfigLoader类的基本功能
#include "../loader/ConfigLoader.hpp"
#include <iostream>

std::string basedir = "";
std::string configPath = basedir + "config\\engine.ini";
int main()
{
    ConfigLoader configLoader;
    configLoader.setBaseDir(basedir);
    configLoader.loadConfig(configPath, "Display");

    printf("Configuration loaded from %s\n", configPath.c_str());

    printf("Display Width: %d\n", std::get<int>(configLoader.getValue("DisplayWidth")));
    printf("Display Height: %d\n", std::get<int>(configLoader.getValue("DisplayHeight")));
    // printf("Fullscreen: %s\n", std::get<bool>(configLoader.getValue("FullScreen")) ? "true" : "false");
    // auto fullscreenValue = configLoader.getValue("FullScreen");
    // if (std::holds_alternative<bool>(fullscreenValue)) {
    //     bool isFullscreen = std::get<bool>(fullscreenValue);
    //     printf("Fullscreen: %s\n", isFullscreen ? "true" : "false");
    // } else {
    //     printf("Fullscreen value is not a boolean.\n");
    // }
}