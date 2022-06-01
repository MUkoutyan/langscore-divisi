#include "platform_base.h"

std::filesystem::copy_options langscore::platform_base::convertCopyOption(OverwriteTextMode mode)
{
	switch(mode)
	{
		case OverwriteTextMode::Both:
			return std::filesystem::copy_options::overwrite_existing;
			break;
		case OverwriteTextMode::LeaveOld:
			return std::filesystem::copy_options::skip_existing;
			break;
		case OverwriteTextMode::LeaveOldNonBlank:
			return std::filesystem::copy_options::overwrite_existing;
			break;
		case OverwriteTextMode::OverwriteNew:
			return std::filesystem::copy_options::overwrite_existing;
			break;
	}
	return std::filesystem::copy_options::none;
}
