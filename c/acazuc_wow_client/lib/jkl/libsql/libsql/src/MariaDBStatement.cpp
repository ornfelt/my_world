#include "MariaDBStatement.h"
#include "MariaDBConnection.h"
#include "Connection.h"
#include "Exception.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

namespace libsql
{

	MariaDBStatement::MariaDBStatement(MariaDBConnection &connection, const std::string &request)
	: connection(connection)
	, paramsCount(0)
	, resultCount(0)
	{
		if (!(this->statement = mysql_stmt_init(this->connection.getConnection())))
			throw Exception(mysql_errno(this->connection.getConnection()), mysql_error(this->connection.getConnection()));
		if (mysql_stmt_prepare(this->statement, request.c_str(), request.length()))
		{
			unsigned int err = mysql_stmt_errno(this->statement);
			std::string error = mysql_stmt_error(this->statement);
			mysql_stmt_close(this->statement);
			throw Exception(err, error);
		}
		this->params.resize(mysql_stmt_param_count(this->statement));
		this->result.resize(mysql_stmt_field_count(this->statement));
		/*this->meta = mysql_stmt_result_metadata(this->statement);
		if (!this->meta)
		{
			unsigned int err = mysql_stmt_errno(this->statement);
			std::string error = mysql_stmt_error(this->statement);
			mysql_stmt_close(this->statement);
			throw Exception(err, error);
		}*/
		/*for (uint8_t i = 0; i < this->resultNb; ++i)
		{
			this->result[i].buffer_type = this->meta->fields[i].type;
			this->result[i].buffer = new char[this->meta->fields[i].length]();
			this->result[i].buffer_length = this->meta->fields[i].length;
			this->result[i].length = NULL;
			this->result[i].is_null = NULL;
			this->result[i].is_unsigned = 0;
			this->result[i].error = NULL;
		}*/
	}

	MariaDBStatement::~MariaDBStatement()
	{
		mysql_stmt_close(this->statement);
	}

	void MariaDBStatement::execute()
	{
		this->paramsCount = 0;
		this->resultCount = 0;
		if (this->params.size())
		{
			if (mysql_stmt_bind_param(this->statement, this->params.data()))
				throw Exception(mysql_stmt_errno(this->statement), mysql_stmt_error(this->statement));
		}
		if (mysql_stmt_execute(this->statement))
			throw Exception(mysql_stmt_errno(this->statement), mysql_stmt_error(this->statement));
		if (this->result.size())
		{
			if (mysql_stmt_bind_result(this->statement, this->result.data()))
				throw Exception(mysql_stmt_errno(this->statement), mysql_stmt_error(this->statement));
		}
		if (mysql_stmt_store_result(this->statement))
			throw Exception(mysql_stmt_errno(this->statement), mysql_stmt_error(this->statement));
	}

	bool MariaDBStatement::fetch()
	{
		int result;
		if ((result = mysql_stmt_fetch(this->statement)) == MYSQL_NO_DATA)
			return false;
		else if (result && result != MYSQL_DATA_TRUNCATED)
			throw Exception(mysql_stmt_errno(this->statement), mysql_stmt_error(this->statement));
		return true;
	}

	uint64_t MariaDBStatement::getAffectedRows()
	{
		return mysql_stmt_affected_rows(this->statement);
	}

	uint64_t MariaDBStatement::getLastInsertId()
	{
		return mysql_stmt_insert_id(this->statement);
	}

	void MariaDBStatement::putValue(MYSQL_BIND *bind, enum enum_field_types type, void *value, size_t length, size_t *len, my_bool *is_null, my_bool is_unsigned, my_bool *error)
	{
		bind->buffer_type = type;
		bind->buffer = value;
		bind->buffer_length = length;
		bind->length = (long unsigned int*)len;
		bind->is_null = is_null;
		bind->is_unsigned = is_unsigned;
		bind->error = error;
	}

	void MariaDBStatement::addValue(enum enum_field_types type, const void *value, size_t length, size_t *len, my_bool is_unsigned)
	{
		putValue(&(this->params[this->paramsCount++]), type, const_cast<void*>(value), length, len, NULL, is_unsigned, NULL);
	}

	void MariaDBStatement::putBool(bool *value)
	{
		putUInt8(reinterpret_cast<uint8_t*>(value));
	}

	void MariaDBStatement::putInt8(int8_t *value)
	{
		addValue(MYSQL_TYPE_TINY, value, 1, NULL, 0);
	}

	void MariaDBStatement::putUInt8(uint8_t *value)
	{
		addValue(MYSQL_TYPE_TINY, value, 1, NULL, 1);
	}

	void MariaDBStatement::putInt16(int16_t *value)
	{
		addValue(MYSQL_TYPE_SHORT, value, 2, NULL, 0);
	}

	void MariaDBStatement::putUInt16(uint16_t *value)
	{
		addValue(MYSQL_TYPE_SHORT, value, 2, NULL, 1);
	}

	void MariaDBStatement::putInt32(int32_t *value)
	{
		addValue(MYSQL_TYPE_LONG, value, 4, NULL, 0);
	}

	void MariaDBStatement::putUInt32(uint32_t *value)
	{
		addValue(MYSQL_TYPE_LONG, value, 4, NULL, 1);
	}

	void MariaDBStatement::putInt64(int64_t *value)
	{
		addValue(MYSQL_TYPE_LONGLONG, value, 8, NULL, 0);
	}

	void MariaDBStatement::putUInt64(uint64_t *value)
	{
		addValue(MYSQL_TYPE_LONGLONG, value, 8, NULL, 1);
	}

	void MariaDBStatement::putFloat(float *value)
	{
		addValue(MYSQL_TYPE_FLOAT, value, 4, NULL, 0);
	}

	void MariaDBStatement::putDouble(double *value)
	{
		addValue(MYSQL_TYPE_DOUBLE, value, 8, NULL, 0);
	}

	void MariaDBStatement::putString(const char *value, size_t *len)
	{
		addValue(MYSQL_TYPE_STRING, value, *len, len, 0);
	}

	void MariaDBStatement::getValue(enum enum_field_types type, void *value, size_t length, size_t *len, my_bool is_unsigned)
	{
		putValue(&(this->result[this->resultCount++]), type, value, length, len, NULL, is_unsigned, NULL);
	}

	void MariaDBStatement::getBool(bool *value)
	{
		getUInt8(reinterpret_cast<uint8_t*>(value));
	}

	void MariaDBStatement::getInt8(int8_t *value)
	{
		getValue(MYSQL_TYPE_TINY, value, 1, NULL, 0);
	}

	void MariaDBStatement::getUInt8(uint8_t *value)
	{
		getValue(MYSQL_TYPE_TINY, value, 1, NULL, 1);
	}

	void MariaDBStatement::getInt16(int16_t *value)
	{
		getValue(MYSQL_TYPE_SHORT, value, 2, NULL, 0);
	}

	void MariaDBStatement::getUInt16(uint16_t *value)
	{
		getValue(MYSQL_TYPE_SHORT, value, 2, NULL, 1);
	}

	void MariaDBStatement::getInt32(int32_t *value)
	{
		getValue(MYSQL_TYPE_LONG, value, 4, NULL, 0);
	}

	void MariaDBStatement::getUInt32(uint32_t *value)
	{
		getValue(MYSQL_TYPE_LONG, value, 4, NULL, 1);
	}

	void MariaDBStatement::getInt64(int64_t *value)
	{
		getValue(MYSQL_TYPE_LONGLONG, value, 8, NULL, 0);
	}

	void MariaDBStatement::getUInt64(uint64_t *value)
	{
		getValue(MYSQL_TYPE_LONGLONG, value, 8, NULL, 1);
	}

	void MariaDBStatement::getFloat(float *value)
	{
		getValue(MYSQL_TYPE_FLOAT, value, 4, NULL, 1);
	}

	void MariaDBStatement::getDouble(double *value)
	{
		getValue(MYSQL_TYPE_DOUBLE, value, 8, NULL, 1);
	}

	void MariaDBStatement::getString(char *value, size_t *size)
	{
		getValue(MYSQL_TYPE_STRING, value, *size, size, 1);
	}

}
