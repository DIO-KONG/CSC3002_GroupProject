#include "Timer.hpp"

Timer::Timer() : sf::Clock()
{
    // Timer类构造函数实现
}
Timer::~Timer()
{
    // Timer类析构函数实现
}

double Timer::getElapsedSeconds() const
{
    // 获取经过的秒数实现
    return sf::Clock::getElapsedTime().asSeconds();
}
