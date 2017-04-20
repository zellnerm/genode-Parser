#pragma once

#include <base/signal.h>
#include <parser/parser_session.h>
#include <os/attached_ram_dataspace.h>
#include <os/server.h>
#include <root/component.h>
#include <timer_session/connection.h>
#include <util/string.h>
#include <cap_session/connection.h>
#include "mon_manager/mon_manager_connection.h"
#include "mon_manager/mon_manager.h"

struct Parser_session_component : Genode::Rpc_object<Parser_session>
{
public:
	Parser_session_component(Server::Entrypoint& ep);
	virtual ~Parser_session_component();
	// Get profiling data as an XML file.
	Genode::Ram_dataspace_capability profile_data();

	//live monitoring data
	Genode::Ram_dataspace_capability live_data();

protected:
	Server::Entrypoint& _ep;
	Genode::Cap_connection _cap;
	static Genode::Number_of_bytes _profile_ds_size();

	Genode::Attached_ram_dataspace _profile_data;
	Genode::Attached_ram_dataspace _live_data;

	Mon_manager::Connection _mon_manager;
	Genode::Ram_dataspace_capability mon_ds_cap;
};

struct Parser_root_component : Genode::Root_component<Parser_session_component>
{
public:
	Parser_root_component(Server::Entrypoint* ep, Genode::Allocator *allocator) :
		Genode::Root_component<Parser_session_component>(&ep->rpc_ep(), allocator),
		_ep(*ep)
	{
		PDBG("Creating root component.");
	}

protected:
	Server::Entrypoint& _ep;
	Parser_session_component* _create_session(const char *args)
	{
		PDBG("Creating Parser session.");
		return new (md_alloc()) Parser_session_component(_ep);
	}
};
