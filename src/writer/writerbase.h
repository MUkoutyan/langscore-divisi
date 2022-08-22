#ifndef WRITERBASE_H
#define WRITERBASE_H

#include "../serialize_base.h"
#include <tuple>
#include <filesystem>
#include "nlohmann/json.hpp"

#ifdef ENABLE_TEST
#include "iutest.hpp"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, DetectStringPositionFromFile);

#endif

namespace langscore
{
    class writerbase
    {
#ifdef ENABLE_TEST
        IUTEST_FRIEND_TEST(Langscore_Writer, DetectStringPositionFromFile);
#endif
    public:

        using TextCodec = std::u8string;

        writerbase(std::vector<std::u8string> langs, const nlohmann::json& json);
        writerbase(std::vector<std::u8string> langs, std::vector<TranslateText> texts);
        virtual ~writerbase();

        virtual bool write(std::filesystem::path writePath, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld) = 0;
        virtual bool merge(std::filesystem::path srcPath) { return true; }

        void setOverwriteMode(OverwriteTextMode overwriteMode){
            this->overwriteMode = overwriteMode;
        }
        std::vector<TranslateText>& curerntTexts() { return texts; }
        bool isDebug = false;

    protected:
        std::vector<std::u8string> useLangs;
        std::vector<TranslateText> texts;
        OverwriteTextMode overwriteMode;
        bool stackText;
        std::u8string stackTextStr;
        bool rangeComment;

        static void writeU8String(std::ofstream& out, std::u8string text);

        void addText(const nlohmann::json& json, std::u8string note = u8"");
        void addText(std::u8string text, std::u8string note = u8"");
        void json2tt(const nlohmann::json& json);

        std::tuple<std::u8string, bool> getObjectClass(const nlohmann::json& root);
        bool checkIgnoreKey(const std::u8string& currentClassName, const std::u8string& key, bool hasSpecIgnoreKeys);

        std::tuple<bool, int> checkEventCommandCode(const nlohmann::json& obj);
        void convertJArray(const nlohmann::json& arr, std::u8string parentClass = u8"", std::u8string arrayinKey = u8"");
        void convertJObject(const nlohmann::json& root);

        std::vector<TranslateText> convertScriptToCSV(std::filesystem::path path);
        enum class ProgressNextStep
        {
            Continue,
            Break,
            Throught
        };
        virtual ProgressNextStep checkCommentLine(TextCodec&) { return ProgressNextStep::Throught;  }

    };
}

#endif // WRITERBASE_H
