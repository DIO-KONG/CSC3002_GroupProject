#pragma once

#include "GameObj.hpp"
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <memory>
#include <cmath>

class Player : public BaseObj
{
public:
    Player(std::weak_ptr<EventSys>        eventSys,
           std::weak_ptr<sf::RenderWindow> window,
           b2WorldId                      worldId,
           std::weak_ptr<GameInputRead>   input);

    ~Player() override;

    void initialize() override;
    void update() override;
    void update(float deltaTime);
    void setSpawnPosition(float x, float y);
    void draw() override;
    b2ShapeId getMainShapeId() const { return m_mainShapeId; }

    void setMoveSpeed(float speed) { m_moveSpeed = speed; }
    void setJumpSpeed(float speed) { m_jumpSpeed = speed; }

    //血量受伤相关接口
    void  setMaxHealth(float h);
    float getHealth() const;
    float getMaxHealth() const;
    bool  isAlive() const;
    void  heal(float amount);
    void  kill();
    void  takeDamage(float dmg);
    bool  isAliveFlag() const { return m_isAlive; }
    float getHealthRatio() const { return (m_maxHealth > 0.0f ? m_health / m_maxHealth : 0.0f); }

    
    // 设置环境对移动速度的影响（1.0 = 正常，<1.0 = 变慢）
    void setEnvSpeedScale(float s) { m_envSpeedScale = s; }
    //查询有没有踩在冰上
    bool isOnIce() const { return m_envSpeedScale < 0.999f; }

    void takeEnvironmentalDamage(float dmg);  // 新增这个

    // 给 Scene 用来做碰撞的外轮廓
    sf::FloatRect getBounds() const;


    // 明确由 Scene 传入水域区域（中心+宽高，AABB“碰撞”判断）
    void setWaterRegion(float x, float y, float w, float h)
    {
        m_hasWaterRegion = true;
        m_waterCenterX   = x;
        m_waterCenterY   = y;
        m_waterHalfW     = w * 0.5f;
        m_waterHalfH     = h * 0.5f;
    }

    bool isInWater() const { return m_inWater; }
    sf::Vector2f getPosition() const;

    struct ProjectileSpawnRequest {
        std::string type;      // "ICE" 或 "FIRE"
        sf::Vector2f position; // 生成位置
        bool facingRight;      // 朝向
    };

    // 回调类型定义
    using ProjectileSpawnCallback = std::function<void(const ProjectileSpawnRequest&)>;

    // 存储回调函数
    ProjectileSpawnCallback m_projectileCallback;

    // 设置回调的函数
    void setProjectileSpawnCallback(const ProjectileSpawnCallback& cb) {
        m_projectileCallback = cb;
    }

    // 处理发射逻辑
    void handleProjectileFire();

    // 计算子弹生成位置
    sf::Vector2f getProjectileSpawnPosition() const;
    
private:
    // ===== 物理相关 =====
    b2WorldId    m_world;
    b2BodyId     m_body{};
    sf::Vector2f m_spawnPos{0.0f, 0.0f};
    b2ShapeId    m_mainShapeId = b2_nullShapeId;

    // ===== 血量相关 =====
    float m_maxHealth          = 3.0f;
    float m_health             = 3.0f;
    bool  m_isAlive            = true;
    float m_invincibleTime     = 0.0f;   // 受伤后无敌时间计时
    float m_invincibleDuration = 1.0f;   // 无敌持续 1 秒
    float m_spawnProtectionTime = 0.0f;   // 出生后保护时间（秒）


    // ===== 移动 / 跳跃（陆地）=====
    float m_moveSpeed    = 320.0f;
    float m_jumpSpeed    = 400.0f;
    int   m_maxJumpCount = 2;
    int   m_jumpCount    = 0;
    bool  m_grounded     = false;
    bool  m_isJumpingUp  = false;
    float m_envSpeedScale = 1.0f;   // 缺省正常速度

    // ===== 水下系统 =====
    bool  m_hasWaterRegion = false;     // 是否配置了水域
    bool  m_inWater        = false;     // 当前是否在水中

    float m_waterCenterX   = 0.0f;
    float m_waterCenterY   = 0.0f;
    float m_waterHalfW     = 0.0f;
    float m_waterHalfH     = 0.0f;

    // 水下“力”的大小（可以按手感调）
    float m_buoyancyAcc  = 2300.0f;    // 上浮力（越大越往上飘）
    float m_waterDrag    = 50.0f;    // 阻尼（越大越“黏”）
    float m_diveForce    = 6000.0f;   // 下潜加速度（空格）

    void applyWaterPhysics(float dt);
    void updateWaterStateByRegion();    // 每帧根据 AABB 判断是否在水中

    // ===== 朝向 =====
    bool  m_facingRight = true;
    float m_baseScaleX  = 1.0f;

    // ===== 动画缩放 =====
    float m_targetHeight    = 0.0f;
    float m_runScaleFactor  = 2.1f;
    float m_jumpScaleFactor = 1.1f;
    float m_swimScaleFactor = 2.0f;   // 游泳动画高度缩放

    void  rescaleToTargetHeight();

    // ===== 贴图 =====
    std::optional<sf::Texture> m_idleTexture;
    std::optional<sf::Texture> m_runTexture;
    std::optional<sf::Texture> m_jumpTexture;
    std::optional<sf::Texture> m_swimTexture;   // 水下游泳贴图

    // ===== 动画帧 =====
    std::vector<sf::IntRect> m_runFrames;
    std::vector<sf::IntRect> m_swimFrames;
    sf::IntRect              m_jumpFrame;

    int   m_currentRunFrame  = 0;
    int   m_currentSwimFrame = 0;
    float m_animTimer        = 0.0f;
    float m_animFrameTime    = 0.08f;

    float m_moveDir = 0.0f;

    // ===== 射击冷却 =====
    float m_fireCooldown = 0.0f;      // 当前冷却时间
    float m_fireCooldownMax = 1.0f;   // 冷却时间上限（1秒）

    // ===== 内部逻辑 =====
    void handleHorizontalMovement();
    void handleJump();
    void updateGroundedState(float deltaTime);
    void syncSpriteWithBody();
    void updateSpriteFacing(float dirX);
    void updateAnimation(float dt);
    void applyAnimationFrame(sf::Texture& tex, const sf::IntRect& rect, float heightScale);
};