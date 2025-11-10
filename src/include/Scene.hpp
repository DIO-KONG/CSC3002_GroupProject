#pragma once
#include "GameObj.hpp"
#include "EventSys.hpp"
#include "ResourceLoader.hpp"
#include <box2d/box2d.h>    
#include <memory>
#include <vector>
#include <variant>

class Scene
{   
    using GameObjs = std::variant<std::monostate/*待添加后续的GameObj子类（最好用unique_ptr）*/>;
    public:
        Scene() = default;
        ~Scene() = default;

        // 初始化场景
        virtual void init(std::string sceneConfigPath, 
                          std::weak_ptr<EventSys> eventSys, 
                          std::weak_ptr<sf::RenderWindow> window,
                          std::weak_ptr<GameInputRead> input);
        // 更新场景状态
        virtual void update(const float deltaTime,const int subStepCount = 4);
        // 渲染场景内容
        virtual void render();
        // 注册即时事件
        virtual void regImmEvent(const EventSys::ImmEventPriority priority, const EventSys::EventFunc& func);
        // 注册定时事件
        virtual void regTimedEvent(const sf::Time delay, const EventSys::EventFunc& func);

    protected:
        // 场景中的游戏对象列表
        std::vector<GameObjs> sceneAssets;
        // Box2D物理世界生成器
        b2WorldDef worldDef;
        // Box2D物理世界
        std::optional<b2WorldId> world;
        // EventSys指针
        std::weak_ptr<EventSys> eventSysPtr;
        // RenderWindow指针
        std::weak_ptr<sf::RenderWindow> windowPtr;
        // GameInput指针
        std::weak_ptr<GameInputRead> inputPtr;
};