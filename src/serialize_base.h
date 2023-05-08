#pragma once
#include <string>
#include <vector>
#include <map>

namespace langscore
{
    enum class MergeTextMode
    {
        AcceptSource = 0,   //���̃t�@�C�����c��
        AcceptTarget = 1,   //��̃t�@�C�����c��
        MergeKeepSource,    //���������ꍇ�A���t�@�C���̓��e��ێ�����B
        MergeKeepTarget,    //���������ꍇ�A��t�@�C���̓��e��ێ�����B
        Both,               //�����c��
    };

    struct PluginInfo
    {
        std::u8string name;
        std::u8string filename; //�g���q�͂��邱��
        bool status = false;
        std::u8string description;
        std::map<std::u8string, std::u8string> parameters;
    };

    //�|��e�L�X�g�̃N���X
    class TranslateText
    {
    public:
        TranslateText():TranslateText(u8"", {}) {}
        TranslateText(std::u8string origin, std::vector<std::u8string> langs)
            : original(std::move(origin)), translates(), scriptLineInfo(), code(0)
        {
            for(auto& lang : langs){
                this->translates[lang] = u8"";
            }
        }
        std::u8string original; //�I���W�i���̕���
        std::map<std::u8string, std::u8string> translates;  //�e���ꖈ�̕���
        std::u8string scriptLineInfo;   //�X�N���v�g�ł̃I���W�i���̕��͂̍s���
        int code;   //RPG�c�N�[���ŃR�}���h�����o�����ۂɎg�p

        //CSV�p�̃w�b�_�[���쐬
        auto createHeader() const
        {
            std::vector<std::u8string> result;
            result.emplace_back(u8"original");
            for(const auto& l : translates){
                result.emplace_back(l.first);
            }
            return result;
        }
    };

    using ScriptPackage = std::vector<std::tuple<std::u8string, std::vector<TranslateText>>>;
}