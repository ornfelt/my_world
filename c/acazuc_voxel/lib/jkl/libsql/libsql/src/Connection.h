#ifndef LIBSQL_CONNECTION_H
#define LIBSQL_CONNECTION_H

#include <cstdint>
#include <string>

namespace libsql
{

	class Statement;

	class Connection
	{

	private:

	public:
		virtual ~Connection() {};
		virtual void setAutocommit(bool autocommit) = 0;
		virtual void commit() = 0;
		virtual void rollback() = 0;
		virtual uint64_t getLastInsertId() = 0;
		virtual Statement *prepare(const std::string &request) = 0;
		virtual std::string getServerInfos() = 0;
		virtual std::string getClientInfos() = 0;
		virtual std::string getHostInfos() = 0;

	};

}

#endif
