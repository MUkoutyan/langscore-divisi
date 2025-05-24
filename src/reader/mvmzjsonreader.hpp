#pragma once

#include "jsonreader.hpp"
#include <filesystem>

namespace langscore
{

    namespace keys
    {
        constexpr const char8_t* name = u8"name";
        constexpr const char8_t* nickname = u8"nickname";
        constexpr const char8_t* profile = u8"profile";
        constexpr const char8_t* description = u8"description";
        constexpr const char8_t* list = u8"list";
        constexpr const char8_t* events = u8"events";
        constexpr const char8_t* message1 = u8"message1";
        constexpr const char8_t* message2 = u8"message2";
        constexpr const char8_t* message3 = u8"message3";
        constexpr const char8_t* message4 = u8"message4";
        constexpr const char8_t* armorTypes = u8"armorTypes";
        constexpr const char8_t* currencyUnit = u8"currencyUnit";
        constexpr const char8_t* elements = u8"elements";
        constexpr const char8_t* equipTypes = u8"equipTypes";
        constexpr const char8_t* gameTitle = u8"gameTitle";
        constexpr const char8_t* skillTypes = u8"skillTypes";
        constexpr const char8_t* terms = u8"terms";
        constexpr const char8_t* weaponTypes = u8"weaponTypes";
        constexpr const char8_t* pages = u8"pages";
        constexpr const char8_t* code = u8"code";
        constexpr const char8_t* parameters = u8"parameters";
        constexpr const char8_t* basic = u8"basic";
        constexpr const char8_t* commands = u8"commands";
        constexpr const char8_t* params = u8"params";
        constexpr const char8_t* messages = u8"messages";
        constexpr const char8_t* actionFailure = u8"actionFailure";
        constexpr const char8_t* actorDamage = u8"actorDamage";
        constexpr const char8_t* actorDrain = u8"actorDrain";
        constexpr const char8_t* actorGain = u8"actorGain";
        constexpr const char8_t* actorLoss = u8"actorLoss";
        constexpr const char8_t* actorNoDamage = u8"actorNoDamage";
        constexpr const char8_t* actorNoHit = u8"actorNoHit";
        constexpr const char8_t* actorRecovery = u8"actorRecovery";
        constexpr const char8_t* alwaysDash = u8"alwaysDash";
        constexpr const char8_t* bgmVolume = u8"bgmVolume";
        constexpr const char8_t* bgsVolume = u8"bgsVolume";
        constexpr const char8_t* buffAdd = u8"buffAdd";
        constexpr const char8_t* buffRemove = u8"buffRemove";
        constexpr const char8_t* commandRemember = u8"commandRemember";
        constexpr const char8_t* counterAttack = u8"counterAttack";
        constexpr const char8_t* criticalToActor = u8"criticalToActor";
        constexpr const char8_t* criticalToEnemy = u8"criticalToEnemy";
        constexpr const char8_t* debuffAdd = u8"debuffAdd";
        constexpr const char8_t* defeat = u8"defeat";
        constexpr const char8_t* emerge = u8"emerge";
        constexpr const char8_t* enemyDamage = u8"enemyDamage";
        constexpr const char8_t* enemyDrain = u8"enemyDrain";
        constexpr const char8_t* enemyGain = u8"enemyGain";
        constexpr const char8_t* enemyLoss = u8"enemyLoss";
        constexpr const char8_t* enemyNoDamage = u8"enemyNoDamage";
        constexpr const char8_t* enemyNoHit = u8"enemyNoHit";
        constexpr const char8_t* enemyRecovery = u8"enemyRecovery";
        constexpr const char8_t* escapeFailure = u8"escapeFailure";
        constexpr const char8_t* escapeStart = u8"escapeStart";
        constexpr const char8_t* evasion = u8"evasion";
        constexpr const char8_t* expNext = u8"expNext";
        constexpr const char8_t* expTotal = u8"expTotal";
        constexpr const char8_t* file = u8"file";
        constexpr const char8_t* levelUp = u8"levelUp";
        constexpr const char8_t* loadMessage = u8"loadMessage";
        constexpr const char8_t* magicEvasion = u8"magicEvasion";
        constexpr const char8_t* magicReflection = u8"magicReflection";
        constexpr const char8_t* meVolume = u8"meVolume";
        constexpr const char8_t* obtainExp = u8"obtainExp";
        constexpr const char8_t* obtainGold = u8"obtainGold";
        constexpr const char8_t* obtainItem = u8"obtainItem";
        constexpr const char8_t* obtainSkill = u8"obtainSkill";
        constexpr const char8_t* partyName = u8"partyName";
        constexpr const char8_t* possession = u8"possession";
        constexpr const char8_t* preemptive = u8"preemptive";
        constexpr const char8_t* saveMessage = u8"saveMessage";
        constexpr const char8_t* seVolume = u8"seVolume";
        constexpr const char8_t* substitute = u8"substitute";
        constexpr const char8_t* surprise = u8"surprise";
        constexpr const char8_t* useItem = u8"useItem";
        constexpr const char8_t* victory = u8"victory";
    }
	class mvmz_jsonreader: public jsonreaderbase
	{
	public:
		mvmz_jsonreader(const std::filesystem::path& path, std::vector<std::u8string> useLangs, const nlohmann::json& json)
			: jsonreaderbase(json)
			, stackText(false)
			, stackTextStr(u8"")
			, currentDataType(DetectDataType(path))
		{
            this->useLangList = std::move(useLangs);
			json2tt();	
		}
		~mvmz_jsonreader() override {}

	private:
		void json2tt() override
		{
			if(json.is_array())
			{
				convertJArray(json, 0);
			}
			else if(json.is_object())
			{
				convertJsonObject(json, 0);
			}
		}

		enum class DataType
		{
			Actors, Armors, Classes, CommonEvents, Enemies, Items, Map, 
			Skills, States, System, Troops, Weapons
		};

        //特定のデータタイプで検出するキー
        const std::map<DataType, std::vector<std::u8string>> detectClassKeys = {
            {DataType::Actors,			{keys::name, keys::nickname, keys::profile}},
            {DataType::Armors,			{keys::name, keys::description}},
            {DataType::Classes,			{keys::name}},
            //CommonEventsはlist内のオブジェクトに対してcodeの値を調べる必要がある。
            {DataType::CommonEvents,    {keys::list}},
            {DataType::Enemies,			{keys::name}},
            {DataType::Items,			{keys::name, keys::description}},
            {DataType::Map,				{keys::events}},
            {DataType::Skills,			{keys::name, keys::description, keys::message1, keys::message2}},
            {DataType::States,			{keys::name, keys::description, keys::message1, keys::message2, keys::message3, keys::message4}},
            {DataType::System,			{keys::armorTypes, keys::currencyUnit, keys::elements, keys::equipTypes,
                                         keys::gameTitle, keys::skillTypes, keys::terms, keys::weaponTypes}},
            {DataType::Troops,			{keys::pages}},
            {DataType::Weapons,			{keys::name, keys::description}},
        };
        //データタイプ内で検出するキー detectClassKeysで検出されたキーは、この中に含まれているかをまず調べる。
        //検出した場合はこの中のキーを検出する。これはdetectKeyInObjectの値も、再帰的にdetectKeyInObjectで検索されなければいけない。
        const std::unordered_map<std::u8string, std::vector<std::u8string>> detectKeyInObject = {
            {keys::events,		{keys::pages}},
            {keys::pages,		{keys::list}},
            {keys::list,		{keys::code, keys::parameters}},
            {keys::terms,		{keys::basic, keys::commands, keys::params, keys::messages}},
            {keys::messages,	{keys::actionFailure, keys::actorDamage, keys::actorDrain, keys::actorGain,
                                 keys::actorLoss, keys::actorNoDamage, keys::actorNoHit, keys::actorRecovery,
                                 keys::alwaysDash, keys::bgmVolume, keys::bgsVolume, keys::buffAdd, keys::buffRemove,
                                 keys::commandRemember, keys::counterAttack, keys::criticalToActor, keys::criticalToEnemy,
                                 keys::debuffAdd, keys::defeat, keys::emerge, keys::enemyDamage, keys::enemyDrain,
                                 keys::enemyGain, keys::enemyLoss, keys::enemyNoDamage, keys::enemyNoHit,
                                 keys::enemyRecovery, keys::escapeFailure, keys::escapeStart, keys::evasion,
                                 keys::expNext, keys::expTotal, keys::file, keys::levelUp, keys::loadMessage,
                                 keys::magicEvasion, keys::magicReflection, keys::meVolume, keys::obtainExp,
                                 keys::obtainGold, keys::obtainItem, keys::obtainSkill, keys::partyName, keys::possession,
                                 keys::preemptive, keys::saveMessage, keys::seVolume, keys::substitute, keys::surprise,
                                 keys::useItem, keys::victory}}
        };

        const std::unordered_map<std::u8string, const char8_t*> keyToTextType = {
            // battleMessage 系
            {keys::actionFailure,       TranslateText::battleMessage},
            {keys::actorDamage,         TranslateText::battleMessage},
            {keys::actorDrain,          TranslateText::battleMessage},
            {keys::actorGain,           TranslateText::battleMessage},
            {keys::actorLoss,           TranslateText::battleMessage},
            {keys::actorNoDamage,       TranslateText::battleMessage},
            {keys::actorNoHit,          TranslateText::battleMessage},
            {keys::actorRecovery,       TranslateText::battleMessage},
            {keys::buffAdd,             TranslateText::battleMessage},
            {keys::buffRemove,          TranslateText::battleMessage},
            {keys::commandRemember,     TranslateText::battleMessage},
            {keys::counterAttack,       TranslateText::battleMessage},
            {keys::criticalToActor,     TranslateText::battleMessage},
            {keys::criticalToEnemy,     TranslateText::battleMessage},
            {keys::debuffAdd,           TranslateText::battleMessage},
            {keys::defeat,              TranslateText::battleMessage},
            {keys::emerge,              TranslateText::battleMessage},
            {keys::enemyDamage,         TranslateText::battleMessage},
            {keys::enemyDrain,          TranslateText::battleMessage},
            {keys::enemyGain,           TranslateText::battleMessage},
            {keys::enemyLoss,           TranslateText::battleMessage},
            {keys::enemyNoDamage,       TranslateText::battleMessage},
            {keys::enemyNoHit,          TranslateText::battleMessage},
            {keys::enemyRecovery,       TranslateText::battleMessage},
            {keys::escapeFailure,       TranslateText::battleMessage},
            {keys::escapeStart,         TranslateText::battleMessage},
            {keys::evasion,             TranslateText::battleMessage},
            {keys::levelUp,             TranslateText::battleMessage},
            {keys::magicEvasion,        TranslateText::battleMessage},
            {keys::magicReflection,     TranslateText::battleMessage},
            {keys::obtainExp,           TranslateText::battleMessage},
            {keys::obtainGold,          TranslateText::battleMessage},
            {keys::obtainItem,          TranslateText::battleMessage},
            {keys::obtainSkill,         TranslateText::battleMessage},
            {keys::preemptive,          TranslateText::battleMessage},
            {keys::substitute,          TranslateText::battleMessage},
            {keys::surprise,            TranslateText::battleMessage},
            {keys::useItem,             TranslateText::battleMessage},
            {keys::victory,             TranslateText::battleMessage},

            {keys::partyName,           TranslateText::nameType},

            {keys::alwaysDash,          TranslateText::other},
            {keys::bgmVolume,           TranslateText::other},
            {keys::bgsVolume,           TranslateText::other},
            {keys::meVolume,            TranslateText::other},
            {keys::seVolume,            TranslateText::other},
            {keys::possession,          TranslateText::other},
            {keys::saveMessage,         TranslateText::message},
            {keys::loadMessage,         TranslateText::message},
            {keys::file,                TranslateText::other},

            {keys::expNext,             TranslateText::other},
            {keys::expTotal,            TranslateText::other}
        };

		bool stackText;
		std::u8string stackTextStr;
		DataType currentDataType;

		void addText(const nlohmann::json& json, int code = 0)
		{
			std::string valStr;
			json.get_to(valStr);

			//stackTextが有効の場合、手動の中央揃え等で空行を使用する場合があるため、空文字の検出は行わない。
			//if(valStr.empty()){ return; }

			std::u8string original(valStr.begin(), valStr.end());
			addText(std::move(original), code);
		}
		void addText(std::u8string text, int code = 0)
		{
			if(stackText){
				//1行に付き必ず改行が挟まる。(VXAceのみの仕様？MV/MZは要確認)
				stackTextStr += text + u8'\n';
				return;
			}
			else
			{
				if(code == 401)
                {
					if(0 < text.size() && *(text.rbegin()) == u8'\n') {
						text.erase((text.rbegin().base())-1);
					}
                    if(0 < text.size() && *(text.rbegin()) == u8'\r') {
                        text.erase((text.rbegin().base()) - 1);
                    }
				}
                if(text.empty()) {
                    //ただの空文は無視する。
                    return;
                }
			}

			//※リード処理はCSV用のテキストに変換しない。

			TranslateText t(std::move(text), useLangList);
			t.code = code;

            t.textType.emplace_back(textTypeForMaker);
            textTypeForMaker = TranslateText::other;

			auto result = std::find_if(texts.begin(), texts.end(), [&t](const auto& x){
				return x.original == t.original;
			});
			if(result == texts.end()){
				texts.emplace_back(std::move(t));
			}
            else if((code == 320 || code == 324) && result->code == 102) {
                //名前の変更・二つ名の変更の場合、選択肢の文章がある場合はcodeを上書きする。
                result->code = code;
                result->textType = t.textType;
            }
		}

		std::u8string getObjectClass(const nlohmann::json& root)
		{
			std::u8string currentObjectKey = u8"";

			const auto& mainClassKeys = detectClassKeys.at(currentDataType);

			for(auto& key : mainClassKeys)
			{
				auto strKey = utility::cnvStr<std::string>(key);
				if(root.find(strKey) != root.end())
				{
					currentObjectKey = utility::cnvStr<std::u8string>(root[strKey]);
				}
			}

			return currentObjectKey;
		}

		std::tuple<bool, int> checkEventCommandCode(const nlohmann::json& obj)
		{
			bool result = false;
			int code = 0;
			for(auto s = obj.begin(); s != obj.end(); ++s)
			{
				if(s.key() == "code"){
					//許可するコード
					s->get_to(code);
					switch(code){
                        case 101: [[fallthrough]]; //メッセージのパラメータ(401の前に来る)
						case 102: [[fallthrough]]; //選択肢
						case 401: [[fallthrough]]; //文章の表示
						case 405: [[fallthrough]]; //スクロールの文章の表示
						case 320: [[fallthrough]]; //アクター名の変更
						case 324: [[fallthrough]]; //二つ名の変更
						case 325: //プロフィールの変更
							//case 231: //画像の表示
							result = true;
							break;
						default:
							break;
					}
				}

				if(result){ break; }
			}
			return std::forward_as_tuple(result, code);
		}
		void convertJArray(const nlohmann::json& arr, int code, std::u8string parentClass = u8"")
		{
			if(detectKeyInObject.find(parentClass) != detectKeyInObject.end())
			{
				const auto& recursiveDetects = detectKeyInObject.at(parentClass);
				for(auto s = arr.begin(); s != arr.end(); ++s)
				{
					if(s->is_array()){
						convertJArray(*s, code, parentClass);
						continue;
					}
					else if(s->is_object())
					{
						convertJObjectInKey(parentClass, *s, code);
					}
					else if(s->is_string()){
						addText(*s, code);
					}
				}
			}
			else
			{
				for(auto s = arr.begin(); s != arr.end(); ++s)
				{
					if(s->is_array()){
						convertJArray(*s, code, parentClass);
					}
					else if(s->is_object()){
						convertJsonObject(*s, code);
					}
					else if(s->is_string())
					{
						addText(*s, code);
					}
				}
			}
		}

		void convertJsonObject(const nlohmann::json& root, int code)
		{
			if(root.empty()){ return; }

			const auto& mainClassKeys = detectClassKeys.at(currentDataType);
			convertJObjectCore(mainClassKeys, root, code);
		}

		void convertJObjectInKey(std::u8string rootKey, const nlohmann::json& root, int code)
		{
			if(root.empty()){ return; }

			if(currentDataType == DataType::System && rootKey == u8"messages"){
				spetializeSystemMessage(root);
			}
			else{
				const auto& mainClassKeys = detectKeyInObject.at(rootKey);
				convertJObjectCore(mainClassKeys, root, code);
			}
		}

		void convertJObjectCore(utility::u8stringlist mainClassKeys, const nlohmann::json& root, int code)
		{
			for(auto s = root.begin(); s != root.end(); ++s)
			{
				if(s->is_null()){ continue; }
				auto key = utility::cnvStr<std::u8string>(s.key());
				for(auto& checkKey : mainClassKeys)
				{
					if(checkKey != key){ continue; }

					if(checkKey == u8"code")
                    {
                        std::u8string textTypeForMakerTmp;
						auto [isCheckParameters, _code] = checkEventCommandCode(root);
						code = _code;

                        if(code == 101)
                        {
                            //顔グラフィックが指定されているかのチェック。
                            //@parameter(valueは配列)の第一要素が空でなければ顔グラ有り。
                            for(auto s = root.begin(); s != root.end(); ++s)
                            {
                                if(s.key() != "parameters") {
                                    continue;
                                }
                                nlohmann::json arrayData;
                                s->get_to(arrayData);
                                if(1 < arrayData.size()) {
                                    std::string faceGraphicName;
                                    arrayData[0].get_to(faceGraphicName);
                                    if(faceGraphicName.empty()) {
                                        textTypeForMakerTmp = TranslateText::message;
                                    }
                                    else {
                                        textTypeForMakerTmp = TranslateText::messageWithGraphic;
                                    }
                                    isCheckParameters = false;
                                    break;
                                }
                            }
                        }
                        else if(code == 105)
                        {
                            textTypeForMakerTmp = TranslateText::message;
                        }
                        else if(code == 320 || code == 324)
                        {
                            for(auto s = root.begin(); s != root.end(); ++s)
                            {
                                if(s.key() != "parameters") {
                                    continue;
                                }
                                nlohmann::json arrayData;
                                s->get_to(arrayData);
                                if(1 < arrayData.size()) {
                                    std::string actorNameText;
                                    arrayData[1].get_to(actorNameText);

                                    if(actorNameText.empty()) {
                                        textTypeForMaker = TranslateText::other;
                                    }
                                    else {
                                        textTypeForMaker = TranslateText::nameType;
                                    }
                                    break;
                                }
                            }
                        }
                        else if(code == 325)
                        {
                            for(auto s = root.begin(); s != root.end(); ++s)
                            {
                                if(s.key() != "parameters") {
                                    continue;
                                }
                                nlohmann::json arrayData;
                                s->get_to(arrayData);
                                if(1 < arrayData.size()) {
                                    //プロフィールの変更はイベント内に文章が埋められているので、
                                    //addText後の入れ替えではなく即時に適用できるようにする。
                                    textTypeForMaker = TranslateText::descriptionType;
                                }
                            }
                        }

						if(stackText == false && (code == 401 || code == 405)){
							stackText = true;
						}
						else if(stackText && (code != 401 && code != 405)){
							stackText = false;
							addText(stackTextStr, 401);
							stackTextStr.clear();
                            //401 -> 401 -> 101等で先にtextTypeForMakerを設定すると
                            //先の文章のタイプが埋められてしまうためここで設定する。
                            if(textTypeForMakerTmp.empty() == false) {
                                textTypeForMaker = std::move(textTypeForMakerTmp);
                            }
						}

						if(isCheckParameters == false){
                            //401 -> 401 -> 101等で先にtextTypeForMakerを設定すると
                            //先の文章のタイプが埋められてしまうためここで設定する。
                            if(textTypeForMakerTmp.empty() == false) {
                                textTypeForMaker = std::move(textTypeForMakerTmp);
                            }
                            return; 
                        }
					}

					const auto& val = s.value();
					if(detectKeyInObject.find(checkKey) != detectKeyInObject.end())
					{
						if(val.is_array()){
							convertJArray(val, code, key);
						}
						else if(val.is_object()){
							convertJObjectInKey(key, val, code);
						}
						continue;
					}

					if(val.is_array()){
						convertJArray(val, code, key);
						continue;
					}
					else if(val.is_object()){
						convertJsonObject(val, code);
						continue;
					}
					else if(val.is_string() == false){ continue; }

                    detectTextType(key);

					addText(*s, code);
				}
			}
		}

        void detectTextType(const std::u8string& key)
        {

            if(key.find(u8"message") != std::u8string::npos) {
                textTypeForMaker = TranslateText::battleMessage;
            }
            else if(key == u8"name" || key == u8"nickname") {
                if(currentDataType == DataType::Skills || currentDataType == DataType::Enemies) {
                    textTypeForMaker = TranslateText::battleName;
                }
                else {
                    textTypeForMaker = TranslateText::nameType;
                }
            }
            else if(key == u8"description" || key == u8"profile") {
                textTypeForMaker = TranslateText::descriptionType;
            }
            else if(keyToTextType.find(key) != keyToTextType.end()) {
                textTypeForMaker = keyToTextType.at(key);
            }
            else {
                textTypeForMaker = TranslateText::other;
            }
        }

		void spetializeSystemMessage(const nlohmann::json& root)
		{
			for(auto s = root.begin(); s != root.end(); ++s)
			{
				if(s->is_null()){ continue; }
                detectTextType(utility::cnvStr<std::u8string>(s.key()));
				const auto& val = s.value();
				if(val.is_string() == false){ continue; }
				addText(*s, 0);
			}
		}

		DataType DetectDataType(const std::filesystem::path& path)
		{
			auto filename = path.filename().u8string();

			const auto FindStr = [&](std::u8string_view s){
				return filename.find(s) == 0;
			};

			if(FindStr(u8"Actors")){
				return DataType::Actors;
			}
			else if(FindStr(u8"Armors")){
				return DataType::Armors;
			}
			else if(FindStr(u8"Classes")){
				return DataType::Classes;
			}
			else if(FindStr(u8"CommonEvents")){
				return DataType::CommonEvents;
			}
			else if(FindStr(u8"Enemies")){
				return DataType::Enemies;
			}
			else if(FindStr(u8"Items")){
				return DataType::Items;
			}
			else if(FindStr(u8"Map")){
				return DataType::Map;
			}
			else if(FindStr(u8"Skills")){
				return DataType::Skills;
			}
			else if(FindStr(u8"States")){
				return DataType::States;
			}
			else if(FindStr(u8"System")){
				return DataType::System;
			}
			else if(FindStr(u8"Troops")){
				return DataType::Troops;
			}
			else if(FindStr(u8"Weapons")){
				return DataType::Weapons;
			}
			return DataType::Map;
		}

        std::u8string textTypeForMaker = TranslateText::other;
	};
}