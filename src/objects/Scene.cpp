#include "Scene.hpp"

void Scene::init(std::string sceneConfigPath, 
            std::weak_ptr<EventSys> eventSys, 
            std::weak_ptr<sf::RenderWindow> window,
            std::weak_ptr<GameInputRead> input) {
    // 初始化EventSys和RenderWindow指针
    eventSysPtr = eventSys;
    windowPtr = window;
    inputPtr = input;
    this->configPath = sceneConfigPath;
    // 加载场景配置
    ResourceLoader loader(sceneConfigPath);
    // 初始化Box2D物理世界
    worldDef = b2DefaultWorldDef();
    float gravityX = std::get<float>(loader.getResource("gravityX"));
    float gravityY = std::get<float>(loader.getResource("gravityY"));
    worldDef.gravity = {gravityX, gravityY};
    world = b2CreateWorld(&worldDef);
    // 设定容器键
    std::vector<std::string> objKeys = loader.getObjKeys();
    for (const std::string& key : objKeys) {
        // 遍历每一种对象类型
        loader.addObjKey(key);
        int objCount = loader.getObjCount(key);
        for (int i = 0; i < objCount; ++i) {
            // 遍历每个对象并添加到场景
            addObject(loader.getAllObjResources(i, key));
        }
    }
}

void Scene::reload() {
    // 清空对象列表
    sceneAssets.clear();
    // 重新加载场景配置
    ResourceLoader loader(configPath);
    // 重载Box2D物理世界
    worldDef = b2DefaultWorldDef();
    float gravityX = std::get<float>(loader.getResource("gravityX"));
    float gravityY = std::get<float>(loader.getResource("gravityY"));
    worldDef.gravity = {gravityX, gravityY};
    world = b2CreateWorld(&worldDef);
}

void Scene::update(const float deltaTime, const int subStepCount) {
    // 更新Box2D物理世界
    if (world.has_value()) {
        b2World_Step(world.value(), deltaTime, subStepCount);
    }
    // 更新场景内所有游戏对象，注意物理模拟已经在上面完成
    for (auto& objVariant : sceneAssets) {
        // 根据实际的GameObj子类进行更新
        // 示例:
        // if (auto obj = std::get_if<std::unique_ptr<GameObj>>(&objVariant)) {
        //     (*obj)->update();
        // }
    }
}

void Scene::render() {
    // 渲染场景内所有游戏对象
    for (auto& objVariant : sceneAssets) {
        // 根据实际的GameObj子类进行渲染
        // 示例:
        // if (auto obj = std::get_if<std::unique_ptr<GameObj>>(&objVariant)) {
        //     (*obj)->draw();
        // }
    }
}

void Scene::regImmEvent(const EventSys::ImmEventPriority priority, const EventSys::EventFunc& func) {
    // 注册即时事件
    if (auto eventSys = eventSysPtr.lock()) {
        eventSys->regImmEvent(priority, func);
    }
}

void Scene::regTimedEvent(const sf::Time delay, const EventSys::EventFunc& func) {
    // 注册定时事件
    if (auto eventSys = eventSysPtr.lock()) {
        eventSys->regTimedEvent(delay, func);
    }
}

void Scene::addObject(const ResourceLoader::ResourceDict& objConfig) {
    // 根据objConfig创建游戏对象并添加到sceneAssets
    // 分支逻辑根据objConfig中的类型信息决定创建哪种GameObj子类
}