#include "Banner.h"
#include "GitRevision.h"
#include "StringFormat.h"

void mpool::Banner::Show(std::string_view applicationName, void (*log)(std::string_view text), void (*logExtraInfo)())
{

	log(mpool::StringFormatFmt("{} ({})", GitRevision::GetFullVersion(), applicationName));
	log("<Ctrl-C> to stop.");
	log("-----------");
	log("Hello From mpool, mpool writen in c++17");
	log("-----------");

	if (logExtraInfo)
	{
		logExtraInfo();
	}

	log(" ");
}