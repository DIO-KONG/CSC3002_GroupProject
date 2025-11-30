#include "GameObj.hpp"

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
            printf("Draw event registered.\n");
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

void GraphicObj::update() {
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
            printf("Draw event registered.\n");
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
    b2Polygon groundBox = b2MakeBox(width/2, height/2);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef ();
    b2CreatePolygonShape (groundId, &groundShapeDef, &groundBox);
    // Debug
    printf("Block Box2D body created at (%.2f, %.2f) with size (%.2f, %.2f)\n", posX, posY, width, height);

    if (blockType == GRASS) {
        // 设置草地方块的物理属性
        // 草方块作为固定平台（不可破坏）
    }else if (blockType == WATER) {
        // 设置水地方块的物理属性
    }else if (blockType == ICE) {
        // 设置冰地方块的物理属性
    }else if (blockType == LAVA) {
        // 设置熔岩地方块的物理属性
    }
}

void Block::setPtrs(const std::weak_ptr<EventSys>& eventSys,
                    const std::weak_ptr<sf::RenderWindow>& window,
                    const std::weak_ptr<b2WorldId>& world) {
    eventSysPtr = eventSys;
    windowPtr.emplace(window);
    worldPtr.emplace(world);
}

void Block::update() {
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