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
		ErrorStatus analyze() override;
		ErrorStatus write() override;

		//void copyData(langscore::OverwriteTextMode option = langscore::OverwriteTextMode::LeaveOld) override;

	private:
		void fetchFilePathList();

		void writeAnalyzedData();
		void writeAnalyzedRvScript();

		void writeFixedData();
		void writeFixedRvScript();
		void writeFixedGraphFileNameData();

		void rewriteScriptList();

		utility::u8stringlist formatSystemVariable(std::filesystem::path path);
		
		std::filesystem::path exportFolderPath(std::filesystem::path fileName, std::filesystem::path dir = "") override;

	};
}