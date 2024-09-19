#ifndef PREPAREDSTATEMENT_H_
#define PREPAREDSTATEMENT_H_

#include "Define.h"
#include "Duration.h"
#include "Optional.h"
#include "SQLOperation.h"
#include "MySQLPreparedStatement.h"

#include <future>
#include <tuple>
#include <variant>
#include <vector>

namespace mpool::Types
{
	template<typename T>
	using is_default = std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<std::vector<uint8>, T>>;

	template<typename T>
	using is_enum_v = std::enable_if_t<std::is_enum_v<T>>;

	template<typename T>
	using is_non_string_view_v = std::enable_if_t<!std::is_base_of_v<std::string_view, T>>;
}

struct PreparedStatementData
{
	std::variant<
	bool,
	uint8,
	uint16,
	uint32,
	uint64,
	int8,
	int16,
	int32,
	int64,
	float,
	double,
	std::string,
	std::vector<uint8>,
	std::nullptr_t
	> data;

	template<typename T>
	static std::string ToString(T value);

	static std::string ToString(std::nullptr_t);
};

class POOL_DATABASE_API PreparedStatementBase
{
	friend class PreparedStatementTask;

public:
	explicit PreparedStatementBase(uint32 index, uint8 capacity);
	explicit PreparedStatementBase(std::shared_ptr<MySQLPreparedStatement> preparedStatement, uint8 capacity);

	virtual ~PreparedStatementBase();

    [[nodiscard]] virtual std::shared_ptr<MySQLPreparedStatement> GetPreparedStmt() const
    {
        return m_preparedStatement;
    }

	//set numerlic and default binary
	template<typename T>
	inline mpool::Types::is_default<T> SetData(const uint8 index, T value)
	{
		SetValidData(index, value);
	}

	//set enums
	template<typename T>
	inline mpool::Types::is_enum_v<T> SetData(const uint8 index, T value)
	{
		SetValidData(index, std::underlying_type_t<T>(value));
	}

	//set string_view
	inline void SetData(const uint8 index, std::string_view value)
	{
		SetValidData(index, value);
	}

	//set nullptr
	inline void SetData(const uint8 index, std::nullptr_t = nullptr)
	{
		SetValidData(index);
	}

	//set default binary
	template<std::size_t Size>
	inline void SetData(const uint8 index, std::array<uint8, Size> const& value)
	{
		std::vector<uint8> vec(value.begin(), value.end());
		SetValidData(index, vec);
	}

	//set duration
	template<typename _Rep, typename _Period>
	inline void SetData(const uint8 index, std::chrono::duration<_Rep, _Period> const& value, bool convertToUin32 = true)
	{
		SetValidData(index, convertToUin32 ? static_cast<uint32>(value.count()) : value.count());
	}

	//set all
	template<typename... Args>
	inline void SetArguments(Args&&... args)
	{
		SetDataTuple(std::make_tuple(std::forward<Args>(args)...));
	}

	[[nodiscard]] uint32 GetIndex() const
	{
		return m_index;
	}

	[[nodiscard]] std::vector<PreparedStatementData> const& GetParameters() const
	{
		return statement_data;
	}

    void SetParameters(std::vector<PreparedStatementData> const& a)
    {
        statement_data = a;
    }

    //异步sql queue
    bool isPrepared = false;//异步是否预编译
    std::string sql = "";
protected:
	template<typename T>
	mpool::Types::is_non_string_view_v<T> SetValidData(const uint8 index, T const& value);

	void SetValidData(const uint8 index);
	void SetValidData(const uint8 index, std::string_view value);

	template<typename... Ts>
	inline void SetDataTuple(std::tuple<Ts...> const& argsList)
	{
		std::apply(
			[this](Ts const&... arguments)
			{
				uint8 index{ 0 };
				((SetData(index, arguments), index++), ...);
			}, argsList
		);
	}

	uint32 m_index;

	std::vector<PreparedStatementData> statement_data;

    //add
    std::shared_ptr<MySQLPreparedStatement> m_preparedStatement;

	PreparedStatementBase(PreparedStatementBase const& right) = delete;
	PreparedStatementBase& operator=(PreparedStatementBase const& right) = delete;
};

template<typename T>
class PreparedStatement : public PreparedStatementBase
{
public:
	explicit PreparedStatement(uint32 index, uint8 capacity) : PreparedStatementBase(index, capacity)
	{

	}

    PreparedStatement(T* conn, std::shared_ptr<MySQLPreparedStatement> preparedStatement, uint8 capacity) :
    connection(conn),
    PreparedStatementBase(preparedStatement, capacity)
	{

	}

    [[nodiscard]] T* GetPreparedConn() const
    {
        return connection;
    }

private:
	PreparedStatement(PreparedStatement const& right) = delete;
	PreparedStatement& operator=(PreparedStatement const& right) = delete;

    //改造直接prepare sql 将连接对象缓存到参数对象中
    T* connection;
};


class POOL_DATABASE_API PreparedStatementTask : public SQLOperation
{
public:
	PreparedStatementTask(PreparedStatementBase* stmt, bool async = false);
	~PreparedStatementTask() override;

	bool Execute() override;

	PreparedQueryResultFuture GetFuture()
	{
		return m_result->get_future();
	}
protected:
	PreparedStatementBase* m_stmt;
	bool m_has_result;
	PreparedQueryResultPromise* m_result;
};


#endif // !PREPAREDSTATEMENT_H_
