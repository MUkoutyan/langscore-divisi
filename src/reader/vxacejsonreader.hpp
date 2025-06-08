#pragma once

#include "jsonreader.hpp"

namespace langscore
{
	class vxace_jsonreader : public jsonreaderbase
	{
	public:
		vxace_jsonreader(std::vector<std::u8string> useLangs, nlohmann::json json)
			: jsonreaderbase(std::move(json))
			, stackText(false)
			, stackTextStr(u8"")
		{
            this->useLangList = std::move(useLangs);
			json2tt();
		}
		~vxace_jsonreader() override {}

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
	private:

		//共通して無視するキー
		const std::vector<std::u8string> ignoreKeys = {
			u8"class", u8"@note", u8"@character_name",
			u8"@self_switch_ch", u8"@switches", u8"@title1_name", u8"@variables"
		};
		//特定のクラスに存在する、無視するキー
		const std::map<std::u8string, std::vector<std::u8string>> ignoreForClassKeys = {
			{u8"RPG::UsableItem::Damage", {u8"@formula"}},
			{u8"RPG::Enemy",            {u8"@battler_name"}},
			{u8"RPG::Event",            {u8"@name"}},
			{u8"RPG::SE",               {u8"@name"}},
			{u8"RPG::ME",               {u8"@name"}},
			{u8"RPG::BGM",              {u8"@name"}},
			{u8"RPG::BGS",              {u8"@name"}},
			{u8"RPG::Map",              {u8"@battleback1_name", u8"@battleback2_name", u8"@parallax_name"}},
			{u8"RPG::Troop",            {u8"@name"}},
			{u8"RPG::Actor",            {u8"@face_name"}},
			{u8"RPG::System",           {u8"@battleback1_name", u8"@battleback2_name"} }
		};

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
            using namespace std::literals::string_literals;

			if(stackText){
				stackTextStr += text + u8'\n';
				return;
			}
			else 
            {
                if(code == 401) {
                    if(0 < text.size() && *(text.rbegin()) == u8'\n') {
                        text.erase((text.rbegin().base()) - 1);
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
		}

		std::tuple<std::u8string, bool> getObjectClass(const nlohmann::json& root)
		{
			bool hasSpecIgnoreKeys = false;
			std::u8string currentClassName = u8"";
			if(root.find("class") != root.end())
			{
				const std::string _currentClassName = root["class"];
				currentClassName = std::u8string(_currentClassName.begin(), _currentClassName.end());
				if(currentClassName != u8"")
				{
					hasSpecIgnoreKeys = ignoreForClassKeys.find(currentClassName) != ignoreForClassKeys.end();
				}
			}

			return std::forward_as_tuple(currentClassName, hasSpecIgnoreKeys);
		}
		bool checkIgnoreKey(const std::u8string& currentClassName, const std::u8string& key, bool hasSpecIgnoreKeys)
		{
			if(std::find(ignoreKeys.cbegin(), ignoreKeys.cend(), key) != ignoreKeys.cend()){ return true; }
			if(hasSpecIgnoreKeys){
				const auto& fields = ignoreForClassKeys.at(currentClassName);
				if(std::find(fields.cbegin(), fields.cend(), key) != fields.cend()){
					return true;
				}
			}
			return false;
		}

        //jsonのobj内に"@code"キーが存在し、かつそのコード値が検出するものかどうかをチェックする。
		std::tuple<bool, int> checkEventCommandCode(const nlohmann::json& obj)
		{
			bool result = false;
			int code = 0;
			for(auto s = obj.begin(); s != obj.end(); ++s)
			{
				if(s.key() == "@code"){
					//許可するコード
					s->get_to(code);
					switch(code){
                        case 101: [[fallthrough]];	//メッセージウィンドウの情報
						case 102: [[fallthrough]];	//選択肢
						case 401: [[fallthrough]];	//文章の表示
						case 405: [[fallthrough]];	//文章のスクロール表示
						case 320: [[fallthrough]];	//アクター名の変更
						case 324:		//二つ名の変更
                        case 325: //プロフィールの変更
						//case 231:		//画像の表示
							result = true;
							[[fallthrough]];
						default:
							break;
					}
				}

				if(result){ break; }
			}
			return std::forward_as_tuple(result, code);
		}
		void convertJArray(const nlohmann::json& arr, int code, std::u8string parentClass = u8"", std::u8string arrayinKey = u8"")
		{
			bool hasSpecIgnoreKeys = ignoreForClassKeys.find(parentClass) != ignoreForClassKeys.end();
			for(auto s = arr.begin(); s != arr.end(); ++s)
			{
				if(s->is_array()){
					convertJArray(*s, code, parentClass, arrayinKey);
					continue;
				}
				else if(s->is_object()){
					convertJsonObject(*s, code);
				}
				else if(s->is_string())
				{
					if(checkIgnoreKey(parentClass, arrayinKey, hasSpecIgnoreKeys)){ continue; }
                    
                    if(parentClass == u8"RPG::EventCommand")
                    {
                        if(code == 101)
                        {
                            //顔グラフィックが指定されているかのチェック。
                            //@parameter(valueは配列)の第一要素が空でなければ顔グラ有り。
                            std::string valStr;
                            s->get_to(valStr);
                            if(valStr.empty() == false) {
                                //メッセージウィンドウに顔グラフィック指定がある。
                                textTypeForMaker = TranslateText::messageWithGraphic;
                            }
                            else {
                                textTypeForMaker = TranslateText::message;
                            }
                        }
                        else {
                            addText(*s, code);
                        }
                    }
                    else {
                        addText(*s, code);
                    }
				}
			}
		}
		void convertJsonObject(const nlohmann::json& root, int parent_code)
		{
			if(root.empty()){ return; }

			auto [currentClassName, hasSpecIgnoreKeys] = getObjectClass(root);

			auto [acceptCommand, code] = checkEventCommandCode(root);
			if(code == 0){ code = parent_code; }
			if(currentClassName == u8"RPG::EventCommand")
            {
                if(code == 101) 
                {
                    //顔グラフィックが指定されているかのチェック。
                    //@parameter(valueは配列)の第一要素が空でなければ顔グラ有り。
                    for(auto s = root.begin(); s != root.end(); ++s)
                    {
                        if(s.key() != "@parameter") {
                            continue;
                        }
                        nlohmann::json arrayData;
                        s->get_to(arrayData);
                        if(1 < arrayData.size()) {
                            std::string faceGraphicName;
                            arrayData[0].get_to(faceGraphicName);
                            if(faceGraphicName.empty()) {
                                textTypeForMaker = TranslateText::message;
                            }
                            else {
                                textTypeForMaker = TranslateText::messageWithGraphic;
                            }
                            break;
                        }
                    }
                }
                else if(code == 320)
                {
                    for(auto s = root.begin(); s != root.end(); ++s)
                    {
                        if(s.key() != "@parameters") {
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
				if(stackText == false && (code == 401 || code == 405)){
					stackText = true;
				}
				else if(stackText && (code != 401 && code != 405)){
					stackText = false;
                    if(textTypeForMaker.empty()) { textTypeForMaker = TranslateText::message; }
					addText(stackTextStr, 401);
					stackTextStr.clear();
				}
				if(acceptCommand == false){ return; }
			}

			for(auto s = root.begin(); s != root.end(); ++s)
			{
				const auto& _key = s.key();
				std::u8string key(_key.begin(), _key.end());
				const auto& val = s.value();
				if(val.is_array()){
					convertJArray(val, code, currentClassName, key);
					continue;
				}
				else if(val.is_object()){
					convertJsonObject(val, code);
					continue;
				}
				else if(val.is_string() == false){ continue; }

				if(checkIgnoreKey(currentClassName, key, hasSpecIgnoreKeys)){
					continue;
				}

                if(currentClassName == u8"RPG::State" || currentClassName == u8"RPG::Skill") {
                    if(key.find(u8"@message") != std::u8string::npos) {
                        textTypeForMaker = TranslateText::battleMessage;
                    }
                    else if(key == u8"@name") {
                        textTypeForMaker = TranslateText::battleName;
                    }
                    else if(key == u8"@description") {
                        textTypeForMaker = TranslateText::descriptionType;
                    }
                }
                else {
                    if(key == u8"@name") {
                        textTypeForMaker = TranslateText::nameType;
                    }
                    else if(key == u8"@nickname") {
                        textTypeForMaker = TranslateText::nameType;
                    }
                    else if(key == u8"@description") {
                        textTypeForMaker = TranslateText::descriptionType;
                    }
                }

				addText(*s, code);
			}

		}


		bool stackText;
        std::u8string textTypeForMaker = TranslateText::other;
		std::u8string stackTextStr;

	};
}