#ifndef FIELD_H_
#define FIELD_H_

#include "DatabaseEnvFwd.h"
#include "Define.h"
#include "Duration.h"

#include <array>
#include <string_view>
#include <vector>

namespace mpool::Types
{
	template<typename T>
	using is_chrono_v = std::enable_if_t<std::is_same_v<Milliseconds, T>
		|| std::is_same_v<Seconds, T>
		|| std::is_same_v<Minutes, T>
		|| std::is_same_v<Hours, T>
		|| std::is_same_v<Days, T>
		|| std::is_same_v<Weeks, T>
		|| std::is_same_v<Years, T>
		|| std::is_same_v<Months, T>, T>;
}

using Binary = std::vector<uint8>;

enum class DatabaseFieldTypes : uint8
{
	Null,
	Int8,
	Int16,
	Int32,
	Int64,
	Float,
	Double,
	Decimal,
	Date,
	Binary
};

struct QueryResultFieldMetadata
{
	std::string TableName{};
	std::string TableAlias{};
	std::string Name{};
	std::string Alias{};
	std::string TypeName{};
	uint32 Index = 0;
	DatabaseFieldTypes Type = DatabaseFieldTypes::Null;
};

/**
	@class Field

	@brief Class used to access individual fields of database query result

	Guideline on field type matching:

	|   MySQL type           |  method to use                          |
	|------------------------|-----------------------------------------|
	| TINYINT                | Get<bool>, Get<int8>, Get<uint8>        |
	| SMALLINT               | Get<int16>, Get<uint16>                 |
	| MEDIUMINT, INT         | Get<int32>, Get<uint32>                 |
	| BIGINT                 | Get<int64>, Get<uint64>                 |
	| FLOAT                  | Get<float>                              |
	| DOUBLE, DECIMAL        | Get<double>                             |
	| CHAR, VARCHAR,         | Get<std::string>, Get<std::string_view> |
	| TINYTEXT, MEDIUMTEXT,  | Get<std::string>, Get<std::string_view> |
	| TEXT, LONGTEXT         | Get<std::string>, Get<std::string_view> |
	| TINYBLOB, MEDIUMBLOB,  | Get<Binary>, Get<std::string>           |
	| BLOB, LONGBLOB         | Get<Binary>, Get<std::string>           |
	| BINARY, VARBINARY      | Get<Binary>                             |

	Return types of aggregate functions:

	| Function |       Type        |
	|----------|-------------------|
	| MIN, MAX | Same as the field |
	| SUM, AVG | DECIMAL           |
	| COUNT    | BIGINT            |
*/

class POOL_DATABASE_API Field
{
	friend class ResultSet;
	friend class PreparedResultSet;

public:
	Field();
	~Field() = default;

	[[nodiscard]] inline bool IsNull() const
	{
		return data.value == nullptr;
	}

	template<typename T>
	inline std::enable_if_t<std::is_arithmetic_v<T>, T> Get() const
	{
		return GetData<T>();
	}

	template<typename T>
	inline std::enable_if_t<std::is_same_v<std::string, T>, T> Get() const
	{
		return GetDataString();
	}

	template<typename T>
	inline std::enable_if_t<std::is_same_v<std::string_view, T>, T> Get() const
	{
		return GetDataStringView();
	}

	template<typename T>
	inline std::enable_if_t<std::is_same_v<Binary, T>, T> Get() const
	{
		return GetDataBinary();
	}

	template<typename T, size_t S>
	inline std::enable_if_t<std::is_same_v<Binary, T>, std::array<uint8, S>> Get() const
	{
		std::array<uint8, S> buf = {};
		GetBinarySizeChecked(buf.data(), S);
		return buf;
	}

	template<typename T>
	inline mpool::Types::is_chrono_v<T> Get(bool convertToUin32 = true) const
	{
		return convertToUin32 ? T(GetData<uint32>()) : T(GetData<uint64>());
	}

	DatabaseFieldTypes GetType() { return meta->Type; }
protected:
	struct 
	{
		char const* value;
		uint32 length;
		bool  raw;
	} data;

	void SetByteValue(char const* newValue, uint32 length);
	void SetStructuredValue(char const* newValue, uint32 length);
	[[nodiscard]] bool IsType(DatabaseFieldTypes type) const;
	[[nodiscard]] bool IsNumeric() const;
private:
	template<typename T>
	T GetData() const;

	std::string GetDataString() const;
	std::string_view GetDataStringView() const;
	Binary GetDataBinary() const;

	QueryResultFieldMetadata const* meta;
	void LogWrongType(std::string_view getter, std::string_view typeName) const;
	void SetMetadata(QueryResultFieldMetadata const* fieldMeta);
	void GetBinarySizeChecked(uint8* buf, size_t size) const;
};


#endif // !FIELD_H_
