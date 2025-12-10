#include "GameObj.hpp"
#include <cmath>

BaseObj::BaseObj(){
    // 构造函数
}

BaseObj::~BaseObj(){
    // 析构函数
}

void BaseObj::initialize() {
    // 默认初始化行为，可以在派生类中重写
}

void BaseObj::update() {
    // 默认更新行为，可以在派生类中重写
}

void BaseObj::update(const float deltaTime) {
    // 带有deltaTime参数的默认更新行为，可以在派生类中重写
    (void)deltaTime; // 避免未使用参数的警告
}

void BaseObj::draw() {
    // 默认绘制行为，通过任务系统调度绘制事件
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
            eventSys->regImmEvent(EventSys::ImmEventPriority::DRAW, drawEvent);
            // printf("Draw event registered.\n");
        }
        else {
            // 无法绘制，可能需要记录日志或抛出异常
        }
    }
}

void BaseObj::regImmEvent(const EventSys::ImmEventPriority priority, const EventSys::EventFunc& func) {
    // 注册即时事件
    if (auto eventSys = eventSysPtr.lock()) {
        eventSys->regImmEvent(priority, func);
    }
}

void BaseObj::regTimedEvent(const sf::Time delay, const EventSys::EventFunc& func) {
    // 注册定时事件
    if (auto eventSys = eventSysPtr.lock()) {
        eventSys->regTimedEvent(delay, func);
    }
}

// -------------------------------- GraphicObj类实现 --------------------------------
GraphicObj::GraphicObj() : BaseObj() {
    // 构造函数
}

GraphicObj::~GraphicObj() {
    // 析构函数
}

void GraphicObj::initialize(const ResourceLoader::ResourceDict& objConfig) {
    // 初始化图形对象
    // Debug
    printf(".............Initializing GraphicObj...........\n");
    // 设置特征，例如支持绘制
    features["drawable"] = true;
    // 设置图形类型
    std::string typeStr = std::get<std::string>(objConfig.at("type"));
    // 解析objConfig以设置图形类型（BACKGROUND或BUTTON）
    if (typeStr == "BACKGROUND") {
        // 处理背景图形的特定初始化
        // Debug
        printf("Type is BACKGROUND\n");
        graphicType = BACKGROUND;
    }else if (typeStr == "BUTTON") {
        // 处理按钮图形的特定初始化
        // Debug
        printf("Type is BUTTON\n");
        graphicType = BUTTON;
    }
    // Debug
    printf("..........Loading Texture and Setting Sprite..........\n");
    // 解析objConfig以设置纹理等
    std::string texturePath = std::get<std::string>(objConfig.at("texture"));
    printf("Texture Path: %s\n", texturePath.c_str());
    texture.emplace();
    if (texture->loadFromFile(texturePath)) {
        sprite.emplace(texture.value());
        // Debug
        printf("Texture and Sprite Loaded.\n");
    }
    // 设置纹理位置
    float posX = std::get<float>(objConfig.at("x"));
    float posY = std::get<float>(objConfig.at("y"));
    sf::Vector2f position(posX, posY);
    if (sprite.has_value()) {
        sprite->setPosition(position);
    }
    // Debug
    printf("...............GraphicObj initialized................\n");
}

void GraphicObj::setPtrs(const std::weak_ptr<EventSys>& eventSys,
                         const std::weak_ptr<sf::RenderWindow>& window,
                         const std::weak_ptr<GameInputRead>& input) {
    eventSysPtr = eventSys;
    windowPtr.emplace(window);
    inputPtr.emplace(input);
}

void GraphicObj::update(float deltaTime) {
    // 更新图形对象状态
    this->draw();
}

void GraphicObj::draw() {
    // 检查类是否为可以画图的对象
    if (features.find("drawable") == features.end() || !features.at("drawable")) {
        // 该对象不支持绘制
        printf("This object is not drawable.\n");
        return;
    }
    // 检查Sprite是否存在
    if (!sprite.has_value()) {
        // 没有可用的Sprite进行绘制
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
            eventSys->regImmEvent(EventSys::ImmEventPriority::DRAWBACKGROUND, drawEvent);
            // printf("Draw event registered.\n");
        }
        else {
            // 无法绘制，可能需要记录日志或抛出异常
        }
    }
}


// -------------------------------- Block类实现 --------------------------------

Block::Block() : BaseObj() {
    // 构造函数
}

Block::~Block() {
    // 析构函数
}

void Block::initialize(const ResourceLoader::ResourceDict& objConfig) {
    // 初始化方块对象
    // 设置特征，例如支持绘制
    features["drawable"] = true;
    features["box2d"] = true;
    // 解析objConfig以设置方块类型和生命值
    std::string typeStr = std::get<std::string>(objConfig.at("type"));
    health = std::get<float>(objConfig.at("health"));
    // 根据typeStr设置blockType
    if (typeStr == "GRASS") {
        blockType = GRASS;
    }else if (typeStr == "WATER") {
        blockType = WATER;
    }else if (typeStr == "ICE") {
        blockType = ICE;
    }else if (typeStr == "LAVA") {
        blockType = LAVA;
    }
    // 加载纹理和设置Sprite
    std::string texturePath = std::get<std::string>(objConfig.at("texture"));
    texture.emplace();
    if (texture->loadFromFile(texturePath)) {
        sprite.emplace(texture.value());
    }
    // 设置纹理位置
    float posX = std::get<float>(objConfig.at("x"));
    float posY = std::get<float>(objConfig.at("y"));
    sf::Vector2f position(posX, posY);
    if (sprite.has_value()) {
        sprite->setPosition(position);
    }

    // 设置碰撞箱参数等（根据不同type）
    float width = std::get<float>(objConfig.at("width"));
    float height = std::get<float>(objConfig.at("height"));
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    b2Vec2 Bodyposition = {posX+width/2, posY+height/2};
    groundBodyDef.position = Bodyposition; // Box2D坐标系中心点
    groundBodyDef.type = b2_staticBody; // 静态物体
    // 创建Box2D实体和形状
    groundId = b2CreateBody(*worldPtr->lock(), &groundBodyDef);
    // b2Polygon groundBox = b2MakeBox(width/2, height/2);
    // Debug
    printf("Block Box2D body created at (%.2f, %.2f) with size (%.2f, %.2f)\n", posX, posY, width, height);
    b2Polygon groundBox;
    // 微调碰撞箱位置（根据不同类型）
    if (blockType == GRASS) {
        groundBox = b2MakeOffsetBox(width/2, height/2, {0.0f, -height*0.65f}, b2Rot_identity);
        // 设置草地方块的物理属性
        // 草方块作为固定平台（不可破坏）
    }else if (blockType == WATER) {
        groundBox = b2MakeOffsetBox(width/2, height/2, {0.0f, -height*0.65f}, b2Rot_identity);
        // 设置水地方块的属性
    }else if (blockType == ICE) {
        groundBox = b2MakeOffsetBox(width/2, height/2, {0.0f, -height*0.55f}, b2Rot_identity);
        // 设置冰地方块的属性
    }else if (blockType == LAVA) {
        groundBox = b2MakeOffsetBox(width/2, height/2, {0.0f, -height*0.50f}, b2Rot_identity);
        // 设置熔岩地方块的属性
    }
    b2ShapeDef groundShapeDef = b2DefaultShapeDef ();
    b2CreatePolygonShape (groundId, &groundShapeDef, &groundBox);
}

void Block::setPtrs(const std::weak_ptr<EventSys>& eventSys,
                    const std::weak_ptr<sf::RenderWindow>& window,
                    const std::weak_ptr<b2WorldId>& world) {
    eventSysPtr = eventSys;
    windowPtr.emplace(window);
    worldPtr.emplace(world);
}

void Block::update(float deltaTime) {
    // 更新方块状态
    // 例如处理与玩家的交互、动画等
}

void Block::onhit(float damage) {
    health -= damage;
    if (health < 0) {
        onkill();
        health = 0;
    }
}

void Block::onkill() {
    // 方块被破坏时的处理逻辑
    // 例如播放破坏动画、移除方块等
}

// -------------------------------- Enemy类实现 --------------------------------

Enemy::Enemy() : BaseObj() {
    // 构造函数
}

Enemy::~Enemy() {
    // 析构函数
}

void Enemy::initialize(const ResourceLoader::ResourceDict& objConfig) {
    // 初始化敌人对象
    // 设置特征，例如支持绘制和Box2D物理
    features["drawable"] = true;
    features["box2d"] = true;
    // 解析objConfig以设置敌人类型、生命值等
    maxHealth      = std::get<float>(objConfig.at("health"));
    health         = maxHealth;
    attackDamage   = std::get<float>(objConfig.at("attackDamage"));
    attackCooldown = std::get<float>(objConfig.at("attackCooldown"));
    faceRight      = true;
    isAlive        = true;

    // 敌人巡逻路径，到端点调头
    patrolPointA = {std::get<float>(objConfig.at("patrolAx")),
                    std::get<float>(objConfig.at("patrolAy"))};
    patrolPointB = {std::get<float>(objConfig.at("patrolBx")),
                    std::get<float>(objConfig.at("patrolBy"))};
    // 加载纹理和设置Sprite
    std::string texturePath = std::get<std::string>(objConfig.at("texture"));
    texture.emplace();
    if (texture->loadFromFile(texturePath)) {
        sprite.emplace(texture.value());
    } else {
        // 纹理加载失败处理
        printf("Failed to load enemy texture from %s\n", texturePath.c_str());
    }
    // 设置box2d实体(使用临时变量加载数据，构建实体后，临时变量会被释放)
    float posX = std::get<float>(objConfig.at("x"));
    float posY = std::get<float>(objConfig.at("y"));
    float width = std::get<float>(objConfig.at("width"));
    float height = std::get<float>(objConfig.at("height"));
    float density = std::get<float>(objConfig.at("density"));
    float friction = std::get<float>(objConfig.at("friction"));
    float velocityX = std::get<float>(objConfig.at("velocityX"));
    float velocityY = std::get<float>(objConfig.at("velocityY"));
    boxparams = {width, height};

    // 创建Box2D实体和形状
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2Vec2 Bodyposition = {posX+width/2, posY+height/2};
    bodyDef.position = Bodyposition; // Box2D坐标系中心点
    // bodyDef.fixedRotation = true; // 不允许旋转
    bodyDef.type = b2_dynamicBody; // 动态物体

    bodyId = b2CreateBody(*worldPtr->lock(), &bodyDef);

    b2Polygon box = b2MakeBox(width/2, height/2);
    b2ShapeDef shapeDef = b2DefaultShapeDef ();
    shapeDef.density = density;
    shapeDef.material.friction = friction;
    b2CreatePolygonShape (bodyId, &shapeDef, &box);

    // 设置初始速度
    velocity = {velocityX, velocityY};
    b2Body_SetLinearVelocity(bodyId, velocity);

    // Debug
    printf("Enemy Box2D body created...\n");
}

void Enemy::setPtrs(const std::weak_ptr<EventSys>& eventSys,
                    const std::weak_ptr<sf::RenderWindow>& window,
                    const std::weak_ptr<b2WorldId>& world,
                    const std::weak_ptr<GameInputRead>& input) {
    eventSysPtr = eventSys;
    windowPtr.emplace(window);
    worldPtr.emplace(world);
    inputPtr.emplace(input);
}

void Enemy::update(float deltaTime) {
    // Debug
    // printf("Updating Enemy...\n");

    // 更新敌人状态
    if (!isAlive) return;

    // 简单巡逻AI
    b2Vec2 position = b2Body_GetPosition(bodyId);
    if (faceRight) {
        // 向右移动
        b2Body_SetLinearVelocity(bodyId, {std::abs(velocity.x), velocity.y});
        if (position.x >= patrolPointB.x) {
            faceRight = false; // 到达右端点，调头
        }
    } else {
        // 向左移动
        b2Body_SetLinearVelocity(bodyId, {-std::abs(velocity.x), velocity.y});
        if (position.x <= patrolPointA.x) {
            faceRight = true; // 到达左端点，调头
        }
    }

    // 根据Box2D实体位置更新Sprite位置，注意坐标转换
    if (sprite.has_value()) {
        sf::Vector2f spritePosition = {position.x - boxparams.x / 2, position.y - boxparams.y / 2};
        sprite->setPosition(spritePosition);
    }

    // 
    if (attackCooldown > 0) {
        attackCooldown -= deltaTime; // 使用传入的deltaTime
    }
}

void Enemy::draw() {
    if (isAlive) {
        // Debug
        // printf("Drawing Enemy......\n");
        BaseObj::draw();
    }
}

void Enemy::onhit(float damage) {
    // 已经死亡就不再处理
    if (!isAlive) return;

    health -= damage;
    if (health <= 0.0f) {
        health = 0.0f;
        onkill();
    }
}

void Enemy::onkill() {
    // 敌人被击败时的处理逻辑
    isAlive = false;
    printf("Enemy killed!\n");
    // TODO: 这里之后可以加死亡动画 / 销毁 Box2D Body / 从 Scene 中移除
}


sf::FloatRect Enemy::getHitBox() const
{
    if (!sprite.has_value()) {
        return sf::FloatRect();
    }
    return sprite->getGlobalBounds();
}

sf::FloatRect Block::getHitBox() const
{
    if (!sprite.has_value()) {
        return sf::FloatRect();
    }
    return sprite->getGlobalBounds();
}



// -------------------------------- Projectile类实现 --------------------------------

Projectile::Projectile() : BaseObj() {
    // 构造函数
}

Projectile::~Projectile() {
    // 析构函数
}

Projectile::ProjectileType Projectile::fromString(const std::string& typeStr) {
    if (typeStr == "ICE") return ICE;
    if (typeStr == "FIRE") return FIRE;
    return ICE;
}

void Projectile::initializeDynamic(ProjectileType type, 
                                   const sf::Vector2f& position, 
                                   bool facingRight) {
    // printf("[Projectile::initializeDynamic] START\n");
    // printf("[Projectile]   Type: %d\n", (int)type);
    // printf("[Projectile]   Position: (%.2f, %.2f)\n", position.x, position.y);
    // printf("[Projectile]   FacingRight: %s\n", facingRight ? "true" : "false");
    
    features["drawable"] = true;   

    // 设置类型
    projectileType = type;

    // 设置初始位置
    projectilePos = position;

    // 设置速度（硬编码）
    if (type == ProjectileType::ICE) {
        speed = 400.0f;
        damage = 1.0f;
        texturePath = "assets/texture/iceball.png";
        // printf("[Projectile]   ICE - speed=%.2f\n", speed);
    } else if (type == ProjectileType::FIRE) {
        speed = 500.0f;
        damage = 1.0f;
        texturePath = "assets/texture/fireball.png";
        // printf("[Projectile]   FIRE - speed=%.2f\n", speed);
    }

    // 设置朝向
    faceRight = facingRight;

    // 设置速度方向
    direction = sf::Vector2f(faceRight ? speed : -speed, 0.0f);
    // printf("[Projectile]   Direction: (%.2f, %.2f)\n", direction.x, direction.y);
    
    lifetime = 0.0f;
    isActive_ = true;
    // printf("[Projectile]   isActive: true\n");

    // 加载贴图（硬编码）
    texture.emplace();
    if (!texture->loadFromFile(texturePath)) {
        // printf("[Projectile]   ERROR: Failed to load texture from %s\n", texturePath.c_str());
    } else {
        // printf("[Projectile]   Texture loaded successfully from %s\n", texturePath.c_str());
    }
    
    sprite.emplace(*texture);
    
    // 设置贴图缩放：x 0.04，y 0.05
    // 方向翻转：朝右是正数，朝左是负数（翻转贴图）
    float scaleX = faceRight ? -0.04f : 0.04f;  // 注意：原始贴图朝左，所以朝右需要翻转
    sprite->setScale({scaleX, 0.05f});
    // printf("[Projectile]   Sprite scale set to (%.2f, %.2f)\n", scaleX, 0.05f);
    
    // 设置贴图中心点，使得翻转以中心为轴
    auto bounds = sprite->getLocalBounds();
    sprite->setOrigin({bounds.size.x / 2.0f, bounds.size.y / 2.0f});
    // printf("[Projectile]   Sprite origin set to (%.2f, %.2f)\n", bounds.size.x / 2.0f, bounds.size.y / 2.0f);
    
    sprite->setPosition(position);
    // printf("[Projectile]   Sprite created and positioned\n");
    // printf("[Projectile::initializeDynamic] COMPLETE\n");
}

void Projectile::update(const float deltaTime) {
    if (!isActive_) {
        // printf("[Projectile::update] Projectile inactive, skipping\n");
        return;
    }
    
    // 简单直线运动
    sf::Vector2f oldPos = projectilePos;
    projectilePos += direction * deltaTime;
    
    // printf("[Projectile::update] pos: (%.2f, %.2f) -> (%.2f, %.2f), lifetime: %.2fs\n",
        //    oldPos.x, oldPos.y, projectilePos.x, projectilePos.y, lifetime);
    
    if (sprite.has_value()) {
        sprite->setPosition(projectilePos);
    } else {
        // printf("[Projectile::update] WARNING: No sprite available!\n");
    }
    
    // 生命周期管理
    lifetime += deltaTime;
    if (lifetime > 3.0f) { // 例如3秒后消失
        // printf("[Projectile::update] Lifetime expired, deactivating\n");
        isActive_ = false;
    }
}

void Projectile::draw() {
    if (!isActive_) {
        // printf("[Projectile::draw] Projectile inactive, not drawing\n");
        return;
    }
    
    // printf("[Projectile::draw] Drawing at pos=(%.2f, %.2f)\n", projectilePos.x, projectilePos.y);
    
    if (!sprite.has_value()) {
        // printf("[Projectile::draw] ERROR: No sprite!\n");
        return;
    }
    
    if (!texture.has_value()) {
        // printf("[Projectile::draw] ERROR: No texture!\n");
        return;
    }
    
    BaseObj::draw();
    // printf("[Projectile::draw] BaseObj::draw() completed\n");
}

sf::FloatRect Projectile::getBounds() const
{
    if (sprite.has_value()) {
        return sprite->getGlobalBounds();
    }
    return sf::FloatRect();
}

// -------------------------------- Trap类实现 --------------------------------
Trap::Trap() : BaseObj() {
    // 默认状态：未激活、未被摧毁
    isActive_ = false;
    destroyed_ = false;
    hasDamagedPlayer = false;
}

Trap::~Trap() {
}

void Trap::initialize(const ResourceLoader::ResourceDict& objConfig) {
    //重置所有状态变量
    isActive_ = false;
    destroyed_ = false;
    hasDamagedPlayer = false;    
    health = maxHealth;           

    // 初始化陷阱对象
    features["drawable"] = true;

    // ========== 基础类型 ==========
    std::string typeStr = std::get<std::string>(objConfig.at("type"));
    if (typeStr == "SPIKE") {
        trapType = TrapType::SPIKE;
        isGoal_ = false;
    }
    else if (typeStr == "GOAL") {
        trapType = TrapType::GOAL;
        isGoal_ = true;
        damage = 0.f; 
    }


    // 伤害（碰到玩家时掉多少血）
    damage = std::get<float>(objConfig.at("damage"));

    // ========== 血量 & 元素 ==========
    if (objConfig.count("health")) {
        maxHealth = std::get<float>(objConfig.at("health"));
        health    = maxHealth;
    } else {
        maxHealth = health = 2.0f;   // 默认打两下碎
    }

    if (objConfig.count("element")) {
        std::string elemStr = std::get<std::string>(objConfig.at("element"));
        if (elemStr == "FIRE") {
            element = FIRE_ELEMENT;
        } else if (elemStr == "ICE") {
            element = ICE_ELEMENT;
        } else {
            element = NEUTRAL;
        }
    } else {
        element = NEUTRAL;
    }

    // ========== 触发区域 ==========
    float triggerX = std::get<float>(objConfig.at("triggerX"));
    float triggerY = std::get<float>(objConfig.at("triggerY"));
    float triggerW = std::get<float>(objConfig.at("triggerWidth"));
    float triggerH = std::get<float>(objConfig.at("triggerHeight"));
    triggerArea = sf::FloatRect({triggerX, triggerY}, {triggerW, triggerH});

    // ========== 贴图 & Sprite ==========
    std::string texturePath = std::get<std::string>(objConfig.at("texture"));
    texture.emplace();
    if (texture->loadFromFile(texturePath)) {
        sprite.emplace(texture.value());
    }

    float posX = std::get<float>(objConfig.at("x"));
    float posY = std::get<float>(objConfig.at("y"));
    trapPos = {posX, posY};

    float width  = std::get<float>(objConfig.at("width"));
    float height = std::get<float>(objConfig.at("height"));

    if (sprite.has_value() && texture.has_value()) {
        auto texSize = texture->getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            float scaleX = width  / static_cast<float>(texSize.x);
            float scaleY = height / static_cast<float>(texSize.y);

            const float upscaling = 2.0f;   // 你原来用的放大倍数
            sprite->setScale({scaleX * upscaling, scaleY * upscaling});

            auto bounds = sprite->getLocalBounds();
            sprite->setOrigin({bounds.size.x / 2.0f, bounds.size.y / 2.0f});
        }
        sprite->setPosition(trapPos);
    }

    // ========== Box2D 静态刚体 ==========
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2Vec2 bodyPos = {posX + width / 2.0f, posY + height / 2.0f};
    bodyDef.position = bodyPos;
    bodyDef.type     = b2_staticBody;

    bodyId = b2CreateBody(*worldPtr->lock(), &bodyDef);
    b2Polygon box = b2MakeBox(width / 2.0f, height / 2.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(bodyId, &shapeDef, &box);
}

void Trap::setPtrs(const std::weak_ptr<EventSys>& eventSys,
                   const std::weak_ptr<sf::RenderWindow>& window,
                   const std::weak_ptr<b2WorldId>& world) {
    eventSysPtr = eventSys;
    windowPtr.emplace(window);
    worldPtr.emplace(world);
}

void Trap::update(float /*deltaTime*/) {
    // 目前陷阱是静态的，不需要 Tick 逻辑
}

void Trap::activate() {
    isActive_ = true;
    printf("[Trap] ACTIVATED at (%.1f, %.1f)\n", trapPos.x, trapPos.y);
    }

void Trap::draw() {
    // 只在激活且没被摧毁时绘制
    if (isActive_ && !destroyed_) {
        BaseObj::draw();
    }
}

// 返回陷阱对玩家造成伤害的碰撞箱（激活且没被摧毁）
sf::FloatRect Trap::getHitBox() const {
    if (!sprite.has_value() || !isActive_ || destroyed_) {
        return sf::FloatRect();
    }
    return sprite->getGlobalBounds();
}

// 检查玩家是否进入“触发区域”（用来启动突刺）
bool Trap::checkTrigger(const sf::FloatRect& playerBounds) const {
    float pLeft   = playerBounds.position.x;
    float pTop    = playerBounds.position.y;
    float pRight  = pLeft + playerBounds.size.x;
    float pBottom = pTop  + playerBounds.size.y;

    float tLeft   = triggerArea.position.x;
    float tTop    = triggerArea.position.y;
    float tRight  = tLeft + triggerArea.size.x;
    float tBottom = tTop  + triggerArea.size.y;

    bool intersect =
        (pLeft < tRight)  &&
        (pRight > tLeft)  &&
        (pTop < tBottom)  &&
        (pBottom > tTop);

    return intersect;
}

// 子弹打到陷阱（元素克制 + 扣血 + 摧毁）
void Trap::onHitByProjectile(Projectile::ProjectileType projType, float dmg) {
    if (destroyed_ || !isActive_) return;

    // ===== 元素克制：例如火刺只能被 ICE 打掉 =====
    if (element == FIRE_ELEMENT && projType != Projectile::ICE) {
        printf("[Trap] Fire spike hit by non-ICE projectile, no effect.\n");
        return;
    }
    if (element == ICE_ELEMENT && projType != Projectile::FIRE) {
        printf("[Trap] Ice spike hit by non-FIRE projectile, no effect.\n");
        return;
    }

    // ===== 扣血 =====
    health -= dmg;
    printf("[Trap] Hit by projectile, hp = %.2f / %.2f\n", health, maxHealth);

    if (health <= 0.0f) {
        health     = 0.0f;
        destroyed_ = true;
        isActive_  = false;
        printf("[Trap] Destroyed!\n");

        // 销毁 Box2D Body，让玩家能穿过去 / 跳过去
        b2DestroyBody(bodyId);
    }
    }

// -------------------------------- ParallaxLayer类实现 --------------------------------

ParallaxLayer::ParallaxLayer() : BaseObj() {
    features["drawable"] = true;
    currentOffset = 0.0f;
    baseOffset = 0.0f;
    scrollSpeed = 0.0f;
    textureWidth = 0.0f;
    textureHeight = 0.0f;
    yPosition = 0.0f;
    layerIndex = 0;
    levelWidth = 10000.0f; // 默认关卡宽度，足够覆盖大多数情况
}

ParallaxLayer::~ParallaxLayer() {
    // 析构函数
}

void ParallaxLayer::initialize(const ResourceLoader::ResourceDict& objConfig) {
    // Debug
    printf(".............Initializing ParallaxLayer...........\n");

    // 加载纹理路径
    std::string texturePath = std::get<std::string>(objConfig.at("texture"));
    // Debug
    printf("Parallax texture path: %s\n", texturePath.c_str());
    // 加载纹理
    sf::Texture tempTexture;
    if (!tempTexture.loadFromFile(texturePath)) {
        printf("Failed to load parallax texture: %s\n", texturePath.c_str());
        return;
    }
    printf("Parallax texture loaded: %s\n", texturePath.c_str());
    
    // 设置纹理为重复模式（关键：支持纹理平铺）
    tempTexture.setRepeated(true);
    
    // 获取纹理尺寸
    textureWidth = static_cast<float>(tempTexture.getSize().x);
    textureHeight = static_cast<float>(tempTexture.getSize().y);
    
    // 保存纹理
    texture = tempTexture;
    
    // 创建精灵
    sprite1 = sf::Sprite(texture.value());
    
    // 获取滚动速度
    scrollSpeed = std::get<float>(objConfig.at("speed"));
    printf("Parallax scroll speed: %.2f\n", scrollSpeed);
    
    // 获取Y位置
    yPosition = std::get<float>(objConfig.at("y"));
    printf("Parallax Y position: %.2f\n", yPosition);
    
    // 获取图层索引（用于确定绘制优先级）
    layerIndex = (std::get<int>(objConfig.at("layer")));
    printf("Parallax layer index: %d\n", layerIndex);
    
    // 设置精灵位置
    sprite1.value().setPosition({0.0f, yPosition});
    
    // 设置纹理矩形：覆盖整个关卡宽度
    // 使用足够大的宽度确保覆盖整个关卡
    sprite1.value().setTextureRect(sf::IntRect(
        {0, 0},
        {static_cast<int>(levelWidth), static_cast<int>(textureHeight)}
    ));
    
    printf("ParallaxLayer initialized: layer=%d, speed=%.2f, y=%.2f\n", 
           layerIndex, scrollSpeed, yPosition);
}

void ParallaxLayer::setPtrs(const std::weak_ptr<EventSys>& eventSys,
                            const std::weak_ptr<sf::RenderWindow>& window) {
    eventSysPtr = eventSys;
    windowPtr = window;
}

void ParallaxLayer::update(float deltaTime) {
    // 基础时间动画（菜单场景使用）
    baseOffset += scrollSpeed * deltaTime;
    
    // 使用模运算保持在纹理宽度范围内（无需循环判断）
    if (textureWidth > 0) {
        baseOffset = std::fmod(baseOffset, textureWidth);
        if (baseOffset < 0) baseOffset += textureWidth;
    }
    
    // 更新纹理矩形的偏移（不改变精灵位置）
    if (sprite1.has_value()) {
        int offsetX = static_cast<int>(baseOffset);
        sprite1.value().setTextureRect(sf::IntRect(
            {offsetX, 0},
            {static_cast<int>(levelWidth), static_cast<int>(textureHeight)}
        ));
    }
}

void ParallaxLayer::updateWithCamera(float deltaTime, sf::Vector2f cameraPos) {
    // 基础时间动画（缓慢自动滚动）
    baseOffset += 5.0f * deltaTime;
    
    // 基于相机位置的视差偏移
    // scrollSpeed 作为视差系数：0.0(不动) ~ 1.0(完全跟随相机)
    float cameraOffset = cameraPos.x * scrollSpeed;
    
    // 组合偏移：基础动画 + 相机视差
    currentOffset = baseOffset + cameraOffset;
    
    // 使用模运算保持在纹理宽度范围内
    if (textureWidth > 0) {
        currentOffset = std::fmod(currentOffset, textureWidth);
        if (currentOffset < 0) currentOffset += textureWidth;
    }
    
    // 更新纹理矩形的偏移（不改变精灵位置）
    if (sprite1.has_value()) {
        int offsetX = static_cast<int>(currentOffset);
        sprite1.value().setTextureRect(sf::IntRect(
            {offsetX, 0},
            {static_cast<int>(levelWidth), static_cast<int>(textureHeight)}
        ));
    }
}

void ParallaxLayer::draw() {
    // 检查窗口指针
    if (!windowPtr.has_value()) {
        printf("ParallaxLayer: windowPtr is not set\n");
        return;
    }
    
    auto window = windowPtr.value().lock();
    if (!window) {
        printf("ParallaxLayer: window is null\n");
        return;
    }
    
    auto eventSys = eventSysPtr.lock();
    if (!eventSys) {
        printf("ParallaxLayer: eventSys is null\n");
        return;
    }
    
    // 根据图层索引确定绘制优先级
    EventSys::ImmEventPriority priority;
    if (layerIndex == 0) {
        priority = EventSys::ImmEventPriority::DRAWPARALLAX_BACKGROUND;
    } else if (layerIndex <= 2) {
        priority = EventSys::ImmEventPriority::DRAWPARALLAX_FAR;
    } else if (layerIndex <= 5) {
        priority = EventSys::ImmEventPriority::DRAWPARALLAX_MID;
    } else {
        priority = EventSys::ImmEventPriority::DRAWPARALLAX_NEAR;
    }
    
    // 检查精灵是否存在
    if (!sprite1.has_value()) {
        return;
    }
    
    // 注册绘制事件（只需绘制一个精灵，纹理重复模式自动处理平铺）
    eventSys->regImmEvent(priority, [window, s1 = sprite1.value()]() {
        window->draw(s1);
    });
}
