#include "EventSys.hpp"

int main()
{
    EventSys eventSys;

    // 注册一个即时事件
    auto printEvent = []() {
        std::cout << "Immediate Event Triggered!" << std::endl;
    };
    eventSys.regImmEvent(EventSys::ImmEventPriority::UPDATE, printEvent);

    // 注册一个定时事件，延迟2秒执行
    auto timedEvent = []() {
        std::cout << "Timed Event Triggered after 2 seconds!" << std::endl;
    };
    sf::Time delay = sf::Time(sf::milliseconds(2000));
    eventSys.regTimedEvent(delay, timedEvent); // 2000毫秒后执行

    // 模拟主循环
    sf::Clock clock;
    while (true)
    {
        // 执行即时事件
        eventSys.executeImmEvents();

        // 执行定时事件
        eventSys.executeTimedEvents();

        // 退出条件（例如运行5秒后退出）
        if (clock.getElapsedTime().asSeconds() > 5.0f)
            break;

        // 小睡一会儿以避免忙等待
        sf::sleep(sf::milliseconds(100));
    }

    return 0;
}