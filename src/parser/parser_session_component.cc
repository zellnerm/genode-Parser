#include "parser_session_component.h"
#include <timer_session/connection.h>
#include <base/env.h>
#include <base/printf.h>
#include <base/process.h>
#include <util/xml_node.h>
#include <util/xml_generator.h>
#include <string>
#include <util/string.h>
#include <trace_session/connection.h>
#include <os/config.h>

Parser_session_component::Parser_session_component(Server::Entrypoint& ep) :
	_ep{ep},
	_cap{},
	_profile_data{Genode::env()->ram_session(), _profile_ds_size()},
	_live_data{Genode::env()->ram_session(), _profile_ds_size()}
{
}

Parser_session_component::~Parser_session_component()
{
}

Genode::Ram_dataspace_capability Parser_session_component::live_data()
{
	static Timer::Connection timer;
	static Genode::Trace::Connection trace(1024*4096, 64*4096, 0);
	Genode::Trace::Subject_id subjects[32];
	size_t num_subjects = trace.subjects(subjects, 32);
	Genode::Trace::RAM_info init;
	int timestamp=1000;

	Genode::Xml_generator xml(_live_data.local_addr<char>(), _live_data.size(), "", [&]()
	{
		for (size_t i = 0; i < num_subjects; i++) {
			Genode::Trace::CPU_info info = trace.cpu_info(subjects[i]);
			Genode::Trace::RAM_info ram_info = trace.ram_info(subjects[i]);
			Genode::Trace::SCHEDULER_info scheduler_info = trace.scheduler_info(subjects[i]);
					if(Genode::strcmp(info.session_label().string(), "init")!=0) {
			       			xml.attribute("id", std::to_string(info.id()).c_str());
						xml.attribute("foc_id", std::to_string(scheduler_info.foc_id()).c_str());
						xml.attribute("execution-time", std::to_string(info.execution_time().value/1000).c_str());
			       			xml.attribute("priority", std::to_string(info.prio()).c_str());
						xml.attribute("session", info.session_label().string());
						xml.attribute("thread", info.thread_name().string());
			      	 		xml.attribute("ram_quota", std::to_string(ram_info.ram_quota()/1024).c_str());
						xml.attribute("ram_used", std::to_string(ram_info.ram_used()/1024).c_str());
						xml.attribute("", "\n");
					}
					if(Genode::strcmp(info.session_label().string(), "init")==0) {
						init=ram_info;
					}
					
		}
		Genode::Trace::SCHEDULER_info scheduler_info = trace.scheduler_info(subjects[0]);
		xml.attribute("Idle time", std::to_string(scheduler_info.idle0().value/1000).c_str());
		xml.attribute("RAM avail in MB", std::to_string(init.ram_quota()/1048576).c_str());
		xml.attribute("CPU 0 online", std::to_string(scheduler_info.core0_is_online()).c_str());
		xml.attribute("CPU 1 online", std::to_string(scheduler_info.core1_is_online()).c_str());
		xml.attribute("CPU 2 online", std::to_string(scheduler_info.core2_is_online()).c_str());
		xml.attribute("CPU 3 online", std::to_string(scheduler_info.core3_is_online()).c_str());
		xml.attribute("Number of cores", std::to_string(scheduler_info.num_cores()).c_str());
		xml.attribute("", "\n");
	});
	
	return _live_data.cap();
}

Genode::Ram_dataspace_capability Parser_session_component::profile_data()
{
	static Genode::Trace::Connection trace(1024*4096, 64*4096, 0);

	Genode::Trace::Subject_id subjects[32];
	size_t num_subjects = trace.subjects(subjects, 32);


	//Task::log_profile_data(Task::Event::EXTERNAL, -1, _shared);

	// Xml_generator directly writes XML data into the buffer on construction, explaining the heavy recursion here.
	//PDBG("Generating event log. %d events have occurred.", _shared.event_log.size());
	Genode::Xml_generator xml(_profile_data.local_addr<char>(), _profile_data.size(), "profile", [&]()
	{
		xml.node("task-descriptions", [&]()
		{
			for (size_t i = 0; i < num_subjects; i++)
			{
				Genode::Trace::CPU_info info = trace.cpu_info(subjects[i]);
				Genode::Trace::RAM_info ram_info = trace.ram_info(subjects[i]);
				xml.node("task", [&]()
				{
					xml.attribute("id", std::to_string(info.id()).c_str());
					xml.attribute("execution-time", std::to_string(info.execution_time().value).c_str());
					xml.attribute("critical-time", std::to_string(0).c_str());
					xml.attribute("priority", std::to_string(info.prio()).c_str());
					xml.attribute("period", std::to_string(0).c_str());
					xml.attribute("offset", std::to_string(0).c_str());
					xml.attribute("quota", std::to_string(ram_info.ram_quota()).c_str());
					xml.attribute("binary", info.thread_name().string());
				});
			}
		});

		/*xml.node("events", [&]()
		{
			for (const Task::Event& event : _shared.event_log)
			{
				xml.node("event", [&]()
				{
					xml.attribute("type", Task::Event::type_name(event.type));
					xml.attribute("task-id", std::to_string(event.task_id).c_str());
					xml.attribute("time-stamp", std::to_string(event.time_stamp).c_str());

					for (size_t i = 0; i < num_subjects; i++)
					{
						Genode::Trace::CPU_info info = trace.cpu_info(subjects[i]);
						Genode::Trace::RAM_info ram_info = trace.ram_info(subjects[i]);
						xml.node("task", [&]()
						{
							xml.attribute("id", std::to_string(info.id()).c_str());
							xml.attribute("session", info.session_label().string());
							xml.attribute("thread", info.thread_name().string());
							xml.attribute("execution-time", std::to_string(info.execution_time().value).c_str());
						});
					}
				});
			}
		});*/
	});

	//_shared.event_log.clear();

	return _profile_data.cap();
}

Genode::Number_of_bytes Parser_session_component::_profile_ds_size()
{
	Genode::Xml_node launchpad_node = Genode::config()->xml_node().sub_node("profile");
	return launchpad_node.attribute_value<Genode::Number_of_bytes>("ds-size", 128 * 1024);
}
