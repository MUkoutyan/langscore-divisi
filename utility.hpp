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

    template<typename basetype, typename base>
    static std::vector<basetype> split(basetype str, base deci)
    {
        auto result_size = std::count(str.begin(), str.end(), deci);

        std::vector<basetype> result;
        result.reserve(result_size);
        auto offset = basetype::size_type(0);
        while(true){
            auto pos = str.find(deci, offset);
            if(pos != basetype::npos){
                result.emplace_back(str.substr(offset, pos - offset));
                offset = pos + 1;
            }
            else{
                result.emplace_back(str.substr(offset));
                break;
            }
        }
        return result;
    }

    static std::string replace(std::string str, std::string old, std::string newStr)
    {
        for(auto begin = str.find(old); begin != std::string::npos; begin = str.find(old, begin)){
            str.replace(begin, old.size(), newStr);
            begin += newStr.size();
        }
        return str;
    }

    template<class From, class To>
    static std::vector<To> convert(std::vector<From> list)
    {
        std::vector<To> result;
        result.reserve(list.size());
        for(auto& s : list){
            result.emplace_back(s.begin(), s.end());
        }
        return result;
    }

    template<class To, class From>
    To cnvStr(From str){
        return To{str.begin(), str.end()};
    }

    const auto toString = [](const std::u8string& str){
        return std::string(str.begin(), str.end());
    };
}

