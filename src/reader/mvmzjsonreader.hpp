#pragma once

#include "jsonreader.hpp"
#include <filesystem>

namespace langscore
{
	class mvmz_jsonreader: public jsonreaderbase
	{
	public:
		mvmz_jsonreader(const std::filesystem::path& path, std::vector<std::u8string> useLangs, const nlohmann::json& json)
			: jsonreaderbase(std::move(useLangs), json)
			, stackText(false)
			, stackTextStr(u8"")
			, currentDataType(DetectDataType(path))
		{
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
				convertJObject(json, 0);
			}
		}

		enum class DataType
		{
			Actors, Armors, Classes, CommonEvents, Enemies, Items, Map, 
			Skills, States, System, Troops, Weapons
		};

		//特定のデータタイプで検出するキー
		const std::map<DataType, std::vector<std::u8string>> detectClassKeys = {
			{DataType::Actors,			{u8"name", u8"nickname", u8"profile"}},
			{DataType::Armors,			{u8"name", u8"description"}},
			{DataType::Classes,			{u8"name"}},
			//CommonEventsはlist内のオブジェクトに対してcodeの値を調べる必要がある。
			{DataType::CommonEvents,    {u8"list"}},
			{DataType::Enemies,			{u8"name"}},
			{DataType::Items,			{u8"name", u8"description"}},
			{DataType::Map,				{u8"events"}},
			{DataType::Skills,			{u8"name", u8"description", u8"message1", u8"message2"}},
			{DataType::States,			{u8"name", u8"description", u8"message1", u8"message2", u8"message3", u8"message4"}},
			{DataType::System,			{u8"armorTypes", u8"currencyUnit", u8"elements", u8"equipTypes", 
										 u8"gameTitle", u8"skillTypes", u8"terms", u8"weaponTypes"}},
			{DataType::Troops,			{u8"pages"}},
			{DataType::Weapons,			{u8"name", u8"description"}},
		};
		//データタイプ内で検出するキー detectClassKeysで検出されたキーは、この中に含まれているかをまず調べる。
		//検出した場合はこの中のキーを検出する。これはdetectKeyInObjectの値も、再帰的にdetectKeyInObjectで検索されなければいけない。
		const std::map<std::u8string, std::vector<std::u8string>> detectKeyInObject = {
			{u8"events",		{u8"pages"}},
			{u8"pages",			{u8"list"}},
			{u8"list",			{u8"code", u8"parameters"}},
			{u8"terms",			{u8"basic", u8"commands", u8"params", u8"messages"}},
			{u8"messages",		{u8"actionFailure", u8"actorDamage", u8"actorDrain", u8"actorGain", 
								 u8"actorLoss", u8"actorNoDamage", u8"actorNoHit", u8"actorRecovery", 
								 u8"alwaysDash", u8"bgmVolume", u8"bgsVolume", u8"buffAdd", u8"buffRemove", 
								 u8"commandRemember", u8"counterAttack", u8"criticalToActor", u8"criticalToEnemy", 
								 u8"debuffAdd", u8"defeat", u8"emerge", u8"enemyDamage", u8"enemyDrain", 
								 u8"enemyGain", u8"enemyLoss", u8"enemyNoDamage", u8"enemyNoHit", 
								 u8"enemyRecovery", u8"escapeFailure", u8"escapeStart", u8"evasion", 
								 u8"expNext", u8"expTotal", u8"file", u8"levelUp", u8"loadMessage", 
								 u8"magicEvasion", u8"magicReflection", u8"meVolume", u8"obtainExp", 
								 u8"obtainGold", u8"obtainItem", u8"obtainSkill", u8"partyName", u8"possession", 
								 u8"preemptive", u8"saveMessage", u8"seVolume", u8"substitute", u8"surprise",
								 u8"useItem", u8"victory"}}
		};

		bool stackText;
		std::u8string stackTextStr;
		DataType currentDataType;

		void addText(const nlohmann::json& json, int code = 0)
		{
			std::string valStr;
			json.get_to(valStr);

			//手動の中央揃え等で空行を使用する場合があるため、空文字の検出は行わない。
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
				if(text.empty()){
					//ただの空文は無視する。
					return;
				}
				if(code == 401){
					if(*(text.rbegin()) == u8'\n'){
						text.erase((text.rbegin().base())-1);
					}
				}
			}

			//※リード処理はCSV用のテキストに変換しない。

			TranslateText t(std::move(text), useLangList);
			t.code = code;
			auto result = std::find_if(texts.begin(), texts.end(), [&t](const auto& x){
				return x.original == t.original;
			});

			if(result == texts.end()){
				texts.emplace_back(std::move(t));
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
		bool checkIgnoreKey(const std::u8string& currentObjectKey, const std::u8string& key)
		{
			//if(std::find(ignoreKeys.cbegin(), ignoreKeys.cend(), key) != ignoreKeys.cend()){ return true; }
			//if(hasSpecIgnoreKeys){
			//	const auto& fields = detectClassKeys.at(currentObjectKey);
			//	if(std::find(fields.cbegin(), fields.cend(), key) != fields.cend()){
			//		return true;
			//	}
			//}
			return false;
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
						case 102: //選択肢
						case 401: //文章の表示
						case 405: //スクロールの文章の表示
						case 320: //アクター名の変更
						case 324: //二つ名の変更
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
						convertJObject(*s, code);
					}
					else if(s->is_string())
					{
						addText(*s, code);
					}
				}
			}
		}

		void convertJObject(const nlohmann::json& root, int code)
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

					if(checkKey == u8"code"){
						auto [result, _code] = checkEventCommandCode(root);
						code = _code;
						if(stackText == false && (code == 401 || code == 405)){
							stackText = true;
						}
						else if(stackText && (code != 401 && code != 405)){
							stackText = false;
							addText(stackTextStr, 401);
							stackTextStr.clear();
						}
						if(result == false){ return; }
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
						convertJObject(val, code);
						continue;
					}
					else if(val.is_string() == false){ continue; }

					if(checkIgnoreKey(key, key)){
						continue;
					}

					addText(*s, code);
				}
			}
		}

		void spetializeSystemMessage(const nlohmann::json& root)
		{
			for(auto s = root.begin(); s != root.end(); ++s)
			{
				if(s->is_null()){ continue; }
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

	};
}