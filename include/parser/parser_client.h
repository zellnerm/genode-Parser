#pragma once

#include <base/printf.h>
#include <base/rpc_client.h>
#include <parser/parser_session.h>

struct Parser_session_client : Genode::Rpc_client<Parser_session>
{
	Parser_session_client(Genode::Capability<Parser_session> cap):
		Genode::Rpc_client<Parser_session>(cap) {}

	Genode::Ram_dataspace_capability profile_data()
	{
		return call<Rpc_profile_data>();
	}

	Genode::Ram_dataspace_capability live_data()
	{
		return call<Rpc_live_data>();
	}

};
