#ifndef WRITERBASE_H
#define WRITERBASE_H

#include "../serialize_base.h"
#include <tuple>
#include <filesystem>
#include "nlohmann/json.hpp"

namespace langscore
{
    class writerbase
    {
    public:

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

        static void writeU8String(std::ofstream& out, std::u8string text);

        void addText(const nlohmann::json& json, std::u8string note = u8"");
        void addText(std::u8string text, std::u8string note = u8"");
        void json2tt(const nlohmann::json& json);

        std::tuple<std::u8string, bool> getObjectClass(const nlohmann::json& root);
        bool checkIgnoreKey(const std::u8string& currentClassName, const std::u8string& key, bool hasSpecIgnoreKeys);

        std::tuple<bool, int> checkEventCommandCode(const nlohmann::json& obj);
        void convertJArray(const nlohmann::json& arr, std::u8string parentClass = u8"", std::u8string arrayinKey = u8"");
        void convertJObject(const nlohmann::json& root);
#ifdef ENABLE_TEST
        friend class Langscore_Test_WriterBase;
#endif
    };
}

#endif // WRITERBASE_H
