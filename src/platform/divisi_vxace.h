#pragma once

#include "platform_base.h"

namespace langscore
{
	class divisi_vxace: public platform_base
	{
	public:
		using platform_base::platform_base;
		~divisi_vxace() override;

		void setProjectPath(std::filesystem::path path) override;
		void prepareAnalyzeProject() override;

		void copyData(std::filesystem::copy_options option) override;
		void convertGraphFileNameData() override;
		void convert(std::filesystem::copy_options option = std::filesystem::copy_options::none) override;

		void setIgnoreScriptPath(utility::filelist ignoreScriptPath) override;

	private:
		void convertRvData();
		void convertRvScript();

		std::filesystem::path outputProjectDataPath(std::filesystem::path fileName, std::filesystem::path dir = "Data") override;

	};
}