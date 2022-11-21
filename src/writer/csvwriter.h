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
        ErrorStatus write(std::filesystem::path path, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) override;
        static ErrorStatus writePlain(std::filesystem::path path, std::vector<utility::u8stringlist> text, MergeTextMode overwriteMode = MergeTextMode::AcceptSource);

#ifdef ENABLE_TEST
        friend class Langscore_Test_WriterBase;
#endif

    protected:
        template<typename str_type>
        static str_type withoutQuote(str_type str)
        {
            //CSV用なので、改行かカンマがあれば何もしない
            if(str.find(str_type::value_type('\n')) != str_type::npos ||
               str.find(str_type::value_type(',')) != str_type::npos){
                return str;
            }
            if(str.empty()){ return str; }

            const auto CheckQuote = [](str_type::value_type c){ return c == str_type::value_type('\"') || c == str_type::value_type('\''); };
            if(CheckQuote(str[0])){
                str.erase(str.begin());
            }
            else{
                //末尾にクオートが有っても、文字列としてのクオート括りではないので無視
                return str;
            }

            auto pos = str.size() - 1;
            if(CheckQuote(str[pos])){
                str.erase(str.begin() + pos);
            }

            return str;
        }
    };
}

#endif // CSVWRITER_H
