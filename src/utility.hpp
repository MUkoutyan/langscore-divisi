#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cassert>

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

    template<typename str_type>
    static str_type replace(str_type str, str_type old, str_type newStr)
    {
        for(auto begin = str.find(old); begin != str_type::npos; begin = str.find(old, begin)){
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

    template<typename str_type>
    str_type left_trim(str_type s, str_type white_space)
    {
        size_t start = s.find_first_not_of(white_space);
        return (start == std::string::npos) ? str_type() : s.substr(start);
    }

    template<typename str_type>
    str_type right_trim(str_type s, str_type white_space)
    {
        size_t end = s.find_last_not_of(white_space);
        return (end == std::string::npos) ? str_type() : s.substr(0, end + 1);
    }

    template<typename str_type>
    str_type trim(str_type s){
        return right_trim(left_trim(s));
    }


    static std::vector<std::uint8_t> getFileData(std::filesystem::path path){
        std::ifstream f(path);
        f.seekg(0, std::ios_base::end);
        auto size = f.tellg();
        f.seekg(0, std::ios_base::beg);

        std::vector<std::uint8_t> result(size, 0);
        f.read(reinterpret_cast<char*>(&result[0]), size);

        return result;
    }

    static std::uint8_t getUTF8ByteLength(char8_t ptr)
    {
        size_t length = 1;
        if(ptr <= 0x7f){ length = 1; }
        else if(0xc2 <= ptr && ptr <= 0xdf){ length = 2; }
        else if(0xe0 <= ptr && ptr <= 0xef){ length = 3; }
        else if(0xf0 <= ptr && ptr <= 0xf4){ length = 4; }
        else {
            assert(("Invalid UTF8 String", false));
        }
        return length;
    }
}

