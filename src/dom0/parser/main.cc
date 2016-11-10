#include <base/env.h>
#include <base/printf.h>
#include <base/rpc_server.h>
#include <base/sleep.h>
#include <os/server.h>

#include "parser_session_component.h"

struct Main
{
	Parser_root_component parser_root;

	Main(Server::Entrypoint& ep) :
		parser_root(&ep, Genode::env()->heap())
	{
		PDBG("parser: Hello!\n");
		Genode::env()->parent()->announce(ep.rpc_ep().manage(&parser_root));
	}
};

/************
 ** Server **
 ************/

namespace Server
{
	char const *name()             { return "parser";      }
	size_t stack_size()            { return 64*1024*sizeof(long); }
	void construct(Entrypoint& ep) { static Main server(ep);     }
}
