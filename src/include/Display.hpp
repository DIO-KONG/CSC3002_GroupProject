#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "ConfigLoader.hpp"

// 前向声明
class Player;

class Camera
{
    public:
        Camera();
        ~Camera();
        void init(sf::Vector2f center = {960.f, 540.f}, sf::Vector2f size = {1920.f, 1080.f});
        // 获取视图
        sf::View& getView();
        // 更新
        void update();
        // 设置视图中心
        void setCenter(sf::Vector2f center);
        // 设置视图大小
        void setSize(sf::Vector2f size);
        // 跟随目标点（通过引用传入Player类进行同步）
        void updateFollowPoint(sf::Vector2f point)
        {
            followPoint = point;
        }
        
    private:
        sf::View view;
        std::weak_ptr<Player> targetObj; // 相机跟随的目标对象 （通常是玩家）
        // 摄像机跟随差值（半个屏幕）
        float minX = 960.f;
        float maxX = 3500.f;
        float followoffsetX = 108.f;
        sf::Vector2f followPoint;
};

class Display
{
    public:
        Display();
        ~Display();
        // 显示窗口
        void display();
        // 更新窗口内容
        void update();
        // 清空窗口内容
        void clear();
        sf::RenderWindow window;
        Camera camera;

    private:
        ConfigLoader windowLoader;
};