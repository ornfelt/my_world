#include "MariaDBConnection.h"
#include "MariaDBStatement.h"
#include "Exception.h"

#include <climits>

namespace libsql
{

	MariaDBConnection::MariaDBConnection(const std::string &sock, const std::string &user, const std::string &password, const std::string &database)
	{
		if (!(this->connection = mysql_init(NULL)))
			throw Exception(0, "Failed to init mysql client");
		if (mysql_optionsv(this->connection, MYSQL_SET_CHARSET_NAME, "utf8mb4"))
		{
			unsigned int err = mysql_errno(this->connection);
			std::string error = mysql_error(this->connection);
			mysql_close(this->connection);
			throw Exception(err, error);
		}
		if (mysql_optionsv(this->connection, MYSQL_INIT_COMMAND, "SET NAMES utf8mb4"))
		{
			unsigned int err = mysql_errno(this->connection);
			std::string error = mysql_error(this->connection);
			mysql_close(this->connection);
			throw Exception(err, error);
		}
		if (!mysql_real_connect(this->connection, NULL, user.c_str(), password.c_str(), database.c_str(), 0, sock.c_str(), 0))
		{
			unsigned int err = mysql_errno(this->connection);
			std::string error = mysql_error(this->connection);
			mysql_close(this->connection);
			throw Exception(err, error);
		}
	}

	MariaDBConnection::MariaDBConnection(const std::string &host, const std::string &user, const std::string &password, const std::string &database, unsigned short port)
	{
		if (!(this->connection = mysql_init(NULL)))
			throw Exception(0, "Failed to init mysql client");
		if (mysql_optionsv(this->connection, MYSQL_SET_CHARSET_NAME, "utf8mb4"))
		{
			unsigned int err = mysql_errno(this->connection);
			std::string error = mysql_error(this->connection);
			mysql_close(this->connection);
			throw Exception(err, error);
		}
		if (mysql_optionsv(this->connection, MYSQL_INIT_COMMAND, "SET NAMES utf8mb4"))
		{
			unsigned int err = mysql_errno(this->connection);
			std::string error = mysql_error(this->connection);
			mysql_close(this->connection);
			throw Exception(err, error);
		}
		if (!mysql_real_connect(this->connection, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, NULL, 0))
		{
			unsigned int err = mysql_errno(this->connection);
			std::string error = mysql_error(this->connection);
			mysql_close(this->connection);
			throw Exception(err, error);
		}
	}

	MariaDBConnection::~MariaDBConnection()
	{
		mysql_close(this->connection);
	}

	void MariaDBConnection::setAutocommit(bool autocommit)
	{
		if (mysql_autocommit(this->connection, autocommit ? 1 : 0))
			throw Exception(mysql_errno(this->connection), mysql_error(this->connection));
	}

	void MariaDBConnection::commit()
	{
		if (mysql_commit(this->connection))
			throw Exception(mysql_errno(this->connection), mysql_error(this->connection));
	}

	void MariaDBConnection::rollback()
	{
		if (mysql_rollback(this->connection))
			throw Exception(mysql_errno(this->connection), mysql_error(this->connection));
	}

	uint64_t MariaDBConnection::getLastInsertId()
	{
		return mysql_insert_id(this->connection);
	}

	Statement *MariaDBConnection::prepare(const std::string &request)
	{
		return new MariaDBStatement(*this, request);
	}

	std::string MariaDBConnection::getServerInfos()
	{
		const char *serverVersion = mysql_get_server_info(this->connection);
		if (!serverVersion)
			return "";
		return std::string(serverVersion);
	}

	std::string MariaDBConnection::getClientInfos()
	{
		const char *serverVersion = mysql_get_client_info();
		if (!serverVersion)
			return "";
		return std::string(serverVersion);
	}

	std::string MariaDBConnection::getHostInfos()
	{
		const char *serverVersion = mysql_get_host_info(this->connection);
		if (!serverVersion)
			return "";
		return std::string(serverVersion);
	}

}
