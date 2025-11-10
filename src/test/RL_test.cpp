#include "ResourceLoader.hpp"
#include <iostream>

int main()
{
    ResourceLoader TestLoader("config\\flat_example.json");
    std::printf("关卡名字是: %s\n", std::get<std::string>(TestLoader.getResource("name")).c_str());
    std::printf("关卡背景图路径是: %s\n", std::get<std::string>(TestLoader.getResource("background")).c_str());
    TestLoader.addObjKey("enemies");
    std::printf("敌人数量是: %d\n", TestLoader.getObjCount("enemies"));
    for (int i = 0; i < TestLoader.getObjCount("enemies"); ++i)
    {
        std::printf("敌人 %d 的类型是: %s\n", i, std::get<std::string>(TestLoader.getObjResources(i, "enemies", "type")).c_str());
        std::printf("敌人 %d 的生命值是: %d\n", i, std::get<int>(TestLoader.getObjResources(i, "enemies", "health")));
    }
    std::printf("测试完成！\n");
}
