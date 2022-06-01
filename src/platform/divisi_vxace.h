#pragma once

#include "platform_base.h"

namespace langscore
{
	class divisi_vxace: public platform_base
	{
	public:
		divisi_vxace();
		~divisi_vxace() override;

		void setProjectPath(std::filesystem::path path) override;
		void prepareAnalyzeProject() override;

		void copyData(langscore::OverwriteTextMode option = langscore::OverwriteTextMode::LeaveOld) override;
		void convertGraphFileNameData() override;
		void convert() override;

		void setIgnoreScriptPath(utility::filelist ignoreScriptPath) override;

	private:
		void convertRvData();
		void convertRvScript();
		utility::u8stringlist formatSystemVariable(std::filesystem::path path);

		std::filesystem::path outputProjectDataPath(std::filesystem::path fileName, std::filesystem::path dir = "") override;

	};
}