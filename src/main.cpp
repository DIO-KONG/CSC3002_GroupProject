#include "ConfigLoader.hpp"
#include "Display.hpp"
#include "EventSys.hpp"
#include "GameInput.hpp"
#include "GameObj.hpp"
#include "ResourceLoader.hpp"
#include "Scene.hpp"

int main()
{
    // 创建显示窗口、事件系统和游戏输入读取器
    Display display;
    EventSys eventSys;
    GameInputRead gameInput;

    // 加载引擎配置
    float deltaTime = 0.0f;
    ConfigLoader engineLoader;
    engineLoader.loadConfig("config/engine.ini", "Engine");
    deltaTime = std::get<float>(engineLoader.getValue("DeltaTime"));

    // 构建scene路径
    auto paths = engineLoader.getAllValues("Path");

    // 创建初始场景（例:菜单场景）
    Scene menuScene;
    // 将初始场景设置为当前场景
    engineLoader.loadConfig("config/engine.ini", "Path");
    std::unique_ptr<Scene> currentScene = std::make_unique<Scene>(menuScene);

    while (display.window.isOpen())
    {
        // 准备下一帧

        // 记录帧开始时间
        sf::Time frameStartTime = eventSys.getElapsedTime();
        
        // 处理窗口是否关闭
        display.update();
        
        // 向事件系统注册按键更新事件
        auto keyUpdateEvent = [&gameInput]() {
            gameInput.update();
        };
        eventSys.regImmEvent(EventSys::ImmEventPriority::INPUT, keyUpdateEvent);
        
        // 检查场景是否可用
        if (!currentScene)
        {
            continue;
        }

        // 场景更新（包括场景内对象更新）
        currentScene->update(deltaTime);

        // 清空显示
        display.clear();

        // 执行事件系统中的即时事件和定时事件
        eventSys.executeImmEvents();
        eventSys.executeTimedEvents();

        // 场景渲染（包括场景内对象渲染）
        display.display();
    }
}
