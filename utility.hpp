#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace utility
{
    using stringlist = std::vector<std::string>;
    using u8stringlist = std::vector<std::u8string>;
    using filelist   = std::vector<std::filesystem::path>;

    template<typename basetype>
    static basetype join(std::vector<basetype> list, basetype deci)
    {
        auto size = 0;
        for(const auto& s : list){ size += s.size() + deci.size(); }

        basetype result;
        result.reserve(size);
        for(const auto& s : list){
            result += s;
            result += deci;
        }
        return result;
    }
}

