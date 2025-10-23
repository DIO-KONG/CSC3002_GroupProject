#include "ConfigLoader.hpp"

ConfigLoader::ConfigLoader()
{
    // ConfigLoader类构造函数实现
    
}

ConfigLoader::~ConfigLoader()
{
    // ConfigLoader类析构函数实现
}

void ConfigLoader::setBaseDir(const std::string& dir)
{
    // 设置基础目录的实现
    this->basedir = dir;
    printf("Base directory set to: %s\n", this->basedir.c_str());
    return;
}

void ConfigLoader::loadConfig(const std::string& filepath, const std::string& section)
{
    // ConfigLoader类加载配置文件实现
    // 读取配置文件并解析指定节的内容
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        // 处理文件打开失败的情况
        // 向Error类发出报错请求（待实现）
        return;
    }

    std::string line;
    bool inSection = false;
    while (std::getline(file, line))
    {
        // 检查当前行是否是目标节
        if (line.find("[") == 0)
        {
            if (inSection)
            {
                // 已经读取完目标节，退出循环
                break;
            }
            if (line == "[" + section + "]")
            {
                // 找到目标节
                inSection = true;
                continue;
            }
        }
        
        // 如果不在目标节，则跳过当前行
        if (!inSection)
        {
            continue;
        }

        // 解析键值对
        auto pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string valueStr = line.substr(pos + 1);
            
            // 去掉 key 与 valueStr 头尾的空白字符
            auto trim = [](std::string &s) {
                const char* ws = " \t\r\n";
                auto start = s.find_first_not_of(ws);
                if (start == std::string::npos) { s.clear(); return; }
                auto end = s.find_last_not_of(ws);
                s = s.substr(start, end - start + 1);
            };

            trim(key);
            trim(valueStr);
            
            ConfigValue value;

            // 简单类型推断
            if (valueStr == "true" || valueStr == "false")
            {
                // 布尔值处理
                // // debug
                // printf("Accept boolean value!!!");
                value = (valueStr == "true");
            }
            else if (valueStr == "")
            {   
                // 空值处理
                value = std::monostate{};
            }
            else
            {
                try
                {
                    size_t idx;
                    int intValue = std::stoi(valueStr, &idx);
                    if (idx == valueStr.size())
                    {
                        value = intValue;
                    }
                    else
                    {
                        float floatValue = std::stof(valueStr, &idx);
                        if (idx == valueStr.size())
                        {
                            value = floatValue;
                        }
                        else
                        {
                            value = valueStr; // 默认作为字符串处理
                        }
                    }
                }
                catch (...)
                {
                    value = valueStr; // 默认作为字符串处理
                }
            }

            // 将解析得到的键值对存储到configData中
            configData[key] = value;
        }
    }
    file.close();
    return;
}

ConfigValue ConfigLoader::getValue(const std::string& key) const
{
    // ConfigLoader类根据键获取配置值的实现
    if (auto it = configData.find(key); it != configData.end()) {
        return it->second;
    }
    return std::monostate{}; // 返回空值表示未找到
}