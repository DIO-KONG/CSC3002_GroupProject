#pragma once
// #include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <map>

enum KeyState {
    KEY_RELEASED,
    KEY_PRESSED,
    KEY_HELD
};


class GameInputRead {
public:
    GameInputRead();
    ~GameInputRead();

    void update();
    KeyState getKeyState(const sf::Keyboard::Key key);
    

private:
    std::map<sf::Keyboard::Key, KeyState> keyStates;
    std::vector<sf::Keyboard::Key> Keys;
};
