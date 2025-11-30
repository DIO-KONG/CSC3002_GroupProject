#include "ResourceLoader.hpp"
#include <iostream>

int main()
{
    ResourceLoader TestLoader("config\\menu.json");
    std::string name = std::get<std::string>(TestLoader.getResource("name"));
    std::cout << "Loaded resource name: " << name << std::endl;
    std::vector<std::string> objKeys = TestLoader.getObjKeys();
    for (const std::string& key : objKeys) {
        printf("Object Key: %s\n", key.c_str());
        int objCount = TestLoader.getObjCount(key);
        printf("Object Count: %d\n", objCount);
    }
    float gravityX = std::get<float>(TestLoader.getResource("gravityX"));
    printf("Gravity X: %.2f\n", gravityX);
    float gravityY = std::get<float>(TestLoader.getResource("gravityY"));
    printf("Gravity Y: %.2f\n", gravityY);
    return 0;
}
