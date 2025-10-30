#include "GameInput.hpp"

GameInputRead::GameInputRead()
{
    // 构造函数实现
    keyStates.clear();
    Keys = {
        sf::Keyboard::Key::W, sf::Keyboard::Key::A, sf::Keyboard::Key::S, sf::Keyboard::Key::D,
        sf::Keyboard::Key::Space, sf::Keyboard::Key::Escape
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
        KeyState& state = keyStates[key];

        if (isPressed)
        {
            if (state == KEY_RELEASED)
            {
                state = KEY_PRESSED; // 刚按下
            }
            else
            {
                state = KEY_HELD; // 持续按下
            }
        }
        else
        {
            state = KEY_RELEASED; // 未按下
        }
    }
}

KeyState GameInputRead::getKeyState(const sf::Keyboard::Key key)
{
    // 获取指定按键的状态
    if (keyStates.find(key) != keyStates.end())
    {
        return keyStates[key];
    }
    return KEY_RELEASED; // 默认返回未按下状态
}