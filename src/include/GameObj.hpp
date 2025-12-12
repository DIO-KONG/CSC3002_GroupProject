
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

    void setWindowPtr(const std::weak_ptr<sf::RenderWindow>& win) { windowPtr.emplace(win); }
    void setEventSysPtr(const std::weak_ptr<EventSys>& eventSys) { eventSysPtr = eventSys; }

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

// 图形类（包括背景，按钮图形等）
class GraphicObj : public BaseObj{
public:
    // 图形类型枚举
    enum GraphicType {
        BACKGROUND,
        BUTTON
    };
    GraphicObj();
    ~GraphicObj() override;
    // 重写基类方法，通过Scene的addObject调用
    void initialize(const ResourceLoader::ResourceDict& objConfig);
    void setPtrs(const std::weak_ptr<EventSys>& eventSys,
                 const std::weak_ptr<sf::RenderWindow>& window,
                 const std::weak_ptr<GameInputRead>& input);
    void update(float deltaTime) override;
    void draw() override;

private:
    GraphicType graphicType;
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

    BlockType getBlockType() const { return blockType; }
    bool isLava() const { return blockType == LAVA; }
    bool isIce()  const { return blockType == ICE;  }
    sf::FloatRect getHitBox() const;
    
    Block();
    ~Block() override;
    // 重写基类方法，通过Scene的addObject调用
    void initialize(const ResourceLoader::ResourceDict& objConfig);
    // 设置核心指针（方块类不需要输入）
    void setPtrs(const std::weak_ptr<EventSys>& eventSys,
                 const std::weak_ptr<sf::RenderWindow>& window,
                 const std::weak_ptr<b2WorldId>& world);
    void update(float deltaTime) override;
    // void draw() override;
    void onhit(float damage);
    void onkill();

private:
    // 方块的生命值 可以设置极高表示不可破坏
    float health;
    // 方块类型
    BlockType blockType;
    // box2d
    b2BodyId groundId;
};

class Enemy : public BaseObj{
public:
    Enemy();
    ~Enemy() override;
    // 通过Scene的addObject调用initialize方法
    void initialize(const ResourceLoader::ResourceDict& objConfig);
    // 设置核心指针（敌人类可能需要输入,比如如果你不会做玩家攻击，那么就直接绑定一个键收到攻击，玩家按下那个键敌人就受伤）
    void setPtrs(const std::weak_ptr<EventSys>& eventSys,
                 const std::weak_ptr<sf::RenderWindow>& window,
                 const std::weak_ptr<b2WorldId>& world,
                 const std::weak_ptr<GameInputRead>& input);
    // update方法(在这里更新敌人的AI行为，同时根据位置更新sprite的位置)
    void update(float deltaTime) override;
    void draw() override;
    // 收到攻击
    void onhit(float damage);
    void onkill();
    // 攻击玩家
    void attackPlayer();
    // 查询状态 / 碰撞箱
    bool isAliveFlag() const {return isAlive;};
    sf::FloatRect getHitBox() const;
    float getAttackDamage() const { return attackDamage; }



private:
    // 敌人的生命值,最大生命值，攻击力，攻击冷却等属性
    float health;
    float maxHealth;
    float attackDamage;
    float attackCooldown;
    // 敌人是否存活，面向方向
    bool isAlive, faceRight;
    // box2d属性
    b2BodyId bodyId;
    b2Vec2 velocity;
    b2Vec2 patrolPointA;
    b2Vec2 patrolPointB;
    sf::Vector2f boxparams; // 用于存储方块的宽度和高度
    // 动画相关（敌人专用）
    std::vector<sf::IntRect> animFrames;  // 每一帧的矩形
    int   currentAnimFrame = 0;           // 当前帧索引
    float animTimer        = 0.0f;        // 计时器
    float animFrameTime    = 0.15f;       // 每帧持续时间（秒）

};

class Projectile : public BaseObj {
public:
    enum ProjectileType {
        ICE,
        FIRE
    };

    Projectile();
    ~Projectile() override;

    void initializeDynamic(ProjectileType type,
                           const sf::Vector2f& position,
                           bool facingRight);
    void update(const float deltaTime) override;

    // 状态 / 伤害 / 碰撞箱
    bool isActive() const { return isActive_; }
    void deactivate()     { isActive_ = false; }

    float getDamage() const { return damage; }
    ProjectileType getType() const { return projectileType; }

    sf::FloatRect getBounds() const;

    void draw() override;
    static ProjectileType fromString(const std::string& typeStr);

private:
    ProjectileType projectileType;
    float speed;
    float damage;
    float lifetime;
    float maxLifetime;
    bool isActive_;
    bool faceRight;
    sf::Vector2f projectilePos;
    sf::Vector2f direction;
    std::string texturePath;
    b2BodyId bodyId;
};

class Trap : public BaseObj {
public:
    enum TrapType {
        SPIKE,
        GOAL
    };

     enum ElementType {
        NEUTRAL,        // 无属性
        FIRE_ELEMENT,   // 火属性刺
        ICE_ELEMENT     // 冰属性刺（以后你想做也可以）
    };
    
    Trap();
    ~Trap() override;

    void initialize(const ResourceLoader::ResourceDict& objConfig);
    void setPtrs(const std::weak_ptr<EventSys>& eventSys,
                 const std::weak_ptr<sf::RenderWindow>& window,
                 const std::weak_ptr<b2WorldId>& world);
    void update(float deltaTime) override;
    void draw() override;

    bool isActive() const { return isActive_; }
    void activate();

    
    // 元素和血量接口
    void onHitByProjectile(Projectile::ProjectileType projType, float dmg);
    bool isDestroyed() const { return destroyed_; }
    ElementType getElement() const { return element; }

    bool checkTrigger(const sf::FloatRect& playerBounds) const;

    float getDamage() const { return damage; }
    sf::FloatRect getHitBox() const;
    
    bool getHasDamagedPlayer() const { return hasDamagedPlayer; }
    void setHasDamagedPlayer(bool v) { hasDamagedPlayer = v; }

    bool isGoal() const { return isGoal_; }
    
private:
    TrapType trapType = SPIKE;
    float damage;

    //元素和血量
    ElementType element = NEUTRAL;
    float maxHealth = 0.0f;
    float health = 0.0f;
    bool destroyed_ = false;
    bool hasDamagedPlayer = false;
    bool isActive_=false;
    bool isGoal_ = false;
    sf::Vector2f trapPos;
    sf::FloatRect triggerArea;
    // sf::Vector2f appearOffset;
    b2BodyId bodyId;
};

// 视差滚动图层类
class ParallaxLayer : public BaseObj {
public:
    ParallaxLayer();
    ~ParallaxLayer() override;

    void initialize(const ResourceLoader::ResourceDict& objConfig);
    void setPtrs(const std::weak_ptr<EventSys>& eventSys,
                 const std::weak_ptr<sf::RenderWindow>& window);
    void update(float deltaTime) override;
    void updateWithCamera(float deltaTime, sf::Vector2f cameraPos);
    void draw() override;

private:
    std::optional<sf::Sprite> sprite1;        // 精灵（使用纹理重复模式）
    std::optional<sf::Texture> texture;       // 图层纹理
    float scrollSpeed;         // 滚动速度（视差系数，0.0-1.0）
    float textureWidth;        // 纹理宽度
    float textureHeight;       // 纹理高度
    float currentOffset;       // 当前偏移量
    float yPosition;           // Y 坐标位置
    int layerIndex;            // 图层索引（用于确定绘制优先级）
    float baseOffset;          // 基础偏移量（用于时间动画）
    float levelWidth;          // 关卡宽度（用于纹理矩形大小）
};