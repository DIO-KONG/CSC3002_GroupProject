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
        // 设置跟随目标
        void setTarget(std::weak_ptr<Player> target);

    private:
        sf::View view;
        std::weak_ptr<Player> targetObj; // 相机跟随的目标对象 （通常是玩家）
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
        // 设置相机目标
        void setCameraTarget(std::weak_ptr<Player> target);
        sf::RenderWindow window;

    private:
        ConfigLoader windowLoader;
        Camera camera;
};