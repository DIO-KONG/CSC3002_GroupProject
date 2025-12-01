#include "Scene.hpp"

void Scene::init(std::string sceneConfigPath, 
            std::weak_ptr<EventSys> eventSys, 
            std::weak_ptr<sf::RenderWindow> window,
            std::weak_ptr<GameInputRead> input) {
    // Debug
    printf("----------------------Initializing Scene------------------------\n");

    
    // 初始化EventSys和RenderWindow指针
    eventSysPtr = eventSys;
    windowPtr = window;
    inputPtr = input;
    this->configPath = sceneConfigPath;

    // 加载场景配置
    ResourceLoader loader(sceneConfigPath);
    // Debug
    printf("Scene config loaded from %s\n", sceneConfigPath.c_str());

    // 初始化Box2D物理世界
    worldDef = b2DefaultWorldDef();
    float gravityX = std::get<float>(loader.getResource("gravityX"));
    float gravityY = std::get<float>(loader.getResource("gravityY"));
    worldDef.gravity = {gravityX, gravityY};
    world = std::make_shared<b2WorldId>(b2CreateWorld(&worldDef));
    // Debug
    printf("Box2D World created with gravity (%.2f, %.2f)\n", gravityX, gravityY);
    printf("----------------------Adding Objects--------------------------\n");
    // 设定容器键
    std::vector<std::string> objKeys = loader.getObjKeys();
    // Debug
    for (const std::string& key : objKeys) {
        printf("Object Key Found: %s\n", key.c_str());
    }
    for (const std::string& key : objKeys) {
        // 遍历每一种对象类型
        loader.addObjKey(key);
        int objCount = loader.getObjCount(key);
        // Debug
        printf("Adding objects of type: %s, count: %d\n", key.c_str(), objCount);
        for (int i = 0; i < objCount; ++i) {
            // 遍历每个对象并添加到场景
            addObject(key, loader.getAllObjResources(i, key));
        }
    }
    // Debug
    printf("Scene initialized with %zu objects.\n", sceneAssets.size());
}

void Scene::reload() {
    // 清空对象列表
    sceneAssets.clear();
    // 销毁现有的Box2D物理世界
    b2DestroyWorld(*world);
    // 重新加载场景配置
    ResourceLoader loader(configPath);
    // 重载Box2D物理世界
    worldDef = b2DefaultWorldDef();
    float gravityX = std::get<float>(loader.getResource("gravityX"));
    float gravityY = std::get<float>(loader.getResource("gravityY"));
    worldDef.gravity = {gravityX, gravityY};
    world = std::make_shared<b2WorldId>(b2CreateWorld(&worldDef));
    // 设定容器键
    std::vector<std::string> objKeys = loader.getObjKeys();
    for (const std::string& key : objKeys) {
        // 遍历每一种对象类型
        loader.addObjKey(key);
        int objCount = loader.getObjCount(key);
        for (int i = 0; i < objCount; ++i) {
            // 遍历每个对象并添加到场景
            addObject(key, loader.getAllObjResources(i, key));
        }
    }
}

void Scene::update(const float deltaTime, const int subStepCount) {
    // 更新Box2D物理世界
    if (world) {
        // b2World_Step(*world, deltaTime, subStepCount);
        auto stepFunc = [this, deltaTime, subStepCount]() {
            b2World_Step(*world, deltaTime, subStepCount);
        };
        regImmEvent(EventSys::ImmEventPriority::BOX2D, stepFunc);
    }
    // 更新场景内所有游戏对象，注意物理模拟已经在上面完成
    for (auto& obj : sceneAssets) {
        // 将对象更新函数注册到事件系统中
        auto updateFunc = [&obj, deltaTime]() {
            obj->update(deltaTime);
        };
        regImmEvent(EventSys::ImmEventPriority::UPDATE, updateFunc);
    }
}

void Scene::render() {
    // 渲染场景内所有游戏对象
    for (auto& obj : sceneAssets) {
        // 绘制每个对象
        obj->draw();
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

void Scene::addObject(const std::string type, const ResourceLoader::ResourceDict& objConfig) {
    // 根据objConfig创建游戏对象并添加到sceneAssets
    // 分支逻辑根据objConfig中的类型信息决定创建哪种GameObj子类
    // Debug
    printf("Adding object of type: %s\n", type.c_str());
    if (type == "GraphicObj") {
        // Debug
        printf("Adding GraphicObj to Scene.\n");
        // 创建GraphicObj对象
        auto newGraphic = std::make_unique<GraphicObj>();
        // 设置GraphicObj的核心指针
        newGraphic->setPtrs(eventSysPtr, windowPtr, inputPtr);
        // 初始化GraphicObj对象
        newGraphic->initialize(objConfig);
        // 添加到场景对象列表
        sceneAssets.push_back(std::move(newGraphic));
    } else if (type == "Block") {
        // Debug
        printf("Adding Block to Scene.\n");
        // 创建Block对象
        auto newBlock = std::make_unique<Block>();
        // 设置Block的核心指针
        newBlock->setPtrs(eventSysPtr, windowPtr, world);
        // 初始化Block对象
        newBlock->initialize(objConfig);
        // 添加到场景对象列表
        sceneAssets.push_back(std::move(newBlock));
    } else if (type == "Enemy") {
        // Debug
        printf("Adding Enemy to Scene.\n");
        // 创建Enemy对象
        auto newEnemy = std::make_unique<Enemy>();
        // 设置Enemy的核心指针
        newEnemy->setPtrs(eventSysPtr, windowPtr, world, inputPtr);
        // 初始化Enemy对象
        newEnemy->initialize(objConfig);
        // 添加到场景对象列表
        sceneAssets.push_back(std::move(newEnemy));
    } else {
        // 其他类型对象的创建逻辑
        printf("Unknown object type: %s. Object not added.\n", type.c_str());
    }
}