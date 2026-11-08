#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <iostream>
#include <string_view>
#include <system_error>

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
        auto offset = 0;
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

    template<typename basetype>
    static std::vector<basetype> split(basetype str, basetype deci)
    {
        size_t pos = 0;
        std::vector<basetype> result;
        while((pos = str.find(deci)) != basetype::npos) {
            auto token = str.substr(0, pos);
            result.emplace_back(std::move(token));
            str.erase(0, pos + deci.length());
        }
        result.emplace_back(str);
        return result;
    }

    template<typename str_type>
    [[nodiscard]]
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

    template<class To, class From>
    std::vector<To> cnvStr(std::vector<From> str) {
        std::vector<To> result;
        result.reserve(str.size());
        for(const auto& s : str) {
            result.emplace_back(cnvStr<To>(s));
        }
        return result;
    }

    template<typename str_type>
    bool includes(str_type source, str_type search)
    {
        auto start = source.find(search);
        return (start != str_type::npos);
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


    //ファイルの内容をそのまま読む。開けない場合は空を返す
    static std::vector<std::uint8_t> getFileData(std::filesystem::path path){
        //テキストモードは改行の変換や 0x1A で読み込みが止まるため、バイナリで読む
        std::ifstream f(path, std::ios::binary);
        if(f.is_open() == false) { return {}; }
        f.seekg(0, std::ios_base::end);
        auto size = f.tellg();
        f.seekg(0, std::ios_base::beg);
        if(size <= 0) { return {}; }

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
            assert(("Invalid UTF8 String" && false));
        }
        return length;
    }

    //文字列から拡張子を除去するテンプレート関数
    template<typename str_type>
    str_type removeExtension(str_type str)
    {
		auto pos = str.find_last_of('.');
        if(pos != str_type::npos){
			str.erase(pos);
		}
		return str;
	}

//ログ出力用。std::filesystem::pathをそのまま出力するとANSIコードページに変換されるため、UTF-8 にする
    inline std::string toUtf8(const std::filesystem::path& path) {
        return cnvStr<std::string>(path.u8string());
    }

    //std::filesystem の処理に失敗していれば、内容を標準エラーに出して true を返す
    inline bool FileSystemFailed(const std::error_code& errorCode, std::string_view operation, const std::filesystem::path& path)
    {
        if(static_cast<bool>(errorCode) == false) { return false; }
        //message() は ANSI コードページの文字列になり UTF-8 のログで化けるため、番号を出す
        std::cerr << "File system error : " << operation << " : " << toUtf8(path) << " (" << errorCode.value() << ")" << std::endl;
        return true;
    }

    //フォルダ内の項目を、例外を投げずに一覧にする。
    //失敗した場合は標準エラーに出し、それまでに取れた分を返す。先に一覧にするため、走査中にファイルを消しても問題ない
    template<class DirectoryIterator = std::filesystem::directory_iterator>
    std::vector<std::filesystem::directory_entry> ListDirectory(const std::filesystem::path& path)
    {
        std::vector<std::filesystem::directory_entry> entries;
        std::error_code errorCode;
        for(auto it = DirectoryIterator(path, errorCode); it != DirectoryIterator(); it.increment(errorCode)) {
            entries.emplace_back(*it);
        }
        FileSystemFailed(errorCode, "list directory", path);
        return entries;
    }

    inline void removeReadOnly(const std::filesystem::path& path)
    {
        namespace fs = std::filesystem;

        std::error_code errorCode;
        if(fs::exists(path, errorCode) == false) { return; }
        fs::permissions(path, fs::perms::owner_write, fs::perm_options::add, errorCode);
        FileSystemFailed(errorCode, "permissions", path);

        if(fs::is_directory(path, errorCode) == false) { return; }

        for(const auto& entry : ListDirectory<fs::recursive_directory_iterator>(path))
        {
            if(entry.is_directory(errorCode)) { continue; }
            fs::permissions(entry.path(), fs::perms::owner_write, fs::perm_options::add, errorCode);
            FileSystemFailed(errorCode, "permissions", entry.path());
        }
    }

}

