
#include <filesystem>
#include "serialize_base.h"
#include "utility.hpp"

namespace langscore
{
    class marshal_replacer
    {
    public:
        marshal_replacer(std::filesystem::path marshal_file_path);

        bool replace(std::vector<TranslateText>, std::u8string target_language);
    };
}