#include "platform_base.h"


std::filesystem::copy_options langscore::platform_base::convertCopyOption(MergeTextMode mode)
{
	switch(mode)
	{
		case MergeTextMode::Both:
			return std::filesystem::copy_options::overwrite_existing;
			break;
		case MergeTextMode::AcceptSource:
			return std::filesystem::copy_options::skip_existing;
			break;
		case MergeTextMode::MergeKeepSource:
			return std::filesystem::copy_options::overwrite_existing;
			break;
		case MergeTextMode::MergeKeepTarget:
			return std::filesystem::copy_options::overwrite_existing;
			break;
	}
	return std::filesystem::copy_options::none;
}
