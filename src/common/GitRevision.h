#ifndef GITREVISION_H_
#define GITREVISION_H_

#include "Define.h"

namespace GitRevision
{
	POOL_COMMON_API char const* GetHash();
	POOL_COMMON_API char const* GetDate();
	POOL_COMMON_API char const* GetBranch();
	POOL_COMMON_API char const* GetCMakeCommand();
	POOL_COMMON_API char const* GetCMakeVersion();
	POOL_COMMON_API char const* GetHostOSVersion();
	POOL_COMMON_API char const* GetBuildDirectory();
	POOL_COMMON_API char const* GetSourceDirectory();
	POOL_COMMON_API char const* GetMySQLExecutable();
	POOL_COMMON_API char const* GetFullVersion();
	POOL_COMMON_API char const* GetCompanyNameStr();
	POOL_COMMON_API char const* GetLegalCopyrightStr();
	POOL_COMMON_API char const* GetFileVersionStr();
	POOL_COMMON_API char const* GetProductVersionStr();
}

#endif