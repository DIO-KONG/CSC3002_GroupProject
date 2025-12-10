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
    levelCompleted_ = false;
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
    
    // 加载死亡提示用的字体
    if (deathFont.openFromFile("assets/fonts/ALGER.TTF"))
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
    // 清空子弹列表
    projectiles.clear();   
    // 清空对象列表
    sceneAssets.clear();
    levelCompleted_ = false;
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
    // 特殊处理：ParallaxLayer根据场景类型使用不同更新方式
    for (auto& obj : sceneAssets) {
        // 尝试将对象转换为ParallaxLayer
        ParallaxLayer* parallaxLayer = dynamic_cast<ParallaxLayer*>(obj.get());
        if (parallaxLayer) {
            // 如果是视差图层，根据场景类型选择更新方式
            if (useParallaxWithCamera) {
                // 关卡场景：使用相机位置更新（视差效果）
                auto updateFunc = [parallaxLayer, deltaTime, cameraPos = this->cameraPosition]() {
                    parallaxLayer->updateWithCamera(deltaTime, cameraPos);
                };
                regImmEvent(EventSys::ImmEventPriority::UPDATE, updateFunc);
            } else {
                // 菜单场景：使用普通更新（基于时间的自动滚动）
                auto updateFunc = [parallaxLayer, deltaTime]() {
                    parallaxLayer->update(deltaTime);
                };
                regImmEvent(EventSys::ImmEventPriority::UPDATE, updateFunc);
            }
        } else {
            // 普通对象正常更新
            auto updateFunc = [&obj, deltaTime]() {
                obj->update(deltaTime);
            };
            regImmEvent(EventSys::ImmEventPriority::UPDATE, updateFunc);
        }
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

    // 5) 子弹 vs 敌人 + Trap 碰撞检测
    for (auto& projPtr : projectiles) {
        if (!projPtr || !projPtr->isActive()) continue;

        sf::FloatRect pBounds = projPtr->getBounds();

        float pLeft   = pBounds.position.x;
        float pTop    = pBounds.position.y;
        float pRight  = pLeft + pBounds.size.x;
        float pBottom = pTop  + pBounds.size.y;

        bool bulletConsumed = false;

        // ① 先对子弹 vs 敌人做检测
        for (auto& obj : sceneAssets) {
            if (!obj) continue;

            auto enemy = dynamic_cast<Enemy*>(obj.get());
            if (!enemy) continue;
            if (!enemy->isAliveFlag()) continue;

            sf::FloatRect eBounds = enemy->getHitBox();

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
                bulletConsumed = true;
                break; // 一颗子弹只打中一个敌人就算了
            }
        }

        // 敌人那边已经把子弹吃掉了，就不用再检测 Trap 了
        if (bulletConsumed || !projPtr->isActive()) continue;

        // ② 子弹 vs Trap 碰撞检测
        for (auto& obj : sceneAssets) {
            if (!obj) continue;

            auto trap = dynamic_cast<Trap*>(obj.get());
            if (!trap) continue;
        if (trap->isDestroyed()) continue;

            sf::FloatRect tBounds = trap->getHitBox();

            float tLeft   = tBounds.position.x;
            float tTop    = tBounds.position.y;
            float tRight  = tLeft + tBounds.size.x;
            float tBottom = tTop  + tBounds.size.y;

            bool hit =
                (pLeft < tRight)  &&
                (pRight > tLeft)  &&
                (pTop < tBottom)  &&
                (pBottom > tTop);

            if (hit) {
                // 命中：把子弹类型和伤害传给 Trap，由 Trap 决定要不要扣血
                trap->onHitByProjectile(projPtr->getType(), projPtr->getDamage());
                projPtr->deactivate();
                break;
            }
        }
    }


    // ===== 玩家与敌人 / Trap / Block 碰撞 & 环境效果 =====
    if (playerPtr) {
        auto player = std::dynamic_pointer_cast<Player>(playerPtr);
        if (player && player->isAliveFlag()) {
            sf::FloatRect playerBounds = player->getBounds();

             // ===== 0) 掉落死亡检测 =====
            float playerTopY = playerBounds.position.y;
            if (playerTopY > fallDeathY_) {
                printf("[Scene] Player fell below death line (y = %.1f), killing player.\n",
                    playerTopY);
                // 给一个很大的伤害，复用原有死亡逻辑
                player->takeDamage(9999.0f);
            }

            // ---------- 1) 玩家 vs 敌人：直接掉血 ----------
            for (auto& obj : sceneAssets) {
                if (!obj) continue;

                auto enemy = dynamic_cast<Enemy*>(obj.get());
                if (!enemy) continue;
                if (!enemy->isAliveFlag()) continue;

                sf::FloatRect eBounds = enemy->getHitBox();

                if (rectsIntersect(playerBounds, eBounds)) {
                    float dmg = enemy->getAttackDamage();
                    printf("[Scene] Player touched enemy, damage = %.2f\n", dmg);
                    player->takeDamage(dmg);
                    // 一帧只吃一个敌人的伤害
                    break;
                }
            }

            // ---------- 2) 玩家 vs Trap：每个 Trap 只扣一次血 ----------
            for (auto& obj : sceneAssets) {
                if (!obj) continue;

                auto trap = dynamic_cast<Trap*>(obj.get());
                if (!trap) continue;

                // 未激活的陷阱：检测触发区域
                if (!trap->isActive() && trap->checkTrigger(playerBounds)) {
                    printf("[Scene] Trap triggered -> activating spike.\n");
                    trap->activate();
                }

                if (trap->isActive()) {
                    sf::FloatRect tBounds = trap->getHitBox();

                    if (rectsIntersect(playerBounds, tBounds)) {
                        if (trap->isActive()) {
    sf::FloatRect tBounds = trap->getHitBox();

    if (rectsIntersect(playerBounds, tBounds)) {
            // 如果是 GOAL：通关，不扣血 
            if (trap->isGoal()) {
                if (!levelCompleted_) {
                    levelCompleted_ = true;
                    printf("[Scene] GOAL reached! Level completed.\n");
                }
            }
            //普通 Trap：只扣一次血
            else {
                if (!trap->getHasDamagedPlayer()) {
                    float dmg = trap->getDamage();
                    printf("[Scene] Trap hit player ONCE, damage = %.2f\n", dmg);

                    player->takeDamage(dmg);
                    trap->setHasDamagedPlayer(true);
                }
            }
        }
        else {
            if (!trap->isGoal()) {
                trap->setHasDamagedPlayer(false);
            }
    } 
}

                        
                    }
                }
            }

            // ---------- 3) 玩家 vs Block：熔岩 & 冰面 ----------
            bool onLava = false;
            bool onIce  = false;

            for (auto& obj : sceneAssets) {
                if (!obj) continue;

                auto block = dynamic_cast<Block*>(obj.get());
                if (!block) continue;

                sf::FloatRect bBounds = block->getHitBox();

                // AABB 相交
                float pLeft   = playerBounds.position.x;
                float pTop    = playerBounds.position.y;
                float pRight  = pLeft + playerBounds.size.x;
                float pBottom = pTop  + playerBounds.size.y;

                float bLeft   = bBounds.position.x;
                float bTop    = bBounds.position.y;
                float bRight  = bLeft + bBounds.size.x;
                float bBottom = bTop  + bBounds.size.y;

                bool intersect =
                    (pLeft < bRight)  &&
                    (pRight > bLeft)  &&
                    (pTop < bBottom)  &&
                    (pBottom > bTop);

                if (!intersect) continue;

                if (block->isLava()) {
                    onLava = true;
                } else if (block->isIce()) {
                    onIce = true;
                }
            }

            // --- LAVA：持续掉血（环境伤害，不吃无敌时间） ---
            if (onLava) {
                const float lavaDps = 1.0f;  // 每秒掉 1 点血
                float dmgThisFrame = lavaDps * deltaTime;
                printf("[Scene] Lava damage tick: %.3f\n", dmgThisFrame);
                player->takeEnvironmentalDamage(dmgThisFrame);
            }

            // --- ICE：减速（跳跃不受影响在 Player 里控制） ---
            if (onIce) {
                // 比如：速度乘 0.4
                player->setEnvSpeedScale(0.4f);
            } else {
                // 离开冰面，恢复正常
                player->setEnvSpeedScale(1.0f);
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

        // 4. 如果玩家通关或死亡，叠加结束 UI（YOU WIN / YOU DIED）
    if (playerPtr) {
        auto player   = std::dynamic_pointer_cast<Player>(playerPtr);
        auto eventSys = eventSysPtr.lock();
        auto window   = windowPtr.lock();
        if (!eventSys || !window) {
            return;
        }

        // 4.1 通关优先：YOU WIN
        if (levelCompleted_) {
            eventSys->regImmEvent(
                EventSys::ImmEventPriority::DRAWPLAYER,
                [this, window]()
                {
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

                    // ===== 文本1：YOU WIN! =====
                    sf::Text text1(deathFont, "YOU WIN!", 60);
                    text1.setFillColor(sf::Color(50, 220, 80));

                    sf::FloatRect bounds1 = text1.getLocalBounds();
                    // SFML 3: 用 position + size
                    text1.setOrigin(bounds1.position + bounds1.size * 0.5f);
                    text1.setPosition({center.x, center.y - 40.0f});
                    window->draw(text1);

                    // ===== 文本2：Press Space to return to Menu =====
                    sf::Text text2(deathFont, "Press Space to return to Menu", 30);
                    text2.setFillColor(sf::Color::White);

                    sf::FloatRect bounds2 = text2.getLocalBounds();
                    text2.setOrigin(bounds2.position + bounds2.size * 0.5f);
                    text2.setPosition({center.x, center.y + 30.0f});
                    window->draw(text2);
                }
            );
        }
        // 4.2 没通关但玩家死亡：YOU DIED（沿用你原来的画面）
        else if (player && !player->isAliveFlag()) {
            eventSys->regImmEvent(
                EventSys::ImmEventPriority::DRAWPLAYER,
                [this, window]()
                {
                    // 调试用
                    printf("[Scene::render] Drawing YOU DIED overlay via EventSys\n");

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

                    //YOU DIED
                    sf::Text text1(deathFont, "YOU DIED", 60);
                    text1.setFillColor(sf::Color(200, 30, 30));

                    sf::FloatRect bounds1 = text1.getLocalBounds();
                    // SFML 3: 用 position + size
                    text1.setOrigin(bounds1.position + bounds1.size * 0.5f);
                    text1.setPosition({center.x, center.y - 40.0f});
                    window->draw(text1);

                    //Press R to Restart
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
    } else if (type == "ParallaxLayer") {
        // Debug
        printf("Adding ParallaxLayer to Scene.\n");
        // 创建ParallaxLayer对象
        auto newParallax = std::make_unique<ParallaxLayer>();
        // 设置ParallaxLayer的核心指针（不需要物理世界和输入）
        newParallax->setPtrs(eventSysPtr, windowPtr);
        // 初始化ParallaxLayer对象
        newParallax->initialize(objConfig);
        // 添加到场景对象列表
        sceneAssets.push_back(std::move(newParallax));
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
    } else if (type == "Trap") {
        // Debug
        printf("Adding Trap to Scene.\n");
        // 创建Trap对象
        auto newTrap = std::make_unique<Trap>();
        // 设置Trap的核心指针
        newTrap->setPtrs(eventSysPtr, windowPtr, world);
        // 初始化Trap对象
        newTrap->initialize(objConfig);
        // 添加到场景对象列表
        sceneAssets.push_back(std::move(newTrap));
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