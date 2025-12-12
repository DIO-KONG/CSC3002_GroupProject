#pragma once

// 继承BaseObj基类，与GraphicObj、Block、Enemy等保持一致
#include "GameObj.hpp"
#include <SFML/Audio.hpp>       // SFML音频库
#include <unordered_map>        // 哈希映射容器
#include <string>               // 字符串类型
#include <memory>               // 智能指针
#include <vector>               // 向量容器

// 音频管理器类，继承BaseObj，由Scene统一创建和管理
class AudioManager : public BaseObj {
public:
    // 音频状态枚举，用于跟踪当前音频播放状态
    enum class AudioState {
        STOPPED,    // 已停止状态
        PLAYING,    // 正在播放状态
        PAUSED      // 已暂停状态
    };
    
    // 构造函数
    AudioManager();
    
    // 析构函数，清理音频资源
    ~AudioManager() override;
    
    // ========== 必须覆盖的BaseObj虚函数 ==========
    // 初始化函数（无参数版本），BaseObj要求必须实现
    void initialize() override;
    
    // 更新函数，BaseObj要求必须实现
    void update(float deltaTime) override;
    
    // 绘制函数，BaseObj要求必须实现（音频管理器不需要绘制，但必须实现）
    void draw() override;
    
    // ========== 设置指针方法（与GraphicObj、Block、Enemy保持一致） ==========
    void setPtrs(const std::weak_ptr<EventSys>& eventSys,
                 const std::weak_ptr<sf::RenderWindow>& window,
                 const std::weak_ptr<GameInputRead>& input);
    
    // ========== 音频控制接口（只保留已实现的） ==========
    // 播放指定名称的音乐
    void playMusic(const std::string& musicName);
    
    // 停止当前播放的音乐
    void stopMusic();
    
    // 暂停当前播放的音乐
    void pauseMusic();
    
    // 恢复暂停的音乐
    void resumeMusic();
    
    // 设置主音量（影响所有音频）
    void setMasterVolume(float volume);
    
    // 设置音乐音量（相对主音量）
    void setMusicVolume(float volume);
    
    // 获取当前音乐状态
    AudioState getMusicState() const;
    
    // 获取当前音乐的总时长（秒）
    float getMusicDuration() const;
    
    // 获取当前播放位置（秒）
    float getPlayingOffset() const;
    
    // 响应玩家事件（如受伤、死亡等）
    void onPlayerEvent(const std::string& eventType);
    
    // 响应场景事件（如切换场景等）
    void onSceneEvent(const std::string& eventType);
    
private:
    // ========== 私有成员变量 ==========
    std::unordered_map<std::string, std::string> m_musicFiles;        // 音乐名称到文件路径的映射
    
    std::unique_ptr<sf::Music> m_currentMusic;                        // 当前播放的音乐对象
    
    AudioState m_musicState = AudioState::STOPPED;                    // 当前音乐状态
    std::string m_currentMusicName;                                   // 当前播放的音乐名称
    
    float m_masterVolume = 80.0f;                                     // 主音量（0.0-100.0）
    float m_musicVolume = 70.0f;                                      // 音乐音量（0.0-100.0）
    
    std::string m_configPath;                                         // 音频配置文件路径
    
    // ========== 私有方法 ==========
    // 加载音频配置文件
    void loadAudioConfig(const std::string& configPath);
    
    // 清理音频资源
    void cleanupAudioResources();
    
    // 注册事件监听器，用于响应游戏事件
    void registerEventListeners();
};