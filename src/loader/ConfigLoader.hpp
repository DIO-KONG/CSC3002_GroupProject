#pragma once
#include <unordered_map>
#include <variant>
#include <string>
#include <fstream>
// 使用std::variant来存储不同类型的配置值，例如屏幕分辨率int，角色名称string等
typedef std::variant<std::monostate, bool, int, float, std::string> ConfigValue;
// 使用std::unordered_map来存储键值对形式的配置数据
typedef std::unordered_map<std::string, ConfigValue> ConfigDict;

class ConfigLoader
{
    public:
        ConfigLoader();
        ~ConfigLoader();
        // 加载配置文件,需要两个参数：文件路径和节名称
        void loadConfig(const std::string& filepath, const std::string& section);
        // 根据键获取配置值
        ConfigValue getValue(const std::string& key) const;
    private:
        // 存储配置数据的字典
        ConfigDict configData;
};
