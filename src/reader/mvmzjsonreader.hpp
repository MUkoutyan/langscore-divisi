#pragma once

#include "jsonreader.hpp"
#include <filesystem>

namespace langscore
{

    namespace mvmz_eventcode
    {
        constexpr static int MessageParameter = 101;
        constexpr static int ShowChoices = 102;
        constexpr static int InputNumber = 103;
        constexpr static int SelectItem = 104;
        constexpr static int ScrollMessageParameter = 105;
        constexpr static int CommonEvent = 117;
        constexpr static int ChangeActorName = 320;
        constexpr static int ChangeActorImage = 322;
        constexpr static int ChangeActorNickname = 324;
        constexpr static int ChangeActorProfile = 325;
        constexpr static int ShowText = 401;
        constexpr static int ShowScrollingText = 405;
    }

    namespace mvmz_keys
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
        constexpr const char8_t* note = u8"note";
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
            AuxiliaryNodeInfo auxInfo = {u8"", u8"/", AuxiliaryNodeInfo::Array};
			if(json.is_array())
			{
                convertArrayNode(json, 0, auxInfo);
			}
			else if(json.is_object())
			{
				convertObjectNode(json, 0, auxInfo);
			}
		}

		enum class DataType
		{
			Actors, Armors, Classes, CommonEvents, Enemies, Items, Map, 
			Skills, States, System, Troops, Weapons
		};

        //特定のデータタイプで検出するキー
        const std::map<DataType, std::vector<std::u8string>> detectClassKeys = {
            {DataType::Actors,			{mvmz_keys::name, mvmz_keys::nickname, mvmz_keys::profile}},
            {DataType::Armors,			{mvmz_keys::name, mvmz_keys::description}},
            {DataType::Classes,			{mvmz_keys::name}},
            //CommonEventsはlist内のオブジェクトに対してcodeの値を調べる必要がある。
            {DataType::CommonEvents,    {mvmz_keys::list}},
            {DataType::Enemies,			{mvmz_keys::name}},
            {DataType::Items,			{mvmz_keys::name, mvmz_keys::description}},
            {DataType::Map,				{mvmz_keys::events}},
            {DataType::Skills,			{mvmz_keys::name, mvmz_keys::description, mvmz_keys::message1, mvmz_keys::message2}},
            {DataType::States,			{mvmz_keys::name, mvmz_keys::description, mvmz_keys::message1, mvmz_keys::message2, mvmz_keys::message3, mvmz_keys::message4}},
            {DataType::System,			{mvmz_keys::armorTypes, mvmz_keys::currencyUnit, mvmz_keys::elements, mvmz_keys::equipTypes,
                                         mvmz_keys::gameTitle, mvmz_keys::skillTypes, mvmz_keys::terms, mvmz_keys::weaponTypes}},
            {DataType::Troops,			{mvmz_keys::pages}},
            {DataType::Weapons,			{mvmz_keys::name, mvmz_keys::description}},
        };
        //データタイプ内で検出するキー detectClassKeysで検出されたキーは、この中に含まれているかをまず調べる。
        //検出した場合はこの中のキーを検出する。これはdetectKeyInObjectの値も、再帰的にdetectKeyInObjectで検索されなければいけない。
        const std::unordered_map<std::u8string, std::vector<std::u8string>> detectKeyInObject = {
            {mvmz_keys::events,		{mvmz_keys::pages}},
            {mvmz_keys::pages,		{mvmz_keys::list}},
            {mvmz_keys::list,		{mvmz_keys::code, mvmz_keys::parameters}},
            {mvmz_keys::terms,		{mvmz_keys::basic, mvmz_keys::commands, mvmz_keys::params, mvmz_keys::messages}},
            {mvmz_keys::messages,	{mvmz_keys::actionFailure, mvmz_keys::actorDamage, mvmz_keys::actorDrain, mvmz_keys::actorGain,
                                     mvmz_keys::actorLoss, mvmz_keys::actorNoDamage, mvmz_keys::actorNoHit, mvmz_keys::actorRecovery,
                                     mvmz_keys::alwaysDash, mvmz_keys::bgmVolume, mvmz_keys::bgsVolume, mvmz_keys::buffAdd, mvmz_keys::buffRemove,
                                     mvmz_keys::commandRemember, mvmz_keys::counterAttack, mvmz_keys::criticalToActor, mvmz_keys::criticalToEnemy,
                                     mvmz_keys::debuffAdd, mvmz_keys::defeat, mvmz_keys::emerge, mvmz_keys::enemyDamage, mvmz_keys::enemyDrain,
                                     mvmz_keys::enemyGain, mvmz_keys::enemyLoss, mvmz_keys::enemyNoDamage, mvmz_keys::enemyNoHit,
                                     mvmz_keys::enemyRecovery, mvmz_keys::escapeFailure, mvmz_keys::escapeStart, mvmz_keys::evasion,
                                     mvmz_keys::expNext, mvmz_keys::expTotal, mvmz_keys::file, mvmz_keys::levelUp, mvmz_keys::loadMessage,
                                     mvmz_keys::magicEvasion, mvmz_keys::magicReflection, mvmz_keys::meVolume, mvmz_keys::obtainExp,
                                     mvmz_keys::obtainGold, mvmz_keys::obtainItem, mvmz_keys::obtainSkill, mvmz_keys::partyName, mvmz_keys::possession,
                                     mvmz_keys::preemptive, mvmz_keys::saveMessage, mvmz_keys::seVolume, mvmz_keys::substitute, mvmz_keys::surprise,
                                     mvmz_keys::useItem, mvmz_keys::victory}}
        };

        const std::unordered_map<std::u8string, const char8_t*> keyToTextType = {
            // battleMessage 系
            {mvmz_keys::actionFailure,       TranslateText::battleMessage},
            {mvmz_keys::actorDamage,         TranslateText::battleMessage},
            {mvmz_keys::actorDrain,          TranslateText::battleMessage},
            {mvmz_keys::actorGain,           TranslateText::battleMessage},
            {mvmz_keys::actorLoss,           TranslateText::battleMessage},
            {mvmz_keys::actorNoDamage,       TranslateText::battleMessage},
            {mvmz_keys::actorNoHit,          TranslateText::battleMessage},
            {mvmz_keys::actorRecovery,       TranslateText::battleMessage},
            {mvmz_keys::buffAdd,             TranslateText::battleMessage},
            {mvmz_keys::buffRemove,          TranslateText::battleMessage},
            {mvmz_keys::commandRemember,     TranslateText::battleMessage},
            {mvmz_keys::counterAttack,       TranslateText::battleMessage},
            {mvmz_keys::criticalToActor,     TranslateText::battleMessage},
            {mvmz_keys::criticalToEnemy,     TranslateText::battleMessage},
            {mvmz_keys::debuffAdd,           TranslateText::battleMessage},
            {mvmz_keys::defeat,              TranslateText::battleMessage},
            {mvmz_keys::emerge,              TranslateText::battleMessage},
            {mvmz_keys::enemyDamage,         TranslateText::battleMessage},
            {mvmz_keys::enemyDrain,          TranslateText::battleMessage},
            {mvmz_keys::enemyGain,           TranslateText::battleMessage},
            {mvmz_keys::enemyLoss,           TranslateText::battleMessage},
            {mvmz_keys::enemyNoDamage,       TranslateText::battleMessage},
            {mvmz_keys::enemyNoHit,          TranslateText::battleMessage},
            {mvmz_keys::enemyRecovery,       TranslateText::battleMessage},
            {mvmz_keys::escapeFailure,       TranslateText::battleMessage},
            {mvmz_keys::escapeStart,         TranslateText::battleMessage},
            {mvmz_keys::evasion,             TranslateText::battleMessage},
            {mvmz_keys::levelUp,             TranslateText::battleMessage},
            {mvmz_keys::magicEvasion,        TranslateText::battleMessage},
            {mvmz_keys::magicReflection,     TranslateText::battleMessage},
            {mvmz_keys::obtainExp,           TranslateText::battleMessage},
            {mvmz_keys::obtainGold,          TranslateText::battleMessage},
            {mvmz_keys::obtainItem,          TranslateText::battleMessage},
            {mvmz_keys::obtainSkill,         TranslateText::battleMessage},
            {mvmz_keys::preemptive,          TranslateText::battleMessage},
            {mvmz_keys::substitute,          TranslateText::battleMessage},
            {mvmz_keys::surprise,            TranslateText::battleMessage},
            {mvmz_keys::useItem,             TranslateText::battleMessage},
            {mvmz_keys::victory,             TranslateText::battleMessage},
            {mvmz_keys::note,                TranslateText::note},

            {mvmz_keys::partyName,           TranslateText::nameType},

            {mvmz_keys::alwaysDash,          TranslateText::other},
            {mvmz_keys::bgmVolume,           TranslateText::other},
            {mvmz_keys::bgsVolume,           TranslateText::other},
            {mvmz_keys::meVolume,            TranslateText::other},
            {mvmz_keys::seVolume,            TranslateText::other},
            {mvmz_keys::possession,          TranslateText::other},
            {mvmz_keys::saveMessage,         TranslateText::message},
            {mvmz_keys::loadMessage,         TranslateText::message},
            {mvmz_keys::file,                TranslateText::other},

            {mvmz_keys::expNext,             TranslateText::other},
            {mvmz_keys::expTotal,            TranslateText::other}
        };

		bool stackText;
		std::u8string stackTextStr;
        std::u8string currentEventName;
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
				if(code == mvmz_eventcode::ShowText)
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
            t.eventLabel = currentEventName;

            t.textType.emplace_back(textTypeForMaker);
            textTypeForMaker = TranslateText::other;

			auto result = std::find_if(texts.begin(), texts.end(), [&t](const auto& x){
				return x.original == t.original;
			});
			if(result == texts.end()){
				texts.emplace_back(std::move(t));
			}
            else if((code == mvmz_eventcode::ChangeActorName || code == mvmz_eventcode::ChangeActorNickname) && result->code == mvmz_eventcode::ShowChoices) {
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
                        case mvmz_eventcode::MessageParameter: [[fallthrough]]; //メッセージのパラメータ(401の前に来る)
						case mvmz_eventcode::ShowChoices: [[fallthrough]]; //選択肢
						case mvmz_eventcode::ShowText: [[fallthrough]]; //文章の表示
						case mvmz_eventcode::ShowScrollingText: [[fallthrough]]; //スクロールの文章の表示
						case mvmz_eventcode::ChangeActorImage: [[fallthrough]]; //アクター名の変更
						case mvmz_eventcode::ChangeActorName: [[fallthrough]]; //アクター名の変更
						case mvmz_eventcode::ChangeActorNickname: [[fallthrough]]; //二つ名の変更
						case mvmz_eventcode::ChangeActorProfile: //プロフィールの変更
							//case mvmz_eventcode::ShowImage: //画像の表示
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
		void convertArrayNode(const nlohmann::json& arr, int code, const AuxiliaryNodeInfo& auxInfo)
		{
			if(detectKeyInObject.find(auxInfo.key) != detectKeyInObject.end())
			{
				const auto& recursiveDetects = detectKeyInObject.at(auxInfo.key);
                int index = 0;
				for(auto s = arr.begin(); s != arr.end(); ++s)
				{
					if(s->is_array())
                    {
                        AuxiliaryNodeInfo auxInfo2 = auxInfo;
                        auxInfo2.type  = AuxiliaryNodeInfo::Array;
                        auxInfo2.key.clear();
                        auxInfo2.path += utility::cnvStr<std::u8string>(std::format("[{}]/", index));
                        auxInfo2.index = index;
                        auxInfo2.depth++;

						convertArrayNode(*s, code, auxInfo2);
						continue;
					}
					else if(s->is_object())
					{
                        AuxiliaryNodeInfo auxInfo2 = auxInfo;
                        auxInfo2.type  = AuxiliaryNodeInfo::Object;
                        auxInfo2.key.clear();
                        auxInfo2.path += utility::cnvStr<std::u8string>(std::format("[{}]/", index));
                        auxInfo2.index = index;
                        auxInfo2.depth++;

						convertNodeContainKey(auxInfo.key, *s, code, auxInfo2);
					}
					else if(s->is_string()){
						addText(*s, code);
					}
                    index++;
				}
			}
			else
			{
                int index = 0;
				for(auto s = arr.begin(); s != arr.end(); ++s)
				{
					if(s->is_array())
                    {
                        AuxiliaryNodeInfo auxInfo2 = auxInfo;
                        auxInfo2.type  = AuxiliaryNodeInfo::Array;
                        auxInfo2.key.clear();
                        auxInfo2.path += utility::cnvStr<std::u8string>(std::format("[{}]/", index));
                        auxInfo2.index = index;
                        auxInfo2.depth++;

						convertArrayNode(*s, code, auxInfo2);
					}
					else if(s->is_object())
                    {
                        AuxiliaryNodeInfo auxInfo2 = auxInfo;
                        auxInfo2.type  = AuxiliaryNodeInfo::Array;
                        auxInfo2.key.clear();
                        auxInfo2.path += utility::cnvStr<std::u8string>(std::format("[{}]/", index));
                        auxInfo2.index = index;
                        auxInfo2.depth++;

                        convertNodeContainKey(auxInfo.key, *s, code, auxInfo2);
					}
					else if(s->is_string())
					{
						addText(*s, code);
					}
                    index++;
				}
			}
		}

		void convertObjectNode(const nlohmann::json& root, int code, const AuxiliaryNodeInfo& auxInfo)
		{
			if(root.empty()){ return; }

			const auto& mainClassKeys = detectClassKeys.at(currentDataType);
			convertObjectNodeCore(mainClassKeys, root, code, auxInfo);
		}

		void convertNodeContainKey(std::u8string rootKey, const nlohmann::json& root, int code, const AuxiliaryNodeInfo& auxInfo)
		{
			if(root.empty()){ return; }

			if(currentDataType == DataType::System && rootKey == u8"messages"){
				spetializeSystemMessage(root);
				return;
			}

			//convertArrayNodeはキーがdetectKeyInObjectに無い場合もここへ来るため、
			//その場合は通常のオブジェクトとして走査する。
			const auto detectKeys = detectKeyInObject.find(rootKey);
			if(detectKeys == detectKeyInObject.end()){
				convertObjectNode(root, code, auxInfo);
				return;
			}

			convertObjectNodeCore(detectKeys->second, root, code, auxInfo);
		}

		void convertObjectNodeCore(const utility::u8stringlist& detectKeys, const nlohmann::json& node, int code, const AuxiliaryNodeInfo& auxInfo)
		{
            int index = -1;
			for(auto s = node.begin(); s != node.end(); ++s)
			{
                index++;
				if(s->is_null()){ continue; }
				auto key = utility::cnvStr<std::u8string>(s.key());

                if(key == u8"note")
                {
                    //noteは無条件で追加
                    textTypeForMaker = TranslateText::note;
                    addText(*s, code);
                    continue;
                }

				for(const auto& checkKey : detectKeys)
				{
					if(checkKey != key){ continue; }

					if(checkKey == u8"code")
                    {
                        std::u8string textTypeForMakerTmp;
						auto [isCheckParameters, _code] = checkEventCommandCode(node);
						code = _code;

                        if(code == mvmz_eventcode::MessageParameter)
                        {
                            //顔グラフィックが指定されているかのチェック。
                            //@parameter(valueは配列)の第一要素が空でなければ顔グラ有り。
                            for(auto s = node.begin(); s != node.end(); ++s)
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
                        else if(code == mvmz_eventcode::ScrollMessageParameter)
                        {
                            textTypeForMakerTmp = TranslateText::message;
                        }
                        else if(code == mvmz_eventcode::ChangeActorName || code == mvmz_eventcode::ChangeActorImage || code == mvmz_eventcode::ChangeActorNickname)
                        {
                            for(auto s = node.begin(); s != node.end(); ++s)
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
                        else if(code == mvmz_eventcode::ChangeActorProfile)
                        {
                            for(auto s = node.begin(); s != node.end(); ++s)
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

						if(stackText == false && (code == mvmz_eventcode::ShowText || code == mvmz_eventcode::ShowScrollingText)){
							stackText = true;
						}
						else if(stackText && (code != mvmz_eventcode::ShowText && code != mvmz_eventcode::ShowScrollingText)){
							stackText = false;
							addText(stackTextStr, mvmz_eventcode::ShowText);
							stackTextStr.clear();
                            //401 -> 401 -> MessageParameter等で先にtextTypeForMakerを設定すると
                            //先の文章のタイプが埋められてしまうためここで設定する。
                            if(textTypeForMakerTmp.empty() == false) {
                                textTypeForMaker = std::move(textTypeForMakerTmp);
                            }
						}

						if(isCheckParameters == false){
                            //401 -> 401 -> MessageParameter等で先にtextTypeForMakerを設定すると
                            //先の文章のタイプが埋められてしまうためここで設定する。
                            if(textTypeForMakerTmp.empty() == false) {
                                textTypeForMaker = std::move(textTypeForMakerTmp);
                            }
                            return; 
                        }
					}//if(checkKey == u8"code")

					const auto& val = s.value();
					if(detectKeyInObject.find(checkKey) != detectKeyInObject.end())
					{
						if(val.is_array()){

                            AuxiliaryNodeInfo auxInfo2 = auxInfo;
                            auxInfo2.type  = AuxiliaryNodeInfo::Array;
                            auxInfo2.key   = key;
                            auxInfo2.path += auxInfo2.key;
                            auxInfo2.index = index;
                            auxInfo2.depth++;

							convertArrayNode(val, code, auxInfo2);
						}
						else if(val.is_object())
                        {
                            AuxiliaryNodeInfo auxInfo2 = auxInfo;
                            auxInfo2.type = AuxiliaryNodeInfo::Object;
                            auxInfo2.key = key;
                            auxInfo2.path += auxInfo2.key + u8"/";
                            auxInfo2.index = index;
                            auxInfo2.depth++;

							convertNodeContainKey(key, val, code, auxInfo2);
						}
						continue;
					}

					if(val.is_array())
                    {
                        AuxiliaryNodeInfo auxInfo2 = auxInfo;
                        auxInfo2.type = AuxiliaryNodeInfo::Array;
                        auxInfo2.key = key;
                        auxInfo2.path += auxInfo2.key;
                        auxInfo2.index = index;
                        auxInfo2.depth++;

						convertArrayNode(val, code, auxInfo2);
						continue;
					}
					else if(val.is_object())
                    {

                        AuxiliaryNodeInfo auxInfo2 = auxInfo;
                        auxInfo2.type = AuxiliaryNodeInfo::Array;
                        auxInfo2.key = key;
                        auxInfo2.path += auxInfo2.key + u8"/";
                        auxInfo2.index = index;
                        auxInfo2.depth++;

						convertObjectNode(val, code, auxInfo2);
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