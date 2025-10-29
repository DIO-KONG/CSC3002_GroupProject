#include <SFML/System.hpp>

class Timer : public sf::Clock
{
public:
    Timer();
    ~Timer();
    double getElapsedSeconds() const;
};
