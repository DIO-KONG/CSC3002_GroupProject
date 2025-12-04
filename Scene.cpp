#include "Scene.hpp"
#include "Player.hpp"
#include <SFML/Graphics/Rect.hpp>

static bool rectsIntersect(const sf::FloatRect& a, const sf::FloatRect& b)
{
    // SFML 3: FloatRect 使用 position / size
    float ax1 = a.position.x;
    float ay1 = a.position.y;
    float ax2 = ax1 + a.size.x;
    float ay2 = ay1 + a.size.y;

    float bx1 = b.position.x;
    float by1 = b.position.y;
    float bx2 = bx1 + b.size.x;
    float by2 = by1 + b.size.y;

    // AABB 矩形相交判断
    return (ax1 < bx2) &&
           (ax2 > bx1) &&
           (ay1 < by2) &&
           (ay2 > by1);
}

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
    
    // 加载死亡提示用的字体（路径按你实际项目改）
    if (deathFont.openFromFile("C:/Users/Frank Xie/Desktop/CSC3002_GroupProject-master/assets/fonts/ALGER.TTF"))
    {
        deathFontLoaded = true;
        printf("[Scene] Death UI font loaded.\n");
    }
    else
    {
        deathFontLoaded = false;
        printf("[Scene] WARNING: failed to load death UI font.\n");
    }

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
    // 1) 更新 Box2D 物理世界
    if (world) {
        auto stepFunc = [this, deltaTime, subStepCount]() {
            b2World_Step(*world, deltaTime, subStepCount);
        };
        regImmEvent(EventSys::ImmEventPriority::BOX2D, stepFunc);
    }

    // 2) 更新场景内所有静态/普通游戏对象（Block、Enemy 等）
    for (auto& obj : sceneAssets) {
        auto updateFunc = [&obj, deltaTime]() {
            obj->update(deltaTime);
        };
        regImmEvent(EventSys::ImmEventPriority::UPDATE, updateFunc);
    }

    // 3) 更新玩家对象
    if (playerPtr) {
        auto playerUpdateFunc = [this, deltaTime]() {
            playerPtr->update(deltaTime);
        };
        regImmEvent(EventSys::ImmEventPriority::UPDATE, playerUpdateFunc);
    }

    // 4) 更新子弹对象（直接更新，不走事件系统）
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        if (*it) {
            (*it)->update(deltaTime);
            // 如果子弹已经失效（比如时间到了），直接删掉
            if (!(*it)->isActive()) {
                it = projectiles.erase(it);
            } else {
                ++it;
            }
        } else {
            it = projectiles.erase(it);
        }
    }

    // 5) 子弹 vs 敌人 碰撞检测
    for (auto& projPtr : projectiles) {
    if (!projPtr || !projPtr->isActive()) continue;

    sf::FloatRect pBounds = projPtr->getBounds();

    for (auto& obj : sceneAssets) {
        if (!obj) continue;

        // 只对 Enemy 做检测
        auto enemy = dynamic_cast<Enemy*>(obj.get());
        if (!enemy) continue;
        if (!enemy->isAliveFlag()) continue;

        sf::FloatRect eBounds = enemy->getHitBox();

        // 矩形相交判断
        float pLeft   = pBounds.position.x;
        float pTop    = pBounds.position.y;
        float pRight  = pLeft + pBounds.size.x;
        float pBottom = pTop  + pBounds.size.y;

        float eLeft   = eBounds.position.x;
        float eTop    = eBounds.position.y;
        float eRight  = eLeft + eBounds.size.x;
        float eBottom = eTop  + eBounds.size.y;

        bool hit =
            (pLeft < eRight)  &&
            (pRight > eLeft)  &&
            (pTop < eBottom)  &&
            (pBottom > eTop);

        if (hit) {
            // 命中：敌人扣血，子弹消失
            enemy->onhit(projPtr->getDamage());
            projPtr->deactivate();
            break; // 一颗子弹只打中一个敌人就算了
        }
    }
}

// ===== 玩家与敌人碰撞：玩家掉血 =====
    if (playerPtr) {
        auto player = std::dynamic_pointer_cast<Player>(playerPtr);
        if (player && player->isAliveFlag()) {
            sf::FloatRect playerBounds = player->getBounds();

            for (auto& obj : sceneAssets) {
                if (!obj) continue;

                auto enemy = dynamic_cast<Enemy*>(obj.get());
                if (!enemy) continue;
                if (!enemy->isAliveFlag()) continue;

                sf::FloatRect eBounds = enemy->getHitBox();

                if (rectsIntersect(playerBounds, eBounds)) {
                    // 碰到敌人：按敌人攻击力扣血
                    float dmg = enemy->getAttackDamage();
                    printf("[Scene] Player touched enemy, damage = %.2f\n", dmg);
                    player->takeDamage(dmg);

                    // 防止同一帧被多个敌人连打，这里撞到一个就 break
                    break;
                }
            }
        }
    }


    // 6) 再清理一次已经失效的子弹
    projectiles.remove_if([](const std::unique_ptr<Projectile>& p) {
        return !p || !p->isActive();
    });
}

    

void Scene::render() {
    printf("[Scene::render] called\n");
    if (!playerPtr) {
    printf("[Scene::render] playerPtr is NULL!\n");
}
if (playerPtr) {
    auto p = std::dynamic_pointer_cast<Player>(playerPtr);
    if (p) {
        printf("[Scene::render] player alive? %d\n", p->isAlive());
    }
}

    // 1. 先画场景里的物体
    for (auto& obj : sceneAssets) {
        if (obj) obj->draw();
    }

    // 2. 画玩家
    if (playerPtr) {
        playerPtr->draw();
    }

    // 3. 画子弹
    for (auto& proj : projectiles) {
        if (proj) proj->draw();
    }

    // 4. 如果玩家死亡，叠加 YOU DIED UI
    if (playerPtr) {
        auto player = std::dynamic_pointer_cast<Player>(playerPtr);
        if (player && !player->isAlive()) {

            auto eventSys = eventSysPtr.lock();
            auto window   = windowPtr.lock();
            if (!eventSys || !window) {
                return;
            }

            eventSys->regImmEvent(
                EventSys::ImmEventPriority::DRAWPLAYER,   // 或者 UI / HUD（看你们枚举里有没有）
                [this, window]()
                {
                    // 调试用
                    // printf("[Scene::render] Drawing YOU DIED overlay via EventSys\n");

                    // 拿当前视口
                    sf::View view        = window->getView();
                    sf::Vector2f size    = view.getSize();
                    sf::Vector2f center  = view.getCenter();
                    sf::Vector2f topLeft = center - size * 0.5f;

                    // 半透明黑底
                    sf::RectangleShape bg;
                    bg.setSize(size);
                    bg.setFillColor(sf::Color(0, 0, 0, 180));
                    bg.setPosition(topLeft);
                    window->draw(bg);

                    // ===== 文本1：YOU DIED =====
                    sf::Text text1(deathFont, "YOU DIED", 60);
                    text1.setFillColor(sf::Color(200, 30, 30));

                    sf::FloatRect bounds1 = text1.getLocalBounds();
                    // SFML 3: 用 position + size
                    text1.setOrigin(bounds1.position + bounds1.size * 0.5f);
                    text1.setPosition({center.x, center.y - 40.0f});
                    window->draw(text1);

                    // ===== 文本2：Press R to Restart =====
                    sf::Text text2(deathFont, "Press R to Restart", 30);
                    text2.setFillColor(sf::Color::White);

                    sf::FloatRect bounds2 = text2.getLocalBounds();
                    text2.setOrigin(bounds2.position + bounds2.size * 0.5f);
                    text2.setPosition({center.x, center.y + 30.0f});
                    window->draw(text2);
                }
            );
        }
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