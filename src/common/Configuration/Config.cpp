#include <fstream>
#include <mutex>
#include <unordered_map>
#include <iostream>

#include "Config.h"
#include "StringFormat.h"
#include "StringConvert.h"
#include "Tokenize.h"

namespace 
{
	std::string _filename;
	std::vector<std::string> _additionalFiles;
	std::vector<std::string> _args;
	std::unordered_map<std::string, std::string> _configOptions;
	std::mutex _configLock;
	bool _usingDistConfig = false;

	bool IsAppConfig(std::string_view fileName)
	{
		size_t foundAuth = fileName.find("logon.conf");
		return foundAuth != std::string_view::npos;
	}

	bool IsLoggingSystemOptions(std::string_view optionName)
	{
		size_t foundAppender = optionName.find("Appender.");
		size_t foundLogger = optionName.find("Logger.");

		return foundAppender != std::string_view::npos || foundLogger != std::string_view::npos;
	}

	template<typename Format, typename... Args>
	inline void PrintError(std::string_view filename, Format&& fmt, Args&& ... args)
	{
		std::string message = mpool::StringFormatFmt(std::forward<Format>(fmt), std::forward<Args>(args)...);
		
		if (IsAppConfig(filename))
		{
			fmt::print("{}\n", message);
		}
		else 
		{
			std::cout << "server.loading" << message << std::endl;
		}
	}

	void AddKey(std::string const& optionName, std::string const& optionKey, std::string_view fileName, bool isOptional, [[maybe_unused]] bool isReload)
	{
		auto const& itr = _configOptions.find(optionName);

		if (isOptional && itr == _configOptions.end())
		{
			if (!IsLoggingSystemOptions(optionName) && !isReload) 
			{
				PrintError(fileName, ">Config::LoadFile:Found incorrect option '{}' in config file '{}'.Skip", optionName, fileName);
			}

			return ;
		}

		if (itr != _configOptions.end())
		{
			_configOptions.erase(optionName);
		}

		_configOptions.emplace(optionName, optionKey);
	}

	bool ParseFile(std::string const& file, bool isOptional, bool isReload)
	{
		std::ifstream in(file);

		if (in.fail()) 
		{
			if (isOptional)
			{
				return false;
			}
			throw ConfigException(mpool::StringFormatFmt("Config::LoadFile: Failed open {} file '{}'", isOptional ? "optional" : "", file));
		}

		uint32 count = 0;
		uint32 lineNumber = 0;
		std::unordered_map <std::string, std::string> fileConfigs;

		auto IsDuplicateOption = [&](std::string const& confOption)
		{
			auto const& itr = fileConfigs.find(confOption);
			if (itr != fileConfigs.end())
			{
				PrintError(file, "Config::LoadFile: Dublicate key name '{}' in config file '{}'", confOption, file);
				return true;
			}

			return false;
		};

		while (in.good())
		{
			lineNumber++;
			std::string line;
			std::getline(in, line);

			if (!in.good() && !in.eof())
			{
				throw ConfigException(mpool::StringFormatFmt("Config::LoadFile: Failure  to read line number {} in file '{}'", lineNumber, file));
			}

			line = mpool::String::Trim(line, in.getloc());

			if (line.empty())
			{
				continue;
			}

			if (line[0] == '#' || line[0] == '[')
			{
				continue;
			}

			size_t found = line.find_first_of('#');
			if (found != std::string::npos)
			{
				line = line.substr(0, found);
			}

			auto const equal_pos = line.find('=');

			if (equal_pos == std::string::npos || equal_pos == line.length())
			{
				PrintError(file, "Config::LoadFile: Failure to read line number {} in file '{}'.Skip this line", lineNumber, file);
				continue;
			}

			auto entry = mpool::String::Trim(line.substr(0, equal_pos), in.getloc());
			auto value = mpool::String::Trim(line.substr(equal_pos + 1, std::string::npos), in.getloc());
			
			value.erase(std::remove(value.begin(), value.end(), '"'), value.end());

			if (IsDuplicateOption(entry))
			{
				continue;
			}

			fileConfigs.emplace(entry, value);
			count++;
		}

		if (!count)
		{
			if (isOptional)
			{
				return false;
			}

			throw ConfigException(mpool::StringFormatFmt("Config::LoadFile: empty file '{}'", file));
		}

		for (auto const& [entry, key] : fileConfigs)
		{
			AddKey(entry, key, file, isOptional, isReload);
		}

		return true;
	}

	bool LoadFile(std::string const& file, bool isOptional, bool isReload)
	{
		try
		{
			return ParseFile(file, isOptional, isReload);
		}
		catch (const std::exception& e)
		{
			PrintError(file, "{}", e.what());
		}

		return false;
	}
}

bool ConfigMgr::LoadInitial(std::string const& file, bool isReload)
{
	std::lock_guard<std::mutex> lock(_configLock);
	_configOptions.clear();
	return LoadFile(file, false, isReload);
}

bool ConfigMgr::LoadAdditionalFile(std::string file, bool isOptional, bool isReload)
{
	std::lock_guard<std::mutex> lock(_configLock);
	return LoadFile(file, isOptional, isReload);
}

ConfigMgr* ConfigMgr::instance()
{
	static ConfigMgr instance;
	return &instance;
}

bool ConfigMgr::Reload()
{
	//if (!LoadAppConfigs(true))
	//{
	//	return false;
	//}

	//return LoadModulesConfigs(true, false);

	return false;
}

template<typename T>
T ConfigMgr::GetValueDefault(std::string const& name, T const& def, bool showLogs) const
{
	auto const& itr = _configOptions.find(name);
	if (itr == _configOptions.end())
	{
		if (showLogs)
		{
			std::cout << mpool::StringFormatFmt("server loading, Config: Missing property {} in all config files, at least the .dist file must contain: \"{} = {}\"",
				name, name, mpool::ToString(def)) << std::endl;
		}

		return def;
	}

	auto value = mpool::StringTo<T>(itr->second);
	if (!value)
	{
		if (showLogs)
		{
			std::cout << mpool::StringFormatFmt("server loading, Config: Bad value defined for name '{}', going to use '{}' instead",
				name, mpool::ToString(def)) << std::endl;
		}

		return def;
	}

	return *value;
}

template<>
std::string ConfigMgr::GetValueDefault<std::string>(std::string const& name, std::string const& def, bool showLogs) const
{
	auto const& itr = _configOptions.find(name);
	if (itr == _configOptions.end())
	{
		if (showLogs)
		{
			std::cout << mpool::StringFormatFmt("server loading, config::Missing option {}, add \"{} = {}\"",
				name, name, def) << std::endl;
		}
		return def;
	}

	return itr->second;
}

template<typename T>
T ConfigMgr::GetOption(std::string const& name, T const& def, bool showLogs) const
{
	return GetValueDefault<T>(name, def, showLogs);
}

template<>
bool ConfigMgr::GetOption<bool>(std::string const& name, bool const& def, bool showLogs) const
{
	std::string val = GetValueDefault(name, std::string(def ? "1" : "0"), showLogs);
	
	auto boolVal = mpool::StringTo<bool>(val);
	if (!boolVal)
	{
		if (showLogs)
		{
			std::cout << mpool::StringFormatFmt("server loading, Config: bad value defined for name '{}', going to use '{}' instead",
				name, def ? "true" : "false") << std::endl;
		}

		return def;
	}

	return *boolVal;
}

std::vector<std::string> ConfigMgr::GetKeysByString(std::string const& name)
{
	std::lock_guard<std::mutex> lock(_configLock);
	
	std::vector <std::string> keys;

	for (auto const& [optionName, key] : _configOptions)
	{
		if (!optionName.compare(0, name.length(), name))
		{
			keys.emplace_back(optionName);
		}
	}

	return keys;
}

std::string const ConfigMgr::GetFilename()
{
	std::lock_guard<std::mutex> lock(_configLock);
	return _usingDistConfig ? _filename + ".dist" : _filename;
}

std::vector<std::string> const& ConfigMgr::GetArguments() const
{
	return _args;
}

std::string const ConfigMgr::GetConfigPath()
{
	std::lock_guard<std::mutex> lock(_configLock);

#if POOL_PLATFORM == POOL_PLATFORM_WINDOWS
	return "configs/";
#else
	return std::string(_CONF_DIR) + "/";
#endif
}

void ConfigMgr::Configure(std::string const& initFileName, std::vector<std::string> args, std::string_view modulesConfigList)
{
	_filename = initFileName;
	_args = std::move(args);

	if (!modulesConfigList.empty())
	{
		for (auto const& itr : mpool::Tokenize(modulesConfigList, ',', false))
		{
			_additionalFiles.emplace_back(itr);
		}
	}
}

bool ConfigMgr::LoadAppConfigs(bool isReload)
{
	//#1 Load init config file .conf.dist
	if (!LoadInitial(_filename + ".dist", isReload))
	{
		return false;
	}

	if (!LoadAdditionalFile(_filename, true, isReload))
	{
		_usingDistConfig = true;
	}

	return true;
}

bool ConfigMgr::LoadModulesConfigs(bool isReload, bool isNeedPrintInfo)
{
	if (_additionalFiles.empty())
	{
		return true;
	}

	if (isNeedPrintInfo)
	{
		std::cout << "server loading" << std::endl;
		std::cout << "server.loading Loading modules configuration..." << std::endl;
	}

	//start loading modules
	std::string const& moduleConfigPath = GetConfigPath() + "modules/";
	bool isExistDefaultConfig = true;
	bool isExistDistConfig = true;

	for (auto const& distFileName : _additionalFiles)
	{
		std::string defaultFileName = distFileName;

		if (!defaultFileName.empty())
		{
			//remove suffix .dist
			defaultFileName.erase(defaultFileName.end() - 5, defaultFileName.end());
		}

		//load .conf.dist config
		isExistDistConfig = LoadAdditionalFile(moduleConfigPath + distFileName, false, isReload);

		if (!isReload && !isExistDistConfig)
		{
			std::cout << mpool::StringFormatFmt("server loading, Config::LoadModulesConfigs: Not found original config '{}'. stop loading", distFileName)
				<< std::endl;
			
			return false;
		}

		//load .conf config
		isExistDefaultConfig = LoadAdditionalFile(moduleConfigPath + defaultFileName, true, isReload);

		if (isExistDefaultConfig && isExistDistConfig)
		{
			_moduleConfigFiles.emplace_back(defaultFileName);
		}
		else if (!isExistDefaultConfig && isExistDistConfig)
		{
			_moduleConfigFiles.emplace_back(distFileName);
		}
	}

	if (isNeedPrintInfo)
	{
		if (!_moduleConfigFiles.empty())
		{
			std::cout << "server loading.Using modules configuration:" << std::endl;

			for (auto const& itr : _moduleConfigFiles)
			{
				std::cout << mpool::StringFormatFmt("server loading.{}", itr) << std::endl;
			}
		}
		else
		{
			std::cout << "server loading.Not found modules config files" << std::endl;
		}
	}

	if (isNeedPrintInfo)
	{
		std::cout << "modules config load end" << std::endl;
	}

	return true;
}

//deprecated methods
std::string ConfigMgr::GetStringDefault(std::string const& name, const std::string& def, bool showLogs)
{
	return GetOption<std::string>(name, def, showLogs);
}

bool ConfigMgr::GetBoolDefault(std::string const& name, bool def, bool showLogs)
{
	return GetOption<bool>(name, def, showLogs);
}

int ConfigMgr::GetIntDefault(std::string const& name, int def, bool showLogs)
{
	return GetOption<int32>(name, def, showLogs);
}

float ConfigMgr::GetFloatDefault(std::string const& name, float def, bool showLogs)
{
	return GetOption<float>(name, def, showLogs);
}

#define TEMPLATE_CONFIG_OPTION(__typename) \
	template __typename ConfigMgr::GetOption<__typename>(std::string const& name, __typename const& def, bool showLogs) const;

TEMPLATE_CONFIG_OPTION(std::string)
TEMPLATE_CONFIG_OPTION(uint8)
TEMPLATE_CONFIG_OPTION(int8)
TEMPLATE_CONFIG_OPTION(uint16)
TEMPLATE_CONFIG_OPTION(int16)
TEMPLATE_CONFIG_OPTION(uint32)
TEMPLATE_CONFIG_OPTION(int32)
TEMPLATE_CONFIG_OPTION(uint64)
TEMPLATE_CONFIG_OPTION(int64)
TEMPLATE_CONFIG_OPTION(float)

#undef TEMPLATE_CONFIG_OPTION
