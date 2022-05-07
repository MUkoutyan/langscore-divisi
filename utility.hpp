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
        auto result_size = 0;
        for(const auto& s : list){ result_size += s.size() + deci.size(); }

        basetype result;
        result.reserve(result_size);
        auto size = list.size();
        for(decltype(size) i = 0; i < size; ++i){
            result += list[i];
            if(i < size-1){
                result += deci;
            }
        }
        return result;
    }
}

