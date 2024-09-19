#ifndef QUERYRESULT_H_
#define QUERYRESULT_H_

#include "DatabaseEnvFwd.h"
#include "Define.h"
#include "Field.h"

#include <tuple>
#include <vector>


class POOL_DATABASE_API ResultSet
{
public:
	ResultSet(MySQLResult* result, MySQLField* fields, uint64 rowCount, uint32 fieldCount);
	~ResultSet();

	bool NextRow();
	[[nodiscard]] uint64 GetRowCount() const { return _rowCount; }
	[[nodiscard]] uint32 GetFieldCount() const { return _fieldCount; }
	[[nodiscard]] std::string GetFieldName(uint32 index) const;

	[[nodiscard]] Field* Fetch() const { return _currentRow; }
	Field const& operator[](std::size_t index) const;

	template<typename... Ts>
	inline std::tuple<Ts...> FetchTuple()
	{
		AssertRows(sizeof...(Ts));

		std::tuple<Ts...> theTuple = {};

		std::apply([this](Ts&... args)
			{
				uint8 index{ 0 };
				((args = _currentRow[index].Get<Ts>(), index++), ...);
			}, theTuple);

		return theTuple;
	}

protected:
	std::vector<QueryResultFieldMetadata> _fieldMetadata;
	uint64 _rowCount;
	Field* _currentRow;
	uint32 _fieldCount;
private:
	void CleanUp();
	void AssertRows(std::size_t sizeRows);

	MySQLResult* _result;
	MySQLField* _fields;

	ResultSet(ResultSet const& right) = delete;
	ResultSet& operator=(ResultSet const& right) = delete;
};

class POOL_DATABASE_API PreparedResultSet
{
public:
	PreparedResultSet(MySQLStmt* stmt, MySQLResult* result, uint64 rowCount, uint32 fieldCount);
	~PreparedResultSet();

	bool NextRow();
	[[nodiscard]] uint64 GetRowCount() const { return m_rowCount; }
	[[nodiscard]] uint32 GetFieldCount() const { return m_fieldCount; }

	[[nodiscard]] Field* Fetch() const;
	Field const& operator[](std::size_t index) const;

	template<typename... Ts>
	inline std::tuple<Ts...> FetchTuple()
	{
		AssertRows(sizeof...(Ts));

		std::tuple<Ts...> theTuple = {};

		std::apply([this](Ts&... args)
			{
				uint8 index{0};
				((args = m_rows[uint32(m_rowPosition) * m_fieldCount + index].Get<Ts>(), index++), ...);
			}, theTuple);

		return theTuple;
	}

protected:
	std::vector<QueryResultFieldMetadata> m_fieldMetadata;
	std::vector<Field> m_rows;
	uint64 m_rowCount;
	uint64 m_rowPosition;
	uint32 m_fieldCount;
private:
	MySQLBind* m_rBind;
	MySQLStmt* m_stmt;
	MySQLResult* m_metadataResult;

	void CleanUp();
	bool _NextRow();

	void AssertRows(std::size_t sizeRows);

	PreparedResultSet(PreparedResultSet const& right) = delete;
	PreparedResultSet& operator=(PreparedResultSet const& right) = delete;
};

#endif