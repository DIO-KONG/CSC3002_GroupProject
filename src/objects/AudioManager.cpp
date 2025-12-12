// 包含必要的头文件
#include "../include/AudioManager.hpp"
#include <iostream>       // 控制台输出
#include <fstream>        // 文件操作
#include <nlohmann/json.hpp>  // JSON解析库
#include <algorithm>  // for std::clamp

// 使用nlohmann::json命名空间
using json = nlohmann::json;

// ================= 构造函数 =================
AudioManager::AudioManager() {
    // 音频管理器不参与绘制，但需要标记特征供Scene管理
    features["drawable"] = false;  // 不需要绘制
    features["audio"] = true;      // 标记为音频对象
}

// ================= 析构函数 =================
AudioManager::~AudioManager() {
    // 直接清理音频资源，不在析构函数中注册事件
    cleanupAudioResources();
}

// ================= initialize方法 =================
// 注意：BaseObj基类只有无参数的initialize()函数，所以我们只实现这个版本
// 移除了带参数的initialize()函数，因为BaseObj没有定义该函数
void AudioManager::initialize() {
    // 输出初始化日志
    std::cout << "[AudioManager] Initializing..." << std::endl;
    
    // 设置默认音乐文件映射（直接硬编码）
    m_musicFiles["menu"] = "audio/menu.WAV";
    m_musicFiles["level1"] = "audio/level1_music.ogg";  // 如果有的话
    m_musicFiles["victory"] = "audio/victory.ogg";      // 如果有的话
    m_musicFiles["gameover"] = "audio/gameover.ogg";    // 如果有的话
    
    // 设置音量
    setMasterVolume(80.0f);
    setMusicVolume(70.0f);
    
    // 注册事件监听器，用于响应游戏事件
    registerEventListeners();
    
    // 初始化完成日志
    std::cout << "[AudioManager] Initialized successfully." << std::endl;
}

// ================= 设置指针方法 =================
// 注意：必须使用emplace()来初始化optional<weak_ptr>，不能直接赋值
// 这与GraphicObj、Block、Enemy等类的实现保持一致
void AudioManager::setPtrs(const std::weak_ptr<EventSys>& eventSys,
                          const std::weak_ptr<sf::RenderWindow>& window,
                          const std::weak_ptr<GameInputRead>& input) {
    // 保存事件系统弱指针，用于注册事件
    eventSysPtr = eventSys;
    
    // 保存窗口弱指针（虽然音频不需要绘制，但保持接口一致）
    // 注意：windowPtr是optional<weak_ptr>类型，必须使用emplace
    windowPtr.emplace(window);
    
    // 保存输入系统弱指针，用于音量控制
    // 注意：inputPtr是optional<weak_ptr>类型，必须使用emplace
    inputPtr.emplace(input);
}

// ================= update方法 =================
void AudioManager::update(float deltaTime) {
    // 音频管理器不需要每帧更新，但保留接口用于可能的音频过渡效果
    // 例如：音频淡入淡出、动态音量调整等
    
    // 使用deltaTime进行音频过渡处理（当前为空实现）
    (void)deltaTime; // 避免未使用参数警告
}

// ================= draw方法 =================
// 音频管理器不需要绘制，但BaseObj要求必须实现draw()函数
// 这里提供一个空的实现以满足接口要求
void AudioManager::draw() {
    // 音频管理器不需要绘制，此方法为空实现
}

// ================= 加载音频配置 =================
void AudioManager::loadAudioConfig(const std::string& configPath) {
    // 打开配置文件
    std::ifstream file(configPath);
    if (!file.is_open()) {
        // 文件打开失败，输出错误信息
        std::cerr << "[AudioManager] Failed to load audio config: " << configPath << std::endl;
        return;  // 提前返回
    }
    
    try {
        // 解析JSON文件
        json config;
        file >> config;
        file.close();  // 关闭文件
        
        // 读取主音量设置
        if (config.contains("master_volume")) {
            m_masterVolume = config["master_volume"].get<float>();
        }
        
        // 读取音乐音量设置
        if (config.contains("music_volume")) {
            m_musicVolume = config["music_volume"].get<float>();
        }
        
        // 读取音乐文件映射
        if (config.contains("music") && config["music"].is_object()) {
            // 遍历music对象中的所有键值对
            for (auto& [key, value] : config["music"].items()) {
                if (value.is_string()) {
                    // 保存音乐名称到文件路径的映射
                    m_musicFiles[key] = value.get<std::string>();
                    // 输出加载日志
                    std::cout << "[AudioManager] Loaded music: " << key 
                              << " -> " << m_musicFiles[key] << std::endl;
                }
            }
        }
        
        // 检查默认音乐设置
        if (config.contains("default_music") && config["default_music"].is_string()) {
            std::string defaultMusic = config["default_music"].get<std::string>();
            if (m_musicFiles.find(defaultMusic) != m_musicFiles.end()) {
                // 默认音乐有效，播放默认音乐
                playMusic(defaultMusic);
                std::cout << "[AudioManager] Default music set to: " << defaultMusic << std::endl;
            }
        }
        
    } catch (const json::exception& e) {
        // JSON解析错误，输出错误信息
        std::cerr << "[AudioManager] JSON parsing error: " << e.what() << std::endl;
    }
}

// ================= 播放音乐 =================
void AudioManager::playMusic(const std::string& musicName) {
    // 检查请求的音乐是否存在
    if (m_musicFiles.find(musicName) == m_musicFiles.end()) {
        std::cerr << "[AudioManager] Music not found: " << musicName << std::endl;
        return;  // 音乐不存在，提前返回
    }
    
    // 获取音乐文件路径
    const std::string& filePath = m_musicFiles[musicName];
    
    // 停止当前播放的音乐（如果有）
    if (m_currentMusic) {
        m_currentMusic->stop();
    }
    
    // 创建新的音乐对象
    m_currentMusic = std::make_unique<sf::Music>();
    
    // 尝试打开音乐文件
    if (!m_currentMusic->openFromFile(filePath)) {
        std::cerr << "[AudioManager] Failed to load music file: " << filePath << std::endl;
        m_currentMusic.reset();  // 重置智能指针
        return;  // 文件加载失败，提前返回
    }
    
    // 计算最终音量（主音量 * 音乐相对音量）
    float finalVolume = m_musicVolume * (m_masterVolume / 100.0f);
    
    // 设置音乐属性
    m_currentMusic->setVolume(finalVolume);  // 设置音量
    m_currentMusic->setLooping(true);           // 设置循环播放
    
    // 开始播放音乐
    m_currentMusic->play();
    m_musicState = AudioState::PLAYING;  // 更新状态
    m_currentMusicName = musicName;      // 保存音乐名称
    
    // 输出播放日志
    std::cout << "[AudioManager] Playing music: " << musicName 
              << " (volume: " << finalVolume << ")" << std::endl;
}

// ================= 停止音乐 =================
void AudioManager::stopMusic() {
    // 检查当前是否有音乐正在播放
    if (m_currentMusic && m_musicState == AudioState::PLAYING) {
        m_currentMusic->stop();                 // 停止播放
        m_musicState = AudioState::STOPPED;    // 更新状态
        std::cout << "[AudioManager] Music stopped." << std::endl;
    }
}

// ================= 暂停音乐 =================
void AudioManager::pauseMusic() {
    // 检查当前是否有音乐正在播放
    if (m_currentMusic && m_musicState == AudioState::PLAYING) {
        m_currentMusic->pause();                // 暂停播放
        m_musicState = AudioState::PAUSED;     // 更新状态
        std::cout << "[AudioManager] Music paused." << std::endl;
    }
}

// ================= 恢复音乐 =================
void AudioManager::resumeMusic() {
    // 检查当前是否有音乐处于暂停状态
    if (m_currentMusic && m_musicState == AudioState::PAUSED) {
        m_currentMusic->play();                 // 继续播放
        m_musicState = AudioState::PLAYING;    // 更新状态
        std::cout << "[AudioManager] Music resumed." << std::endl;
    }
}

// ================= 设置主音量 =================
void AudioManager::setMasterVolume(float volume) {
    // 限制音量在0.0-100.0范围内
    m_masterVolume = std::clamp(volume, 0.0f, 100.0f);
    
    // 更新当前音乐的体积（如果有）
    if (m_currentMusic) {
        // 重新计算最终音量
        float finalVolume = m_musicVolume * (m_masterVolume / 100.0f);
        m_currentMusic->setVolume(finalVolume);  // 应用新音量
    }
    
    // 输出音量设置日志
    std::cout << "[AudioManager] Master volume set to: " << m_masterVolume << std::endl;
}

// ================= 设置音乐音量 =================
void AudioManager::setMusicVolume(float volume) {
    // 限制音量在0.0-100.0范围内
    m_musicVolume = std::clamp(volume, 0.0f, 100.0f);
    
    // 更新当前音乐的体积（如果有）
    if (m_currentMusic) {
        // 重新计算最终音量
        float finalVolume = m_musicVolume * (m_masterVolume / 100.0f);
        m_currentMusic->setVolume(finalVolume);  // 应用新音量
    }
    
    // 输出音量设置日志
    std::cout << "[AudioManager] Music volume set to: " << m_musicVolume << std::endl;
}

// ================= 获取音乐状态 =================
AudioManager::AudioState AudioManager::getMusicState() const {
    return m_musicState;  // 返回当前状态
}

// ================= 获取音乐时长 =================
float AudioManager::getMusicDuration() const {
    if (m_currentMusic) {
        // 获取音乐总时长（秒）
        return m_currentMusic->getDuration().asSeconds();
    }
    return 0.0f;  // 未加载音乐，返回0
}

// ================= 获取播放位置 =================
float AudioManager::getPlayingOffset() const {
    if (m_currentMusic) {
        // 获取当前播放位置（秒）
        return m_currentMusic->getPlayingOffset().asSeconds();
    }
    return 0.0f;  // 未播放音乐，返回0
}

// ================= 响应玩家事件 =================
void AudioManager::onPlayerEvent(const std::string& eventType) {
    // 输出事件日志
    std::cout << "[AudioManager] Player event: " << eventType << std::endl;
    
    // 根据事件类型执行不同操作
    if (eventType == "player_death") {
        // 玩家死亡时停止音乐
        stopMusic();
    } else if (eventType == "player_hurt") {
        // 玩家受伤时，短暂降低音量制造效果
        if (m_currentMusic) {
            // 保存原始音量
            float originalVolume = m_currentMusic->getVolume();
            
            // 降低音量到50%
            m_currentMusic->setVolume(originalVolume * 0.5f);
            
            // 通过EventSys注册恢复音量的定时事件
            if (auto eventSys = eventSysPtr.lock()) {
                auto restoreFunc = [this, originalVolume]() {
                    // 0.5秒后恢复原始音量
                    if (m_currentMusic) {
                        m_currentMusic->setVolume(originalVolume);
                    }
                };
                // 注册0.5秒后的定时事件
                eventSys->regTimedEvent(sf::seconds(0.5f), restoreFunc);
            }
        }
    }
    // 可以在这里添加更多玩家事件的处理
}

// ================= 响应场景事件 =================
void AudioManager::onSceneEvent(const std::string& eventType) {
    // 输出事件日志
    std::cout << "[AudioManager] Scene event: " << eventType << std::endl;
    
    // 根据场景事件类型切换音乐
    if (eventType == "scene_menu") {
        playMusic("menu");      // 播放菜单音乐
    } else if (eventType == "scene_level1") {
        playMusic("level1");    // 播放关卡1音乐
    } else if (eventType == "scene_victory") {
        playMusic("victory");   // 播放胜利音乐
    } else if (eventType == "scene_gameover") {
        playMusic("gameover");  // 播放失败音乐
    }
}  // 添加这个右大括号！！！

// ================= 注册事件监听器 =================
void AudioManager::registerEventListeners() {
    // 获取事件系统指针
    auto eventSys = eventSysPtr.lock();
    if (!eventSys) {
        return;  // 事件系统不可用，提前返回
    }
    
    // 获取输入系统指针
    if (!inputPtr.has_value()) {
        return;  // 输入系统未设置，提前返回
    }
    
    // 创建音频控制函数lambda
    auto audioControlFunc = [this]() {
        auto input = inputPtr.value().lock();  // 注意：inputPtr是optional<weak_ptr>，需要先获取value()
        if (!input) {
            return;  // 输入系统不可用，提前返回
        }
        
        // 检查M键是否按下（暂停/继续音乐）
        if (input->getKeyState(sf::Keyboard::Key::M) == GameInputRead::KEY_PRESSED) {
            if (m_musicState == AudioState::PLAYING) {
                pauseMusic();  // 暂停播放
            } else if (m_musicState == AudioState::PAUSED) {
                resumeMusic(); // 继续播放
            }
        }
        
        // 检查加号键是否按下（增加音量）
        if (input->getKeyState(sf::Keyboard::Key::Add) == GameInputRead::KEY_PRESSED) {
            setMasterVolume(m_masterVolume + 10.0f);  // 增加10单位音量
        }
        
        // 检查减号键是否按下（减少音量）
        if (input->getKeyState(sf::Keyboard::Key::Subtract) == GameInputRead::KEY_PRESSED) {
            setMasterVolume(m_masterVolume - 10.0f);  // 减少10单位音量
        }
    };
    
    // 注册音频控制事件，在POST_UPDATE阶段执行
    eventSys->regImmEvent(EventSys::ImmEventPriority::POST_UPDATE, audioControlFunc);
}
 

// ================= 清理音频资源 =================
void AudioManager::cleanupAudioResources() {
    // 输出清理日志
    std::cout << "[AudioManager] Cleaning up audio resources..." << std::endl;
    
    // 停止并释放当前音乐
    if (m_currentMusic) {
        m_currentMusic->stop();  // 停止播放
        m_currentMusic.reset();  // 释放资源
    }
    
    // 清空音乐文件映射
    m_musicFiles.clear();
    
    // 重置状态变量
    m_musicState = AudioState::STOPPED;
    m_currentMusicName.clear();
    
    // 输出清理完成日志
    std::cout << "[AudioManager] Audio resources cleaned up." << std::endl;
}