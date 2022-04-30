#pragma once
#include <string>
#include <vector>

namespace utility
{
    using stringlist = std::vector<std::string>;
    static std::string join(stringlist list, std::string deci)
    {
        auto size = 0;
        for(const auto& s : list){ size += s.size() + deci.size(); }

        std::string result;
        result.reserve(size);
        for(const auto& s : list){
            result += s;
            result += deci;
        }
        return result;
    }
}

