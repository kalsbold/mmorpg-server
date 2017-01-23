#pragma once
#include <gisunnet/types.h>

namespace gisunnet {
	/// Contains result sets and provides access interface.
	class ResultSets
	{
	public:
		ResultSets();
		/*
		/// Gets the result set count.
		/// @return the result set count.
		size_t GetResultSetCount() const;

		/// Seeks the next result set.
		/// @return true if it is not the last result set.
		bool SeekNextResultSet();

		/// Gets the affected row count of the current result set.
		size_t GetAffectedRowCount() const;

		/// Gets the column count of the current result set.
		/// @return the column count of the current result set.
		size_t GetColumnCount() const;

		/// Gets the column name of specified index.
		/// @param column_index column index.
		/// @return column name
		string GetColumnName(const size_t &column_index) const;

		/// Gets the column type of specified index.
		/// @param column_index column index.
		/// @return column type. See mysql/mysql_com.h
		enum_field_types GetColumnType(const size_t &column_index) const;

		/// Gets the row count of the current result set.
		/// @return the row count of the current result set.
		size_t GetRowCount() const;

		/// Seeks the next row.
		/// @return true if it is not the last row.
		bool SeekNextRow();

		/// Check the specified column's value of the current row is null.
		/// @param column_index column index.
		bool IsNull(size_t column_index) const;

		/// Gets the specified culumn's value of the current row as bool.
		/// @param column_index column index.
		/// @return column value.
		bool GetBool(size_t column_index, bool regard_null_as = false) const;

		/// Gets the specified culumn's value of the current row as int.
		/// @param column_index column index.
		/// @return column value.
		int64_t GetInt(size_t column_index, int64_t regard_null_as = 0) const;

		/// Gets the specified culumn's value of the current row as unsigned int.
		/// @param column_index column index.
		/// @return column value.
		uint64_t GetUint(size_t column_index, uint64_t regard_null_as = 0) const;

		/// Gets the specified culumn's value of the current row as float.
		/// @param column_index column index.
		/// @return column value.
		float GetFloat(size_t column_index, float regard_null_as = 0.0) const;

		/// Gets the specified culumn's value of the current row as double.
		/// @param column_index column index.
		/// @return column value.
		double GetDouble(size_t column_index, double regard_null_as = 0.0) const;

		/// Gets the specified culumn's value of the current row as string.
		/// @param column_index column index.
		/// @return column value.
		const char *GetString(size_t column_index) const;

		/// Gets the specified column's value of the current row as datetime.
		/// @param column_index column index.
		/// @return column value.
		WallClock::Value GetDateTime(
			size_t column_index,
			const WallClock::Value &regard_null_as = WallClock::kEpochClock) const;

		/// Gets the specified column's value of the current row as time.
		/// @param column_index column index.
		/// @return column value.
		WallClock::Duration GetTime(
			size_t column_index,
			const WallClock::Duration &regard_null_as =
			WallClock::kEmptyDuration) const;

		/// Gets the specified culumn's value of the current row as bool.
		/// @param column_name column name.
		/// @return column value.
		bool GetBool(const string &column_name, bool regard_null_as = false) const;

		/// Gets the specified culumn's value of the current row as int.
		/// @param column_name column name.
		/// @return column value.
		int64_t GetInt(const string &column_name, int64_t regard_null_as = 0) const;

		/// Gets the specified culumn's value of the current row as unsigned int.
		/// @param column_name column name.
		/// @return column value.
		uint64_t GetUint(const string &column_name, uint64_t regard_null_as = 0) const;

		/// Gets the specified culumn's value of the current row as float.
		/// @param column_name column name.
		/// @return column value.
		float GetFloat(const string &column_name, float regard_null_as = 0.0) const;

		/// Gets the specified culumn's value of the current row as double.
		/// @param column_name column name.
		/// @return column value.
		double GetDouble(const string &column_name, double regard_null_as = 0.0) const;

		/// Gets the specified culumn's value of the current row as string.
		/// @param column_name column name.
		/// @return column value.
		const char *GetString(const string &column_name) const;

		/// Gets the specified culumn's value of the current row as datetime.
		/// @param column_name column name.
		/// @return column value.
		WallClock::Value GetDateTime(
			const string &column_name,
			const WallClock::Value &regard_null_as = WallClock::kEpochClock) const;

		/// Gets the specified culumn's value of the current row as datetime.
		/// @param column_name column name.
		/// @return column value.
		WallClock::Duration GetTime(
			const string &column_name,
			const WallClock::Duration &regard_null_as =
			WallClock::kEmptyDuration) const;

	private:
	*/
	};


	class MySQL
	{
	public:
		struct Error
		{
			int code;
			string desc;
			string sqlstate;
		};

		typedef function<void(const Ptr<ResultSets>&, const Error&)> QueryExecuteHandler;

		static Ptr<MySQL> Create(const string& address, const string& id, const string& password, const string &database,
			size_t connection_count, const string& connection_charset = string(""), bool auto_retry_on_deadlock = false);

		void Initialize();
		void Finalize();

		void ExecuteQueryAsync(const string& query, const QueryExecuteHandler& handler);
		Ptr<ResultSets> ExecuteQuery(const string& query, Error* error = nullptr);

		const string& address() const;
		const string& id() const;
		const string& database() const;
		size_t connection_count() const;

	private:
		MySQL(const string& address, const string& id, const string& password, const string& database,
			size_t connection_count, const string& connection_charset, bool auto_retry_on_deadlock);

	};
}