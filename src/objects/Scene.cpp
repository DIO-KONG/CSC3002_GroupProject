#include "Scene.hpp"
#include "Player.hpp"

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
    // 设置玩家子弹生成回调
    printf("[Scene::init] Checking playerPtr: %s\n", playerPtr ? "EXISTS" : "NULL");
    if (playerPtr) {
        printf("[Scene::init] Setting up projectile callback...\n");
        auto player = std::dynamic_pointer_cast<Player>(playerPtr);
        if (player) {
            printf("[Scene::init] Player cast successful, setting callback\n");
            player->setProjectileSpawnCallback(
                [this](const Player::ProjectileSpawnRequest& req) {
                    printf("[Scene] ===== CALLBACK TRIGGERED =====\n");
                    printf("[Scene]   Type: %s\n", req.type.c_str());
                    printf("[Scene]   Position: (%.2f, %.2f)\n", req.position.x, req.position.y);
                    printf("[Scene]   Facing: %s\n", req.facingRight ? "RIGHT" : "LEFT");
                    
                    auto proj = std::make_unique<Projectile>();
                    printf("[Scene]   Projectile created\n");
                    
                    proj->initializeDynamic(
                        Projectile::fromString(req.type),
                        req.position,
                        req.facingRight
                    );
                    printf("[Scene]   Projectile initialized\n");
                    
                    // 使用公有方法设置指针
                    proj->setWindowPtr(windowPtr);
                    proj->setEventSysPtr(eventSysPtr);
                    printf("[Scene]   Pointers set\n");
                    
                    // 你可以根据需要添加 setWorldPtr 等
                    projectiles.push_back(std::move(proj));
                    printf("[Scene]   Projectile added to list. Total: %zu\n", projectiles.size());
                } 
            );
            printf("[Scene::init] Callback set successfully in init!\n");
        }
    } else {
        printf("[Scene::init] PlayerPtr is NULL, callback will be set in setPlayerPtr()\n");
    }
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
    // 更新玩家对象
    if (playerPtr) {
        auto playerUpdateFunc = [this, deltaTime]() {
            playerPtr->update(deltaTime);
        };
        regImmEvent(EventSys::ImmEventPriority::UPDATE, playerUpdateFunc);
    }
    // 更新子弹对象
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        // 这里可以加生命周期判断，超时/碰撞就移除
        // 例如：(*it)->update(deltaTime);
        // 假设Projectile有isActive和lifetime机制
        if (*it) {
            (*it)->update(deltaTime);
            // 这里简单假设Projectile有isActive和maxLifetime
            // 你可以根据实际情况完善
            // if (!(*it)->isActive) it = projectiles.erase(it);
            // else ++it;
            ++it;
        } else {
            it = projectiles.erase(it);
        }
    }
}

void Scene::render() {
    // 渲染场景内所有游戏对象
    for (auto& obj : sceneAssets) {
        // 绘制每个对象
        obj->draw();
    }
    // 渲染玩家对象
    if (playerPtr) {
        // printf("Rendering Player Object.................\n");
        playerPtr->draw();
    }
    // 渲染子弹对象
    if (!projectiles.empty()) {
        // printf("[Scene] Rendering %zu projectiles\n", projectiles.size());
    }
    for (auto& proj : projectiles) {
        if (proj) proj->draw();
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

void Scene::setPlayerPtr(const std::shared_ptr<BaseObj>& player) {
    // printf("[Scene::setPlayerPtr] Setting player pointer\n");
    playerPtr = player;
    
    // 设置玩家子弹生成回调
    if (playerPtr) {
        // printf("[Scene::setPlayerPtr] Player pointer set, setting up callback...\n");
        auto playerCasted = std::dynamic_pointer_cast<Player>(playerPtr);
        if (playerCasted) {
            // printf("[Scene::setPlayerPtr] Player cast successful, setting callback\n");
            playerCasted->setProjectileSpawnCallback(
                [this](const Player::ProjectileSpawnRequest& req) {
                    // printf("[Scene] ===== CALLBACK TRIGGERED =====\n");
                    // printf("[Scene]   Type: %s\n", req.type.c_str());
                    // printf("[Scene]   Position: (%.2f, %.2f)\n", req.position.x, req.position.y);
                    // printf("[Scene]   Facing: %s\n", req.facingRight ? "RIGHT" : "LEFT");
                    
                    auto proj = std::make_unique<Projectile>();
                    // printf("[Scene]   Projectile created\n");
                    
                    proj->initializeDynamic(
                        Projectile::fromString(req.type),
                        req.position,
                        req.facingRight
                    );
                    // printf("[Scene]   Projectile initialized\n");
                    
                    // 使用公有方法设置指针
                    proj->setWindowPtr(windowPtr);
                    proj->setEventSysPtr(eventSysPtr);
                    // printf("[Scene]   Pointers set\n");
                    
                    projectiles.push_back(std::move(proj));
                    // printf("[Scene]   Projectile added to list. Total: %zu\n", projectiles.size());
                } 
            );
            // printf("[Scene::setPlayerPtr] Callback set successfully!\n");
        } else {
            // printf("[Scene::setPlayerPtr] ERROR: Failed to cast to Player\n");
        }
    } else {
        // printf("[Scene::setPlayerPtr] ERROR: Player pointer is null\n");
    }
}

b2WorldId Scene::getWorldId() {
    return *world;
}