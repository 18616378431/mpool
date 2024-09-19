#ifndef MYSQL_THREADING_H_
#define MYSQL_THREADING_H_

#include "Define.h"

namespace MySQL
{
	POOL_DATABASE_API void Library_Init();
	POOL_DATABASE_API void Library_End();
	POOL_DATABASE_API uint32 GetLibraryVersion();
}

#endif