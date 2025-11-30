#pragma once
#include "EventSys.hpp"
#include "ResourceLoader.hpp"
#include "GameInput.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <box2d/box2d.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <optional>

// 基础游戏抽象类（不能直接实例化）
class BaseObj{
public:
    BaseObj();
    virtual ~BaseObj();
    // 纯虚函数，必须在派生类中重写
    // initialize可能需要的参数：EventSys指针、RenderWindow指针、Box2D世界指针、资源加载器返回的资源字典等
    virtual void initialize();
    // update可能需要的参数：deltaTime、输入状态等
    virtual void update();
    virtual void update(const float deltaTime);
    virtual void draw();
    virtual void regImmEvent(const EventSys::ImmEventPriority priority, const EventSys::EventFunc& func);
    virtual void regTimedEvent(const sf::Time delay, const EventSys::EventFunc& func);
    // Sprite类的信息前往 https://www.sfml-dev.org/documentation/3.0.2/classsf_1_1Sprite.html 查看

protected:
    // 类的特征 "box2d" : 是否拥有Box2D物理属性 "sound" : 是否拥有声音属性 ... 需要在initialize中设定
    std::unordered_map<std::string, bool> features;
    // sprite纹理 在initialize中设定 Sprite类保存Texture的引用，确保Texture在Sprite生命周期内有效
    std::optional<sf::Sprite> sprite;
    // 纹理资源 在initialize中设定 Sprite类保存Texture的引用，确保Texture在Sprite生命周期内有效
    std::optional<sf::Texture> texture;
    // RenderWindow指针 在initialize中设定
    std::optional<std::weak_ptr<sf::RenderWindow>> windowPtr;
    // Box2D世界指针 在initialize中设定
    std::optional<std::weak_ptr<b2WorldId>> worldPtr;
    // 游戏输入状态 在initialize中设定
    std::optional<std::weak_ptr<GameInputRead>> inputPtr;
    // 任务系统指针 在initialize中设定
    std::weak_ptr<EventSys> eventSysPtr;
};

// 方块对象类
class Block : public BaseObj{
public:
    // 方块类型枚举
    enum BlockType {
        GRASS,
        WATER,
        ICE,
        LAVA
    };

    Block();
    ~Block() override;
    // 重写基类方法，通过Scene的addObject调用
    void initialize(const ResourceLoader::ResourceDict& objConfig);
    // 设置核心指针（方块类不需要输入）
    void setPtrs(const std::weak_ptr<EventSys>& eventSys,
                 const std::weak_ptr<sf::RenderWindow>& window,
                 const std::weak_ptr<b2WorldId>& world);
    void update() override;
    // void draw() override;
    void onhit(float damage);
    void onkill();

private:
    // 方块的生命值 可以设置极高表示不可破坏
    float health;
    // 方块类型
    BlockType blockType;
};