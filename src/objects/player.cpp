#include "Player.hpp"
#include "GameInput.hpp"
#include <iostream>
#include <cmath>

Player::Player(std::weak_ptr<EventSys> eventSys,
               std::weak_ptr<sf::RenderWindow> window,
               b2WorldId worldId,
               std::weak_ptr<GameInputRead> input)
    : m_world(worldId)
{
    eventSysPtr = std::move(eventSys);
    windowPtr   = std::weak_ptr<sf::RenderWindow>(window);
    inputPtr    = std::weak_ptr<GameInputRead>(input);

    features["drawable"] = true;
}

Player::~Player() {}

void Player::setSpawnPosition(float x, float y)
{
    m_spawnPos = {x, y};
}

sf::Vector2f Player::getPosition() const
{
    b2Vec2 pos = b2Body_GetPosition(m_body);
    return sf::Vector2f(pos.x, pos.y);
}

void Player::initialize()
{
    // ========== Box2D Body ==========
    b2BodyDef def = b2DefaultBodyDef();
    def.type     = b2_dynamicBody;
    def.position = { m_spawnPos.x, m_spawnPos.y };
    m_body = b2CreateBody(m_world, &def);

    b2Polygon box = b2MakeBox(0.5f, 1.0f);
    // 碰撞箱微调
    // b2Polygon box = b2MakeOffsetBox(0.5f, 1.0f, {0.0f, 40.0f}, b2Rot_identity);
    b2ShapeDef s  = b2DefaultShapeDef();
    s.density                 = 1.0f;
    s.material.friction       = 0.3f;
    s.enableContactEvents     = true; // 开启碰撞事件
    m_mainShapeId             = b2CreatePolygonShape(m_body, &s, &box);

    b2Body_ApplyMassFromShapes(m_body);
    b2Body_SetAngularDamping(m_body, 10.0f);

    // ========== 贴图 ==========
    m_idleTexture.emplace();
    if (!m_idleTexture->loadFromFile("assets/texture/player_idle.png"))
        std::cout << "Fail idle\n";

    m_runTexture.emplace();
    if (!m_runTexture->loadFromFile("assets/texture/player_run.png"))
        std::cout << "Fail run\n";

    m_jumpTexture.emplace();
    if (!m_jumpTexture->loadFromFile("assets/texture/player_jump.png"))
        std::cout << "Fail jump\n";

    // 🆕 游泳贴图
    m_swimTexture.emplace();
    if (!m_swimTexture->loadFromFile("assets/texture/player_swim.png"))
        std::cout << "Fail swim\n";

    // ========== 初始 Sprite ==========
    sprite.emplace(*m_idleTexture);
    sprite->setScale({0.1f, 0.1f});

    m_baseScaleX = sprite->getScale().x;

    auto bounds = sprite->getLocalBounds();
    sprite->setOrigin({bounds.size.x / 2, bounds.size.y / 2});

    m_targetHeight = bounds.size.y * std::fabs(sprite->getScale().y);

    // ========== Run 动画帧 ==========
    {
        auto tex   = m_runTexture->getSize();
        int frameW = tex.x / 4;
        int frameH = tex.y;

        m_runFrames.clear();
        for (int i = 0; i < 4; ++i)
        {
            m_runFrames.emplace_back(
                sf::Vector2i(i * frameW, 0),
                sf::Vector2i(frameW, frameH)
            );
        }
    }

    // ========== Swim 帧==========
    {
        auto tex   = m_swimTexture->getSize();
        int frameW = tex.x / 4;
        int frameH = tex.y;

        m_swimFrames.clear();
        for (int i = 0; i < 4; ++i)
        {
            m_swimFrames.emplace_back(
                sf::Vector2i(i * frameW, 0),
                sf::Vector2i(frameW, frameH)
            );
        }
    }

    // ========== Jump 帧 ==========
    {
        auto tex = m_jumpTexture->getSize();
        m_jumpFrame = sf::IntRect(
            sf::Vector2i(0, 0),
            sf::Vector2i(tex.x, tex.y)
        );
    }

    syncSpriteWithBody();
}

void Player::draw()
{
        // 检查类是否为可以画图的对象
    if (features.find("drawable") == features.end() || !features.at("drawable")) {
        // 该对象不支持绘制
        printf("This object is not drawable.\n");
        return;
    }
    // 检查Sprite是否存在
    if (!sprite.has_value()) {
        // 没有可用的Sprite进行绘制
        printf("No sprite available for drawing.\n");
        return;
    }
    // envrntSys不是optional类型，直接lock
    auto eventSys = eventSysPtr.lock();
    // 先从optional中取出weak_ptr指针,再对取出的weak_ptr进行lock操作
    if (windowPtr.has_value()) {
        auto window = windowPtr.value().lock();
        if (eventSys && window) {
            auto drawEvent = [this, window]() {
                window->draw(this->sprite.value());
            };
            eventSys->regImmEvent(EventSys::ImmEventPriority::DRAWPLAYER, drawEvent);
            // printf("Draw event registered.\n");
        }
        else {
            // 无法绘制，可能需要记录日志或抛出异常
        }
    }
}

void Player::update()
{
    update(1.0f / 60.0f);
}

void Player::update(float deltaTime)
{
    // === 根据重力判断：是不是“水下场景” ===
    b2Vec2 g = b2World_GetGravity(m_world);
    bool worldUnderwater = (g.y < 0.0f);

    if (worldUnderwater)
    {
        m_inWater = true;
    }
    else
    {
        m_inWater = false;
    }

    // std::cout << "[Player] inWater = " << (m_inWater ? 1 : 0)
    //           << " (gravityY=" << g.y << ")\n";

    updateGroundedState(deltaTime);
    handleHorizontalMovement();

    // === 陆地逻辑，水下逻辑 ===
    if (m_inWater)
    {
        // 水下：不再使用space跳跃t逻辑，只用浮力自动上升和下潜
        applyWaterPhysics(1.0f / 60.0f);
    }
    else
    {
        // 陆地：正常跳，二段跳
        handleJump();
    }
    updateAnimation(1.0f / 60.0f);
    syncSpriteWithBody();
}

void Player::syncSpriteWithBody()
{
    if (!sprite.has_value()) return;

    auto pos = b2Body_GetPosition(m_body);
    sprite->setPosition({pos.x, pos.y});
}

void Player::updateGroundedState(float deltaTime)
{
    b2Vec2 v = b2Body_GetLinearVelocity(m_body);
    bool wasGrounded = m_grounded;

    if (std::fabs(v.y) < 0.05f)
    {
        m_grounded = true;
        if (!wasGrounded)
        {
            m_jumpCount   = 0;
            m_isJumpingUp = false;
        }
    }
    else
    {
        m_grounded = false;
    }
}

void Player::updateSpriteFacing(float dirX)
{
    if (!sprite.has_value()) return;

    auto  s   = sprite->getScale();
    float mag = std::fabs(s.x);
    if (mag < 1e-4f) mag = 0.1f;

    if (dirX > 0.0f)
    {
        s.x           = mag;
        m_facingRight = true;
    }
    else if (dirX < 0.0f)
    {
        s.x           = -mag;
        m_facingRight = false;
    }

    sprite->setScale(s);
}

void Player::handleHorizontalMovement()
{
    auto input = inputPtr.value().lock();
    if (!input) return;

    float d = 0.0f;

    if (input->getKeyState(sf::Keyboard::Key::D) != GameInputRead::KEY_RELEASED) d += 1.0f;
    if (input->getKeyState(sf::Keyboard::Key::A) != GameInputRead::KEY_RELEASED) d -= 1.0f;

    m_moveDir = d;

    b2Vec2 v = b2Body_GetLinearVelocity(m_body);
    v.x = d * m_moveSpeed;
    b2Body_SetLinearVelocity(m_body, v);

    updateSpriteFacing(d);
}

void Player::handleJump()
{
    auto input = inputPtr.value().lock();
    if (!input) return;

    auto spaceState = input->getKeyState(sf::Keyboard::Key::Space);

    // ===== 跳跃 =====
    bool requestJump = false;

    if (spaceState == GameInputRead::KEY_PRESSED)
    {
        if (m_grounded)
        {
            // 第一次跳
            m_jumpCount = 1;
            requestJump = true;
        }
        else if (!m_grounded && m_jumpCount < m_maxJumpCount)
        {
            // 空中二段跳
            m_jumpCount++;
            requestJump = true;
        }
    }

    if (requestJump)
    {
        b2Vec2 v = b2Body_GetLinearVelocity(m_body);
        v.y = -m_jumpSpeed;
        b2Body_SetLinearVelocity(m_body, v);

        m_isJumpingUp = true; 
    }

    // ===== 可变跳高度（松开空格时停止上升）=====
    if (m_isJumpingUp)
    {
        b2Vec2 v = b2Body_GetLinearVelocity(m_body);

        if (spaceState == GameInputRead::KEY_RELEASED || v.y >= 0.0f)
        {
            if (v.y < 0.0f)
            {
                v.y *= 0.3f; 
                b2Body_SetLinearVelocity(m_body, v);
            }

            m_isJumpingUp = false;
        }
    }
}

void Player::applyWaterPhysics(float dt)
{
    // === 水下动作 ===
    b2Vec2 v = b2Body_GetLinearVelocity(m_body);

    // 速度衰减阻尼
    float dragFactor = 1.0f - m_waterDrag * dt;
    if (dragFactor < 0.0f) dragFactor = 0.0f;
    v.y *= dragFactor;

    //浮力
    v.y -= m_buoyancyAcc * dt;

    //下潜（按住space）
    auto input = inputPtr.value().lock();
    if (input)
    {
        if (input->getKeyState(sf::Keyboard::Key::Space) != GameInputRead::KEY_RELEASED)
        {
            v.y += m_diveForce * dt;
        }
    }

    b2Body_SetLinearVelocity(m_body, v);
}

void Player::rescaleToTargetHeight()
{
    if (!sprite.has_value()) return;

    auto  b   = sprite->getLocalBounds();
    float cur = b.size.y * std::fabs(sprite->getScale().y);
    if (cur <= 0.0f) return;

    float factor = m_targetHeight / cur;

    auto s = sprite->getScale();
    s.x *= factor;
    s.y *= factor;
    sprite->setScale(s);
}

void Player::applyAnimationFrame(sf::Texture& tex, const sf::IntRect& rect, float heightScale)
{
    if (!sprite.has_value()) return;

    sprite->setTexture(tex, true);
    sprite->setTextureRect(rect);

    float originalHeight = m_targetHeight;
    m_targetHeight *= heightScale;
    rescaleToTargetHeight();
    m_targetHeight = originalHeight;

    auto b = sprite->getLocalBounds();
    sprite->setOrigin({b.size.x / 2, b.size.y / 2});
}

void Player::updateAnimation(float dt)
{
    if (!sprite.has_value()) return;

    bool moving = std::fabs(m_moveDir) > 0.01f;

    // ============ 水下：游泳动画 ============
    if (m_inWater && m_swimTexture.has_value() && !m_swimFrames.empty())
    {
        m_animTimer += dt;
        if (m_animTimer >= m_animFrameTime)
        {
            m_animTimer -= m_animFrameTime;
            m_currentSwimFrame =
                (m_currentSwimFrame + 1) % static_cast<int>(m_swimFrames.size());
        }

        applyAnimationFrame(*m_swimTexture,
                            m_swimFrames[m_currentSwimFrame],
                            m_swimScaleFactor);
        return;
    }

    // ===== 空中：跳跃动画 =====
    if (!m_grounded)
    {
        applyAnimationFrame(*m_jumpTexture, m_jumpFrame, m_jumpScaleFactor);
        return;
    }

    // ===== 静止：idle =====
    if (!moving)
    {
        auto texSize = m_idleTexture->getSize();
        sf::IntRect rect(
            sf::Vector2i(0, 0),
            sf::Vector2i(static_cast<int>(texSize.x),
                         static_cast<int>(texSize.y))
        );

        applyAnimationFrame(*m_idleTexture, rect, 1.0f);
        return;
    }

    // ===== 跑步动画 =====
    m_animTimer += dt;
    if (m_animTimer >= m_animFrameTime)
    {
        m_animTimer -= m_animFrameTime;
        m_currentRunFrame =
            (m_currentRunFrame + 1) % static_cast<int>(m_runFrames.size());
    }

    applyAnimationFrame(*m_runTexture,
                        m_runFrames[m_currentRunFrame],
                        m_runScaleFactor);
}
