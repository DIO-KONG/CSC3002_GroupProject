#pragma once

// 针对MSVC编译器的DLL导出宏定义
#ifdef _WIN32
    #ifdef CONFIGLIB_EXPORTS
        #define CONFIGLIB_API __declspec(dllexport)
    #else
        #define CONFIGLIB_API __declspec(dllimport)
    #endif
#else
    #define CONFIGLIB_API
#endif

#include <unordered_map>
#include <variant>
#include <string>
#include <fstream>

class ConfigLoader
{
    // 使用std::variant来存储不同类型的配置值，例如屏幕分辨率int，角色名称string等
    using ConfigValue = std::variant<std::monostate, bool, int, float, std::string>;
    // 使用std::unordered_map来存储键值对形式的配置数据
    using ConfigDict = std::unordered_map<std::string, ConfigValue>;

    public:
        ConfigLoader();
        ~ConfigLoader();
        // 加载配置文件,需要两个参数：文件路径和节名称
        void loadConfig(const std::string& filepath, const std::string& section);
        // 根据键获取配置值
        ConfigValue getValue(const std::string& key) const;
        // 获取指定节的所有键值对
        ConfigDict getAllValues(const std::string& section) const;
        // 设置基础目录
        void setBaseDir(const std::string& dir);
        
    private:
        // 存储配置数据的字典
        ConfigDict configData;
        std::string basedir = "";
};
