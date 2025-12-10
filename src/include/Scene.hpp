#pragma once
#include "GameObj.hpp"
#include "EventSys.hpp"
#include "ResourceLoader.hpp"
#include "GameInput.hpp"
#include <box2d/box2d.h>
#include <memory>
#include <vector>
#include <list>
#include <variant>
#include <SFML/Graphics.hpp>

class Scene
{   
    public:
        Scene() = default;
        ~Scene() = default;

        // 初始化场景
        virtual void init(std::string sceneConfigPath, 
                          std::weak_ptr<EventSys> eventSys, 
                          std::weak_ptr<sf::RenderWindow> window,
                          std::weak_ptr<GameInputRead> input);
        // 重载场景
        virtual void reload();
        // 更新场景状态
        virtual void update(const float deltaTime,const int subStepCount = 4);
        // 渲染场景内容
        virtual void render();
        // 注册即时事件
        virtual void regImmEvent(const EventSys::ImmEventPriority priority, const EventSys::EventFunc& func);
        // 注册定时事件
        virtual void regTimedEvent(const sf::Time delay, const EventSys::EventFunc& func);
        // 添加对象到场景
        void addObject(const std::string type, const ResourceLoader::ResourceDict& objConfig);
        // 设置玩家指针
        void setPlayerPtr(const std::shared_ptr<BaseObj>& player);
        // 获取Box2D世界ID
        b2WorldId getWorldId();
        bool isLevelCompleted() const { return levelCompleted_; }
        // 设置相机位置（用于视差背景）
        void setCameraPosition(sf::Vector2f cameraPos) { cameraPosition = cameraPos; }
        // 设置是否使用相机视差（true=关卡, false=菜单）
        void setUseParallaxWithCamera(bool use) { useParallaxWithCamera = use; }

    protected:
        // 场景中的游戏对象列表
        std::vector<std::unique_ptr<BaseObj>> sceneAssets;
        // Box2D物理世界生成器
        b2WorldDef worldDef;
        // Box2D物理世界
        std::shared_ptr<b2WorldId> world;
        // EventSys指针
        std::weak_ptr<EventSys> eventSysPtr;
        // RenderWindow指针
        std::weak_ptr<sf::RenderWindow> windowPtr;
        // GameInput指针
        std::weak_ptr<GameInputRead> inputPtr;
        // 资源加载器路径
        std::string configPath;
        // 玩家指针
        std::shared_ptr<BaseObj> playerPtr;
        // 子弹列表
        std::list<std::unique_ptr<Projectile>> projectiles; 

        //死亡界面字体
        sf::Font deathFont;
        bool deathFontLoaded = false;
        // 掉落死亡线：玩家 y 坐标超过这个值就算掉出世界
        float fallDeathY_ = 1200.0f;
        bool levelCompleted_ = false;
        // 相机位置（用于视差背景计算）
        sf::Vector2f cameraPosition = {0.0f, 0.0f};
        // 是否使用相机视差（true=关卡模式, false=菜单模式）
        bool useParallaxWithCamera = false;
};