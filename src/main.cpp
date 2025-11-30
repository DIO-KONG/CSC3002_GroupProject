#include "ConfigLoader.hpp"
#include "Display.hpp"
#include "EventSys.hpp"
#include "GameInput.hpp"
#include "GameObj.hpp"
#include "ResourceLoader.hpp"
#include "Scene.hpp"

int main()
{
    // 创建显示窗口、事件系统和游戏输入读取器（使用智能指针）
    auto display = std::make_shared<Display>();
    auto eventSys = std::make_shared<EventSys>();
    auto gameInput = std::make_shared<GameInputRead>();
    std::shared_ptr<sf::RenderWindow> windowPtr(display, &display->window);

    // Debug
    printf("Display, EventSys, and GameInputRead created.\n");

    // 加载引擎配置
    float deltaTime = 0.0f;
    ConfigLoader engineLoader;
    engineLoader.loadConfig("config/engine.ini", "Engine");
    deltaTime = std::get<float>(engineLoader.getValue("DeltaTime"));
    
    // Debug
    printf("Engine loaded.\n");

    // 构建scene路径
    engineLoader.loadConfig("config/engine.ini", "Path");
    std::string menupth = std::get<std::string>(engineLoader.getValue("MenuPath"));
    std::string level1pth = std::get<std::string>(engineLoader.getValue("level1Path"));
    // 创建初始场景（例:菜单场景）
    std::shared_ptr<Scene> menuScene = std::make_shared<Scene>();
    // Debug
    printf("Menu Scene created.\n");
    // 初始化场景并设置指针
    menuScene->init(menupth,
                    eventSys,
                    windowPtr,
                    gameInput);
    // Debug
    printf("Menu Scene initialized.\n");
    // 当前场景指针
    std::shared_ptr<Scene> currentScene = menuScene;

    while (display->window.isOpen())
    {
        // 记录帧开始时间
        sf::Time frameStartTime = eventSys->getElapsedTime();
        
        // 处理窗口是否关闭
        display->update();
        
        // 向事件系统注册按键更新事件
        auto keyUpdateEvent = [&gameInput]() {
            gameInput->update();
        };
        eventSys->regImmEvent(EventSys::ImmEventPriority::INPUT, keyUpdateEvent);

        // 场景更新（包括场景内对象更新）
        currentScene->update(deltaTime);
        // 执行事件系统中的即时事件和定时事件
        eventSys->executeImmEvents();
        eventSys->executeTimedEvents();

        // 场景渲染
        display->clear();
        currentScene->render();
        display->display();

        // 获取帧结束时间
        sf::Time frameEndTime = eventSys->getElapsedTime();
        // 等待多余时间以维持稳定帧率
        float frameDuration = frameEndTime.asSeconds() - frameStartTime.asSeconds();
        if (frameDuration < deltaTime)
        {
            // Debug
            printf("Frame Duration: %.4f seconds\n", frameDuration);
            sf::sleep(sf::seconds(deltaTime - frameDuration));
        }
    }
}