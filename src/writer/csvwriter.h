#ifndef CSVWRITER_H
#define CSVWRITER_H

#include "writerbase.h"
#include "utility.hpp"


namespace langscore
{
    class csvwriter: public writerbase
    {
    public:
        using writerbase::writerbase;
        constexpr static const char* extension = "csv";

        //現在のインスタンス内のテキストをベースとして、指定したCSVとマージする。
        bool merge(std::filesystem::path sourceFilePath) override;
        ErrorStatus write(std::filesystem::path path, std::u8string defaultLanguage, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) override;
        ErrorStatus writeForAnalyze(std::filesystem::path path, std::u8string defaultLanguage, MergeTextMode overwriteMode = MergeTextMode::AcceptSource);
        static ErrorStatus writePlain(std::filesystem::path path, std::vector<utility::u8stringlist> text, MergeTextMode overwriteMode = MergeTextMode::AcceptSource);

        static std::u8string convertCsvText(std::u8string_view text);
    protected:
        template<typename str_type>
        static str_type withoutQuote(str_type str)
        {
            using char_type = str_type::value_type;
            //CSV用なので、改行かカンマがあれば何もしない
            if(str.find(char_type('\n')) != str_type::npos ||
               str.find(char_type(',')) != str_type::npos){
                return str;
            }
            if(str.empty()){ return str; }

            const auto CheckQuote = [](char_type c){ return c == char_type('\"') || c == char_type('\''); };
            if(CheckQuote(str[0])){
                str.erase(str.begin());
            }
            else{
                //末尾にクオートが有っても、文字列としてのクオート括りではないので無視
                return str;
            }
            if (str.empty()) { return str; }

            auto pos = str.size() - 1;
            if(CheckQuote(str[pos])){
                str.erase(str.begin() + pos);
            }

            return str;
        }

    };
}

#endif // CSVWRITER_H
