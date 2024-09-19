#include "Errors.h"
#include "Duration.h"

#include <cstdio>
#include <cstdlib>
#include <thread>

#if POOL_PLATFORM == POOL_PLATFORM_WINDOWS
#include <Windows.h>
#define Crash(message) \
	ULONG_PTR execeptionArgs[] = { reinterpret_cast<ULONG_PTR>(strdup(message)), reinterpret_cast<ULONG_PTR>(_ReturnAddress()) }; \
	RaiseException(EXCEPTION_ASSERTION_FAILURE, 0, 2, execeptionArgs);
#else
extern "C" { char const* MpoolAssertionFailedMessage = nullptr; }
#define Crash(message) \
	MpoolAssertionFailedMessage = strdup(message); \
	*((volatile int*)nullptr) = 0; \
	exit(1);
#endif

namespace
{
	inline std::string MakeMessage(std::string_view messageType, std::string_view file, uint32 line, std::string_view function,
		std::string_view message, std::string_view fmtMessage = {}, std::string_view debugInfo = {})
	{
		std::string msg = mpool::StringFormatFmt("\n>> {}\n\n# Location '{}:{}'\n# Function '{}'\n# Condition '{}'\n", messageType, file, line, function, message);
		
		if (!fmtMessage.empty())
		{
			msg.append(mpool::StringFormatFmt("# Message '{}'\n", fmtMessage));
		}

		if (!debugInfo.empty())
		{
			msg.append(mpool::StringFormatFmt("\n# Debug info: '{}'\n", debugInfo));
		}

		return mpool::StringFormatFmt(
			"#{0:-^{2}}#\n"
			" {1: ^{2}} \n"
			"#{0:-^{2}}#\n", "", msg, 70);
	}

	inline std::string MakeAbortMessage(std::string_view file, uint32 line, std::string_view function, std::string_view fmtMessage = {})
	{
		std::string msg = mpool::StringFormatFmt("\n>> ABORTED\n\n# Location '{}:{}'\n# Function '{}'\n", file, line, function);

		if (!fmtMessage.empty())
		{
			msg.append(mpool::StringFormatFmt("# Message '{}'\n", fmtMessage));
		}

		return mpool::StringFormatFmt(
			"\n#{0:-^{2}}#\n"
			" {1: ^{2}} \n"
			"#{0:-^{2}}#\n", "", msg, 70);
	}
}

void mpool::Assert(std::string_view file, uint32 line, std::string_view function, std::string_view debugInfo, std::string_view message, std::string_view fmtMessage /* = */)
{
	std::string formattedMessage = MakeMessage("ASSERTION FAILED", file, line, function, message, fmtMessage, debugInfo);
	fmt::print(stderr, "{}", formattedMessage);
	fflush(stderr);
	Crash(formattedMessage.c_str());
}

void mpool::Fatal(std::string_view file, uint32 line, std::string_view function, std::string_view message, std::string_view fmtMessage /* = */)
{
	std::string formattedMessage = MakeMessage("FATAL ERROR", file, line, function, message, fmtMessage);
	fmt::print(stderr, "{}", formattedMessage);
	fflush(stderr);
	std::this_thread::sleep_for(10s);
	Crash(formattedMessage.c_str());
}

void mpool::Error(std::string_view file, uint32 line, std::string_view function, std::string_view message)
{
	std::string formattedMessage = MakeMessage("ERROR", file, line, function, message);
	fmt::print(stderr, "{}", formattedMessage);
	fflush(stderr);
	std::this_thread::sleep_for(10s);
	Crash(formattedMessage.c_str());
}

void mpool::Warning(std::string_view file, uint32 line, std::string_view function, std::string_view message)
{
	std::string formattedMessage = MakeMessage("WARNING", file, line, function, message);
	fmt::print(stderr, "{}", formattedMessage);
}

void mpool::Abort(std::string_view file, uint32 line, std::string_view function, std::string_view fmtMessage /* = */)
{
	std::string formattedMessage = MakeAbortMessage(file, line, function, fmtMessage);
	fmt::print(stderr, "{}", formattedMessage);
	fflush(stderr);
	std::this_thread::sleep_for(10s);
	Crash(formattedMessage.c_str());
}

void mpool::AbortHandler(int sigval)
{
	std::string formattedMessage = StringFormatFmt("Caught signal {}\n", sigval);
	fmt::print(stderr, "{}", formattedMessage);
	fflush(stderr);
	Crash(formattedMessage.c_str());
}

std::string GetDebugInfo()
{
	return "";
}