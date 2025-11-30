#include "EventSys.hpp"

EventSys::EventSys()
{
    // EventSys类的构造函数实现
    eventSysClock.restart();
}

EventSys::~EventSys()
{
    // EventSys类的析构函数实现
}

void EventSys::regImmEvent(const ImmEventPriority eventType, const EventFunc& func)
{
    // 注册即时事件的实现
    ImmEvent newEvent{eventType, func};
    immEventQueue.push(newEvent);
}

void EventSys::regTimedEvent(const sf::Time delay, const EventFunc& func)
{
    // 注册定时事件的实现
    sf::Time triggerTime = eventSysClock.getElapsedTime() + delay;
    TimedEvent newEvent{triggerTime, func};
    timedEventQueue.push(newEvent);
}

void EventSys::executeImmEvents()
{
    // 执行即时事件的实现
    while (!immEventQueue.empty())
    {
        ImmEvent currentEvent = immEventQueue.top();
        try
        {
            currentEvent.func();
            printf("Immediate event executed. Event Type: %d\n", static_cast<int>(currentEvent.priority));
        }
        catch (const std::exception& e)
        {
            // 处理异常：输出日志
            std::cerr << "Error occurred while executing immediate event: " << e.what() << std::endl;
        }
        immEventQueue.pop();
    }
}

void EventSys::executeTimedEvents()
{
    // 执行定时事件的实现
    sf::Time currentTime = eventSysClock.getElapsedTime();
    while (!timedEventQueue.empty() && timedEventQueue.top().triggerTime <= currentTime)
    {
        TimedEvent currentEvent = timedEventQueue.top();
        try
        {
            currentEvent.func();
        }
        catch (const std::exception& e)
        {
            // 处理异常：输出日志
            std::cerr << "Error occurred while executing timed event: " << e.what() << std::endl;
        }
        timedEventQueue.pop();
    }
}

sf::Time EventSys::getElapsedTime() const
{
    return eventSysClock.getElapsedTime();
}