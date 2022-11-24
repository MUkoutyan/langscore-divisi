#pragma once

#include "jsonreader.hpp"

namespace langscore
{
	class mvmz_jsonreader: public jsonreaderbase
	{
	public:
		mvmz_jsonreader(const nlohmann::json& json)
			: jsonreaderbase(json)
			, stackText(false)
			, stackTextStr(u8"")
		{
			json2tt();
		}
		~mvmz_jsonreader() override {}

	private:
		void json2tt() override
		{
			if(json.is_array())
			{
				convertJArray(json);
			}
			else if(json.is_object())
			{
				convertJObject(json);
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
		};

		void addText(const nlohmann::json& json, std::u8string note = u8"")
		{
			std::string valStr;
			json.get_to(valStr);

			//手動の中央揃え等で空行を使用する場合があるため、空文字の検出は行わない。
			//if(valStr.empty()){ return; }

			std::u8string original(valStr.begin(), valStr.end());
			addText(std::move(original), std::move(note));
		}
		void addText(std::u8string text, std::u8string note = u8"")
		{
			if(stackText){
				//1行に付き必ず改行が挟まる。(VXAceのみの仕様？MV/MZは要確認)
				stackTextStr += text + u8'\n';
				return;
			}
			else {
				if(text.empty()){
					//ただの空文は無視する。
					return;
				}
			}

			bool wrapDq = false;
			if(text.find(u8'\n') != std::u8string::npos){
				wrapDq = true;
			}
			if(text.find(u8'\"') != std::u8string::npos){
				wrapDq = true;
				text = utility::replace<std::u8string>(text, u8"\"", u8"\"\"");
			}

			if(wrapDq){
				text = u8"\"" + text + u8"\"";
			}

			TranslateText t(std::move(text), useLangs);
			auto result = std::find_if(texts.begin(), texts.end(), [&t](const auto& x){
				return x.original == t.original;
			});

			if(result == texts.end()){
				texts.emplace_back(std::move(t));
			}
		}

		std::tuple<std::u8string, bool> getObjectClass(const nlohmann::json& root)
		{
			bool hasSpecIgnoreKeys = false;
			std::u8string currentObjectKey = u8"";

			if(root.find("class") != root.end())
			{
				const std::string _currentObjectKey = root["class"];
				currentObjectKey = std::u8string(_currentObjectKey.begin(), _currentObjectKey.end());
				if(currentObjectKey != u8"")
				{
					hasSpecIgnoreKeys = detectKeyInObject.find(currentObjectKey) != detectKeyInObject.end();
				}
			}

			return std::forward_as_tuple(currentObjectKey, hasSpecIgnoreKeys);
		}
		bool checkIgnoreKey(const std::u8string& currentObjectKey, const std::u8string& key, bool hasSpecIgnoreKeys)
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
							//case 231: //画像の表示
							result = true;
						default:
							break;
					}
				}

				if(result){ break; }
			}
			return std::forward_as_tuple(result, code);
		}
		void convertJArray(const nlohmann::json& arr, std::u8string parentClass = u8"", std::u8string arrayinKey = u8"")
		{
			bool hasSpecIgnoreKeys = detectKeyInObject.find(parentClass) != detectKeyInObject.end();
			for(auto s = arr.begin(); s != arr.end(); ++s)
			{
				if(s->is_array()){
					convertJArray(*s, parentClass, arrayinKey);
					continue;
				}
				else if(s->is_object()){
					convertJObject(*s);
				}
				else if(s->is_string())
				{
					if(checkIgnoreKey(parentClass, arrayinKey, hasSpecIgnoreKeys)){ continue; }
					addText(*s, parentClass + u8"=>" + arrayinKey);
				}
			}
		}
		void convertJObject(const nlohmann::json& root)
		{
			if(root.empty()){ return; }

			auto [currentObjectKey, hasSpecIgnoreKeys] = getObjectClass(root);

			if(currentObjectKey == u8"list"){
				auto [result, code] = checkEventCommandCode(root);
				if(stackText == false && code == 401){
					stackText = true;
				}
				else if(stackText && code != 401){
					stackText = false;
					addText(stackTextStr);
					stackTextStr.clear();
				}
				if(result == false){ return; }
			}

			for(auto s = root.begin(); s != root.end(); ++s)
			{
				const auto& _key = s.key();
				std::u8string key(_key.begin(), _key.end());
				const auto& val = s.value();
				if(val.is_array()){
					convertJArray(val, currentObjectKey, key);
					continue;
				}
				else if(val.is_object()){
					convertJObject(val);
					continue;
				}
				else if(val.is_string() == false){ continue; }

				if(checkIgnoreKey(currentObjectKey, key, hasSpecIgnoreKeys)){
					continue;
				}

				addText(*s, currentObjectKey + u8":" + key);
			}

		}


		bool stackText;
		std::u8string stackTextStr;

	};
}