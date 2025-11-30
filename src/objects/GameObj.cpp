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

void BaseObj::draw() {
    // 默认绘制行为，通过任务系统调度绘制事件
    // 检查类是否为可以画图的对象
    bool ifDrawable = false;
    if (features.find("drawable") != features.end()) {
        ifDrawable = features["drawable"];
    }
    if (!ifDrawable) {
        // 该对象不支持绘制操作
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

// -------------------------------- Block类实现 --------------------------------

// Block::Block() : BaseObj() {
//     // 构造函数
// }

// Block::~Block() {
//     // 析构函数
// }

// void Block::initialize() {
//     // 初始化方块对象
//     // 设置特征，例如支持绘制
//     features["drawable"] = true;

//     // 加载纹理和设置Sprite等初始化操作
//     texture = sf::Texture();
//     if (texture->loadFromFile("path_to_block_texture.png")) {
//         sprite = sf::Sprite();
//         sprite->setTexture(texture.value());
//     }
// }