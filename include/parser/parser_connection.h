#pragma once

#include <parser/parser_client.h>
#include <base/connection.h>

struct Parser_connection : Genode::Connection<Parser_session>, Parser_session_client
{
	Parser_connection() :
		Genode::Connection<Parser_session>(session("bar, ram_quota=1M")),

		Parser_session_client(cap())
	{
	}
};
