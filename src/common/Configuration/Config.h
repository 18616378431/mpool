#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdexcept>
#include <string_view>
#include <vector>

#include "Define.h"

//配置管理
class ConfigMgr 
{
	//private
	ConfigMgr() = default;
	ConfigMgr(ConfigMgr const&) = delete;
	ConfigMgr& operator=(ConfigMgr const &) = delete;
	~ConfigMgr() = default;
public:
	bool LoadAppConfigs(bool isReload = false);
	bool LoadModulesConfigs(bool isReload = false, bool isNeedPrintInfo = true);
	void Configure(std::string const& initFileName, std::vector<std::string> args, std::string_view modulesConfigList = {});

	static ConfigMgr* instance();

	bool Reload();

	std::string const GetFilename();
	std::string const GetConfigPath();
	[[nodiscard]] std::vector<std::string> const& GetArguments() const;
	std::vector<std::string> GetKeysByString(std::string const& name);

	template<typename T>
	T GetOption(std::string const& name, T const& def, bool showLogs = true) const;

	[[deprecated("Use GetOption<std::string> instead")]]
	std::string GetStringDefault(std::string const& name, const std::string& def, bool showLogs = true);

	[[deprecated("Use GetOption<bool> instead")]]
	bool GetBoolDefault(std::string const& name, bool def, bool showLogs = true);

	[[deprecated("Use GetOption<int32> instead")]]
	int GetIntDefault(std::string const& name, int def, bool showLogs = true);

	[[deprecated("Use GetOption<float> instead")]]
	float GetFloatDefault(std::string const& name, float def, bool showLogs = true);

	bool isDryRun() { return dryRun; }
	void setDryRun(bool mode) { dryRun = mode; }
private:
	bool LoadInitial(std::string const& file, bool isReload = false);
	bool LoadAdditionalFile(std::string file, bool isOptional = false, bool isReload = false);

	template<typename T>
	T GetValueDefault(std::string const& name, T const& def, bool showLogs = true) const;

	bool dryRun = false;

	std::vector <std::string> _moduleConfigFiles;
};

class ConfigException : public std::length_error
{
public:
	explicit ConfigException(std::string const& message) : std::length_error(message) { }
};

#define sConfigMgr ConfigMgr::instance()

#endif