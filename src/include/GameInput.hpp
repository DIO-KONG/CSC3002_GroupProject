#pragma once
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>



class GameInputRead {
    
public:
    enum KeyState {
        KEY_RELEASED,
        KEY_PRESSED,
        KEY_HELD
    };
    GameInputRead();
    ~GameInputRead();

    // 更新按键状态与鼠标状态
    void update();
    // 获取指定按键的状态
    KeyState getKeyState(const sf::Keyboard::Key key);
    // 设置窗口引用
    void setWindow(sf::RenderWindow* win);
    // 获取鼠标位置 参数relativeToWindow表示是否相对于窗口坐标
    sf::Vector2i getMousePosition(bool relativeToWindow = true);
    

private:
    // 存储按键状态的字典
    std::map<sf::Keyboard::Key, KeyState> keyStates;
    // 跟踪的按键列表
    std::vector<sf::Keyboard::Key> Keys;
    // 窗口引用，用于获取鼠标位置
    sf::RenderWindow* window;
    // 鼠标位置 相对+绝对
    sf::Vector2i mousePositionRelative;
    sf::Vector2i mousePositionGlobal;
};
