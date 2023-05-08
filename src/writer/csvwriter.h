#ifndef CSVWRITER_H
#define CSVWRITER_H

#include "writerbase.h"
#include "utility.hpp"

#ifdef ENABLE_TEST
#define NOMINMAX
#include "iutest.hpp"

class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, ConvertCsvText_ASCII);
class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, ConvertCsvText_Multibyte);

#endif

namespace langscore
{
    class csvwriter: public writerbase
    {
#ifdef ENABLE_TEST
        IUTEST_FRIEND_TEST(Langscore_Writer, ConvertCsvText_ASCII);
        IUTEST_FRIEND_TEST(Langscore_Writer, ConvertCsvText_Multibyte);
#endif
    public:
        using writerbase::writerbase;
        constexpr static const char* extension = "csv";

        //現在のインスタンス内のテキストをベースとして、指定したCSVとマージする。
        bool merge(std::filesystem::path sourceFilePath) override;
        ErrorStatus write(std::filesystem::path path, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) override;
        static ErrorStatus writePlain(std::filesystem::path path, std::vector<utility::u8stringlist> text, MergeTextMode overwriteMode = MergeTextMode::AcceptSource);

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

        std::u8string convertCsvText(std::u8string_view text) const;
    };
}

#endif // CSVWRITER_H
