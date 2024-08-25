#pragma once
#include <string>

class ErrorStatus
{
public:
	static constexpr int NoError = 0;

    enum class Module : std::int16_t
    {
        None = 0,
        DIVISI,
        CONFIG,
        INVOKER,
        PLATFORM_BASE,
        DIVISI_VXACE,
        READERBASE = 10,
        CSVREADER,
        WRITERBASE = 20,
        CSVWRITER,
        RUBYSCRIPTWRITER,
        SCRIPTTEXTPARSER,
    };

    ErrorStatus(Module module = Module::None, int code = NoError): _moduleCode(module), _code(code), specMsg("") {}
    int val() const noexcept { return int(_moduleCode) << 4 | _code; }
    Module moduleCode() const noexcept { return _moduleCode; }
    int code() const noexcept { return _code; }
    bool valid() const noexcept { return _moduleCode == Module::None && _code == NoError; }
    bool invalid() const noexcept { return !valid(); }

    auto operator<=>(const ErrorStatus&) const = default;

    void setSpecMsg(std::string m) { specMsg = std::move(m); }
    std::string toStr()
    {
        switch(_moduleCode){
            case Module::PLATFORM_BASE:
                return errorSpecPlatformBase();
            case Module::DIVISI:
                return errorDivisi();
            case Module::CONFIG:
                return errorConfig();
            case Module::INVOKER:
                return errorInvoker();
            case Module::DIVISI_VXACE:
                return errorDivisiVxAce();
            case Module::READERBASE:
                return errorReaderBase();
            case Module::CSVREADER:
                return errorCsvReader();
            case Module::WRITERBASE:
                return errorWriterBase();
            case Module::CSVWRITER:
                return errorCsvWriter();
            case Module::RUBYSCRIPTWRITER:
                return errorRubyScriptWriter();
            case Module::SCRIPTTEXTPARSER:
                return errorScriptTextParser();
        }
        return "";
    }

private:
    Module _moduleCode;
    std::int16_t _code;
    std::string specMsg;

    std::string errorSpecPlatformBase()
    {
        switch(_code)
        {
            case NoError: return "";
            case 1: return "error code 1 : Failed to Marge";
        }
        return "";
    }
    std::string errorDivisi()
    {
        switch(_code)
        {
            case NoError: return "";
            case 1: return "error code 1 : Not Found Project Path.";
            case 2: return "error code 2 : Unsupport Project Type";
        }
        return "";

    }
    std::string errorConfig()
    {
        return "";
    }
    std::string errorInvoker()
    {
        switch(_code)
        {
            case NoError: return "";
            case 1: return "error code 1 : Unsupport Project Type";
            case 2: return "error code 2 : Not Found Execute file.";
            case 3: return "error code 3 : Not Found Convert file.";
            case 4: return "error code 4 : Failed to convert.";
            case 5: return "error code 5 : Invalid ExecuteFile!!!!";
            case 6: return "error code 6 : Failed Create Output File.";
            case 7: return "error code 7 : Failed Create Output File.";
            case 8: return "error code 8 : Failed Create Output File.";
            case 255: return this->specMsg;
        }
        return "";
    }
    std::string errorDivisiVxAce()
    {
        switch (_code)
        {
        case NoError: return "";
        case 1: return "error code 1 : validate error. Not Found Packing Input Directory.";
        }
        return "";
    }
    std::string errorReaderBase()
    {
        return "";
    }
    std::string errorCsvReader()
    {
        return "";
    }
    std::string errorWriterBase()
    {
        return "";
    }
    std::string errorCsvWriter()
    {
        return "";
    }
    std::string errorRubyScriptWriter()
    {
        return "";
    }
    std::string errorScriptTextParser()
    {
        return "";
    }

};

const static ErrorStatus Status_Success{ErrorStatus::Module::None, ErrorStatus::NoError};