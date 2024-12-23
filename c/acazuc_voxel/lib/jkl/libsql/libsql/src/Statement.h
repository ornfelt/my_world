#ifndef LIBSQL_STATEMENT_H
#define LIBSQL_STATEMENT_H

#include <cstdint>
#include <cstdlib>

namespace libsql
{

	class Statement
	{

	public:
		virtual ~Statement() {};
		virtual void execute() = 0;
		virtual bool fetch() = 0;
		virtual uint64_t getAffectedRows() = 0;
		virtual uint64_t getLastInsertId() = 0;
		virtual void putBool(bool *value) = 0;
		virtual void putInt8(int8_t *value) = 0;
		virtual void putUInt8(uint8_t *value) = 0;
		virtual void putInt16(int16_t *value) = 0;
		virtual void putUInt16(uint16_t *value) = 0;
		virtual void putInt32(int32_t *value) = 0;
		virtual void putUInt32(uint32_t *value) = 0;
		virtual void putInt64(int64_t *value) = 0;
		virtual void putUInt64(uint64_t *value) = 0;
		virtual void putFloat(float *value) = 0;
		virtual void putDouble(double *value) = 0;
		virtual void putString(const char *value, size_t *len) = 0;
		virtual void getBool(bool *value) = 0;
		virtual void getInt8(int8_t *value) = 0;
		virtual void getUInt8(uint8_t *value) = 0;
		virtual void getInt16(int16_t *value) = 0;
		virtual void getUInt16(uint16_t *value) = 0;
		virtual void getInt32(int32_t *value) = 0;
		virtual void getUInt32(uint32_t *value) = 0;
		virtual void getInt64(int64_t *value) = 0;
		virtual void getUInt64(uint64_t *value) = 0;
		virtual void getFloat(float *value) = 0;
		virtual void getDouble(double *value) = 0;
		virtual void getString(char *value, size_t *size) = 0;

	};

}

#endif
