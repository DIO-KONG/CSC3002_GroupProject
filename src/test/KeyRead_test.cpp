#include "GameInput.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>

int main() {
    // 创建窗口 - SFML3 风格
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "KeyRead Test");
    window.setFramerateLimit(60);
    
    // 创建键盘读取器
    GameInputRead keyReader;
    
    // 创建字体
    sf::Font font;
    // 尝试加载字体，但不依赖它
    bool fontLoaded = font.openFromFile("arial.ttf");
    
    // 定义要测试的按键 - 使用 SFML3 的 Key:: 枚举
    std::vector<std::pair<sf::Keyboard::Key, std::string>> testKeys = {
        {sf::Keyboard::Key::W, "W"},
        {sf::Keyboard::Key::A, "A"},
        {sf::Keyboard::Key::S, "S"},
        {sf::Keyboard::Key::D, "D"},
        {sf::Keyboard::Key::Space, "SPACE"},
        {sf::Keyboard::Key::Escape, "ESC"}
    };
    
    // 创建显示矩形
    std::vector<sf::RectangleShape> keyRects(testKeys.size());
    // SFML3 需要字体来创建 Text，所以延迟创建
    std::vector<sf::Text> keyTexts;
    
    // 设置矩形和文本位置
    float rectWidth = 100.0f;
    float rectHeight = 60.0f;
    float spacing = 20.0f;
    float startX = 50.0f;
    float startY = 100.0f;
    
    for (size_t i = 0; i < testKeys.size(); ++i) {
        // 设置矩形
        keyRects[i].setSize(sf::Vector2f(rectWidth, rectHeight));
        keyRects[i].setPosition(sf::Vector2f(startX + i * (rectWidth + spacing), startY));
        keyRects[i].setOutlineThickness(2.0f);
        keyRects[i].setOutlineColor(sf::Color::Black);
        
        // 创建文本 - SFML3 需要字体参数
        if (fontLoaded) {
            keyTexts.emplace_back(font, testKeys[i].second, 16);
            keyTexts[i].setFillColor(sf::Color::Black);
            
            // 居中文本
            sf::FloatRect textBounds = keyTexts[i].getLocalBounds();
            keyTexts[i].setPosition(sf::Vector2f(
                startX + i * (rectWidth + spacing) + (rectWidth - textBounds.size.x) / 2,
                startY + (rectHeight - textBounds.size.y) / 2 - 5
            ));
        } else {
            // 如果没有字体，创建一个虚拟字体的 Text 对象
            sf::Font dummyFont;
            keyTexts.emplace_back(dummyFont, "", 16);
        }
    }
    
    // 创建说明文本 - SFML3 风格，使用可选类型
    std::optional<sf::Text> instructionText;
    if (fontLoaded) {
        instructionText = sf::Text(font, "KeyRead Test - Press W/A/S/D/SPACE/ESC\n"
                                        "White: Released, Gray: Pressed, Green: Held", 20);
        instructionText->setFillColor(sf::Color::White);
        instructionText->setPosition(sf::Vector2f(50, 20));
    }
    
    // 主循环
    while (window.isOpen()) {
        // 处理事件 - SFML3 风格
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        
        // 更新键盘状态
        keyReader.update();
        
        // 更新显示颜色
        for (size_t i = 0; i < testKeys.size(); ++i) {
            KeyState state = keyReader.getKeyState(testKeys[i].first);
            
            switch (state) {
                case KEY_RELEASED:
                    keyRects[i].setFillColor(sf::Color::White);
                    break;
                case KEY_PRESSED:
                    keyRects[i].setFillColor(sf::Color(128, 128, 128)); // 灰色
                    break;
                case KEY_HELD:
                    keyRects[i].setFillColor(sf::Color::Green);
                    break;
            }
        }
        
        // 控制台输出（可选）
        static int frameCounter = 0;
        if (frameCounter % 60 == 0) { // 每秒输出一次
            std::cout << "\n=== Frame " << frameCounter << " ===" << std::endl;
            for (size_t i = 0; i < testKeys.size(); ++i) {
                KeyState state = keyReader.getKeyState(testKeys[i].first);
                std::string stateStr;
                switch (state) {
                    case KEY_RELEASED: stateStr = "RELEASED"; break;
                    case KEY_PRESSED: stateStr = "PRESSED"; break;
                    case KEY_HELD: stateStr = "HELD"; break;
                }
                std::cout << testKeys[i].second << ": " << stateStr << std::endl;
            }
        }
        frameCounter++;
        
        // 渲染
        window.clear(sf::Color::Black);
        
        // 绘制说明文字（如果有字体）
        if (instructionText) {
            window.draw(*instructionText);
        }
        
        // 绘制按键状态
        for (size_t i = 0; i < testKeys.size(); ++i) {
            window.draw(keyRects[i]);
            if (fontLoaded && i < keyTexts.size()) {
                window.draw(keyTexts[i]);
            }
        }
        
        window.display();
    }
    
    return 0;
}