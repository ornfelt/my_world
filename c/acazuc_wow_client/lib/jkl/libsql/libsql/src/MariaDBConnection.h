#ifndef LIBSQL_MARIADB_CONNECTION_H
#define LIBSQL_MARIADB_CONNECTION_H

#include "Connection.h"

#include <mariadb/mysql.h>

#include <string>

namespace libsql
{

	class Statement;

	class MariaDBConnection : public Connection
	{

	private:
		MYSQL *connection;

	public:
		MariaDBConnection(const std::string &sock, const std::string &user, const std::string &password, const std::string &database);
		MariaDBConnection(const std::string &host, const std::string &user, const std::string &password, const std::string &database, unsigned short port);
		~MariaDBConnection();
		void setAutocommit(bool autocommit);
		void commit();
		void rollback();
		uint64_t getLastInsertId();
		Statement *prepare(const std::string &request);
		std::string getServerInfos();
		std::string getClientInfos();
		std::string getHostInfos();
		inline MYSQL *getConnection() {return this->connection;};

	};

}

#endif
