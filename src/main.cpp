#include "ConfigLoader.hpp"
#include "Display.hpp"
#include "EventSys.hpp"
#include "GameInput.hpp"
#include "GameObj.hpp"
#include "ResourceLoader.hpp"
#include "Scene.hpp"
#include "Player.hpp"
#include <SFML/Audio.hpp>


int main()
{
     // 最简单直接的音频测试
    sf::Music music;
    if (!music.openFromFile("audio/menu.ogg")) {
        printf("ERROR: Cannot load audio/menu.ogg\n");
        printf("Trying audio/menu.wav...\n");
        if (!music.openFromFile("audio/menu.ogg")) {
            printf("ERROR: Cannot load any audio file!\n");
            return 1;
        }
    }
    
    music.setVolume(100.0f);
    music.setLooping(true);
    music.play();
    printf("Music is now playing! Press any key in 5 seconds to continue...\n");
    
    sf::sleep(sf::seconds(1.0f));
    
    printf("Continuing with game...\n");
    

    // 创建显示窗口、事件系统和游戏输入读取器（使用智能指针）
    auto display   = std::make_shared<Display>();
    auto eventSys  = std::make_shared<EventSys>();
    auto gameInput = std::make_shared<GameInputRead>();
    std::shared_ptr<sf::RenderWindow> windowPtr(display, &display->window);

    // Debug
    printf("Display, EventSys, and GameInputRead created.\n");

    // 加载引擎配置
    float deltaTime    = 0.0f;
    int   subStepCount = 4;

    ConfigLoader engineLoader;
    engineLoader.loadConfig("config/engine.ini", "Engine");
    deltaTime    = std::get<float>(engineLoader.getValue("DeltaTime"));
    subStepCount = std::get<int>(engineLoader.getValue("subStepCount"));

    // Debug
    printf("Engine loaded.\n");

    // 构建 scene 路径
    engineLoader.loadConfig("config/engine.ini", "Path");
    std::string menupth   = std::get<std::string>(engineLoader.getValue("MenuPath"));
    std::string level1pth = std::get<std::string>(engineLoader.getValue("level1Path"));

    // 创建菜单场景
    std::shared_ptr<Scene> menuScene = std::make_shared<Scene>();
    menuScene->init(
        menupth,
        eventSys,
        windowPtr,
        gameInput
    );
    menuScene->setUseParallaxWithCamera(false); // 菜单使用基于时间的自动滚动

    // 创建关卡场景
    std::shared_ptr<Scene> level1Scene = std::make_shared<Scene>();
    level1Scene->init(
        level1pth,
        eventSys,
        windowPtr,
        gameInput
    );
    level1Scene->setUseParallaxWithCamera(true); // 关卡使用基于相机的视差滚动

    // 创建玩家对象（先给 level1 用的）
    std::shared_ptr<Player> player = std::make_shared<Player>(
        eventSys,
        windowPtr,
        level1Scene->getWorldId(),
        gameInput
    );
    player->initialize();
    player->setSpawnPosition(100.0f, 500.0f);
    level1Scene->setPlayerPtr(player);

    // 当前场景：初始为菜单
    std::string            sceneName    = "Menu";
    std::shared_ptr<Scene> currentScene = menuScene;

    // 进入主循环
    printf("Entering main loop.\n");

    while (display->window.isOpen())
    {
        // 记录帧开始时间
        sf::Time frameStartTime = eventSys->getElapsedTime();

        // 清空窗口内容
        display->clear();

        // 处理窗口关闭事件
        display->update();

        // 注册按键更新事件
        auto keyUpdateEvent = [&gameInput]() {
            gameInput->update();
        };
        eventSys->regImmEvent(EventSys::ImmEventPriority::INPUT, keyUpdateEvent);

        // 注册摄像机跟随事件：只在 Level1 才跟随玩家；菜单场景固定居中
        auto cameraUpdateEvent = [&display, &player, &sceneName, &currentScene]() {
            if (sceneName == "Level1") {
                if (player) {
                    sf::Vector2f playerPos = player->getPosition();
                    display->camera.updateFollowPoint(playerPos);
                }
                // 将相机位置传递给场景（用于视差背景）
                sf::Vector2f cameraCenter = display->camera.getView().getCenter();
                currentScene->setCameraPosition(cameraCenter);
            } else if (sceneName == "Menu") {
                // 菜单场景：相机固定在屏幕中央
                display->camera.setCenter({960.0f, 540.0f});
                currentScene->setCameraPosition({960.0f, 540.0f});
            }
        };
        eventSys->regImmEvent(EventSys::ImmEventPriority::PRE_UPDATE, cameraUpdateEvent);

        // 场景更新 & 渲染
        currentScene->update(deltaTime, subStepCount);
        currentScene->render();

        // 执行事件系统中的即时事件和定时事件
        eventSys->executeImmEvents();
        eventSys->executeTimedEvents();

        // 显示渲染结果
        display->display();

        // 控制帧率
        sf::Time frameEndTime = eventSys->getElapsedTime();
        float    frameDuration = frameEndTime.asSeconds() - frameStartTime.asSeconds();
        if (frameDuration < deltaTime) {
            printf("Frame Duration: %.4f seconds. Sleep for %.4f seconds\n",
                   frameDuration, deltaTime - frameDuration);
            sf::sleep(sf::seconds(deltaTime - frameDuration));
        } else {
            printf("Frame Duration: %.4f seconds. No sleep needed.\n", frameDuration);
        }

        // ========= 下面是场景切换 & 重置逻辑 =========

        // 小工具函数：重置 Level1 场景 + 玩家
        auto resetLevel1 = [&]() {
            level1Scene->reload();

            // 删除旧玩家对象
            player.reset();

            // 使用新的 worldId 创建新玩家
            player = std::make_shared<Player>(
                eventSys,
                windowPtr,
                level1Scene->getWorldId(),
                gameInput
            );
            player->initialize();
            player->setSpawnPosition(100.0f, 500.0f);

            // 重新添加玩家指针
            level1Scene->setPlayerPtr(player);

            printf("Level1 scene reloaded.\n");
        };

        if (sceneName == "Menu")
        {
            // 从菜单进入关卡：按 Space
            GameInputRead::KeyState spaceState =
                gameInput->getKeyState(sf::Keyboard::Key::Space);
            if (spaceState == GameInputRead::KeyState::KEY_PRESSED)
            {
                // 每次从菜单进入关卡前，重置一次关卡和玩家
                resetLevel1();

                currentScene = level1Scene;
                sceneName    = "Level1";
                printf("Switched to Level1 scene.\n");
            }
        }
        else if (sceneName == "Level1")
        {
            // R：重载关卡
            GameInputRead::KeyState rState =
                gameInput->getKeyState(sf::Keyboard::Key::R);
            if (rState == GameInputRead::KeyState::KEY_PRESSED)
            {
                resetLevel1();
            }

            // ★ 通关后按 Space 回到菜单（配合 YOU WIN 文本）
            if (level1Scene->isLevelCompleted())
            {
                GameInputRead::KeyState spaceState =
                    gameInput->getKeyState(sf::Keyboard::Key::Space);
                if (spaceState == GameInputRead::KeyState::KEY_PRESSED)
                {
                    currentScene = menuScene;
                    sceneName    = "Menu";

                    // 回到菜单时，把摄像机拉回一个固定位置（避免还停在关卡远处导致黑屏）
                    display->camera.updateFollowPoint(sf::Vector2f(100.0f, 500.0f));

                    printf("Level completed, return to Menu.\n");
                }
            }

            // ESC：随时回菜单
            GameInputRead::KeyState escState =
                gameInput->getKeyState(sf::Keyboard::Key::Escape);
            if (escState == GameInputRead::KeyState::KEY_PRESSED)
            {
                currentScene = menuScene;
                sceneName    = "Menu";

                // 同样，ESC 回菜单时也重置一下摄像机
                display->camera.updateFollowPoint(sf::Vector2f(100.0f, 500.0f));

                printf("Switched to Menu scene.\n");
            }
        }
    }

    return 0;
}
