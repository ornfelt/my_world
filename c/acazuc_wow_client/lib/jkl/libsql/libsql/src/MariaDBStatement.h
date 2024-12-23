#ifndef LIBSQL_MARIADB_STATEMENT_H
#define LIBSQL_MARIADB_STATEMENT_H

#include "Statement.h"

#include <mariadb/mysql.h>

#include <vector>
#include <string>

namespace libsql
{

	class MariaDBConnection;

	class MariaDBStatement : public Statement
	{

	private:
		std::vector<MYSQL_BIND> params;
		std::vector<MYSQL_BIND> result;
		MariaDBConnection &connection;
		MYSQL_STMT *statement;
		MYSQL_RES *meta;
		unsigned int paramsCount;
		unsigned int resultCount;
		void putValue(MYSQL_BIND *bind, enum enum_field_types type, void *value, size_t length, size_t *len, my_bool *is_null, my_bool is_unsigned, my_bool *error);
		void addValue(enum enum_field_types type, const void *value, size_t length, size_t *len, my_bool is_unsigned);
		void getValue(enum enum_field_types type, void *value, size_t length, size_t *len, my_bool is_unsigned);

	public:
		MariaDBStatement(MariaDBConnection &connection, const std::string &request);
		~MariaDBStatement();
		void execute();
		bool fetch();
		uint64_t getAffectedRows();
		uint64_t getLastInsertId();
		void putBool(bool *value);
		void putInt8(int8_t *value);
		void putUInt8(uint8_t *value);
		void putInt16(int16_t *value);
		void putUInt16(uint16_t *value);
		void putInt32(int32_t *value);
		void putUInt32(uint32_t *value);
		void putInt64(int64_t *value);
		void putUInt64(uint64_t *value);
		void putFloat(float *value);
		void putDouble(double *value);
		void putString(const char *value, size_t *len);
		void getBool(bool *value);
		void getInt8(int8_t *value);
		void getUInt8(uint8_t *value);
		void getInt16(int16_t *value);
		void getUInt16(uint16_t *value);
		void getInt32(int32_t *value);
		void getUInt32(uint32_t *value);
		void getInt64(int64_t *value);
		void getUInt64(uint64_t *value);
		void getFloat(float *value);
		void getDouble(double *value);
		void getString(char *value, size_t *size);
		inline uint32_t getParamsNumber() {return this->params.size();};
		inline uint32_t getResultNumber() {return this->result.size();};
		inline unsigned int getParamsCount() {return this->paramsCount;};
		inline unsigned int getResultCount() {return this->resultCount;};

	};

}

#endif
