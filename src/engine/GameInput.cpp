#include "GameInput.hpp"

GameInputRead::GameInputRead()
{
    // 构造函数实现
    keyStates.clear();
    Keys = {
        sf::Keyboard::Key::W, sf::Keyboard::Key::A, sf::Keyboard::Key::S, sf::Keyboard::Key::D,
        sf::Keyboard::Key::Space, sf::Keyboard::Key::Escape, sf::Keyboard::Key::R,
        sf::Keyboard::Key::J, sf::Keyboard::Key::K
        // 可以根据需要添加更多按键
    };
}

GameInputRead::~GameInputRead()
{
    // 析构函数实现
    keyStates.clear();
    Keys.clear();
}

void GameInputRead::update()
{
    // 更新按键状态
    for (const sf::Keyboard::Key& key : Keys)
    {
        bool isPressed = sf::Keyboard::isKeyPressed(key);
        KeyState& state = GameInputRead::keyStates[key];

        if (isPressed)
        {
            if (state == GameInputRead::KEY_RELEASED)
            {
                state = GameInputRead::KEY_PRESSED; // 刚按下
            }
            else
            {
                state = GameInputRead::KEY_HELD; // 持续按下
            }
        }
        else
        {
            state = GameInputRead::KEY_RELEASED; // 未按下
        }
    }
    // 更新鼠标位置
    mousePositionGlobal = sf::Mouse::getPosition();
    if (window)
    {
        mousePositionRelative = sf::Mouse::getPosition(*window);
    }
}

GameInputRead::KeyState GameInputRead::getKeyState(const sf::Keyboard::Key key)
{
    // 获取指定按键的状态
    if (keyStates.find(key) != keyStates.end())
    {
        return keyStates[key];
    }
    return GameInputRead::KEY_RELEASED; // 默认返回未按下状态
}

void GameInputRead::setWindow(sf::RenderWindow* win)
{
    // 设置窗口引用
    window = win;
}

sf::Vector2i GameInputRead::getMousePosition(bool relativeToWindow)
{
    // 获取鼠标位置 参数relativeToWindow表示是否相对于窗口坐标
    if (relativeToWindow && window)
    {
        return mousePositionRelative;
    }
    return mousePositionGlobal;
}