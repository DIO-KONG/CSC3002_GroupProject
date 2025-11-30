#pragma once
#include <SFML/System.hpp>
#include <functional>
#include <queue>
#include <iostream>
// #include <vector>
// #include <memory>

class EventSys
{
    public:
        using EventFunc = std::function<void()>;
        enum class ImmEventPriority
        {
            // 枚举事件类型与其对应优先级
            INPUT = 0,
            PRE_UPDATE,
            BOX2D,
            UPDATE,
            POST_UPDATE,
            DRAWBACKGROUND,
            DRAW
        };
        struct ImmEvent
        {
            ImmEventPriority priority;
            EventFunc func;
            // unsigned short id;
            // 重载小于运算符以便按优先级排序
            bool operator<(const ImmEvent& other) const
            {
                // return static_cast<int>(priority) > static_cast<int>(other.priority);
                return priority > other.priority;
            }
        };
        struct TimedEvent
        {
            sf::Time triggerTime;
            EventFunc func;
            // unsigned short id;
            // 重载小于运算符以便按时间排序
            bool operator<(const TimedEvent& other) const
            {
                return triggerTime > other.triggerTime;
            }
        };

        EventSys();
        ~EventSys();
        // 注册即时事件 参数：事件类型，事件函数
        void regImmEvent(const ImmEventPriority eventType, const EventFunc& func);
        // 注册定时事件 参数：延迟时间，事件函数
        void regTimedEvent(const sf::Time delay, const EventFunc& func);
        // 执行即时事件
        void executeImmEvents();
        // 执行定时事件
        void executeTimedEvents();
        // 获取事件系统运行时间 （游戏基准时钟）
        sf::Time getElapsedTime() const;

    private:
        // 存储即时事件和定时事件的优先队列
        std::priority_queue<ImmEvent> immEventQueue;
        std::priority_queue<TimedEvent> timedEventQueue;
        // 事件系统计时器
        sf::Clock eventSysClock;
};