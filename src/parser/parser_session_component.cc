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
	killed_tasks.clear();
}

Parser_session_component::~Parser_session_component()
{
}

Genode::Ram_dataspace_capability Parser_session_component::live_data()
{
	mon_ds_cap = Genode::env()->ram_session()->alloc(100*sizeof(Mon_manager::Monitoring_object));
	Mon_manager::Monitoring_object *threads=Genode::env()->rm_session()->attach(mon_ds_cap);
	dead_ds_cap = Genode::env()->ram_session()->alloc(256*sizeof(long long unsigned));
	long long unsigned *rip=Genode::env()->rm_session()->attach(dead_ds_cap);
	_mon_manager.update_info(mon_ds_cap);
	_mon_manager.update_dead(dead_ds_cap);
	
	Genode::Xml_generator xml(_live_data.local_addr<char>(), _live_data.size(), "live", [&]()
	{
		xml.node("task-descriptions", [&]()
		{
			//for (int j = 0; j < 1; j++) {
					for (size_t i = 0; i < 150; i++) {
						//check auf threads[i].session_label.string() ob es mit "init -> taskloader"

						//std::string tp(threads[i].session_label.string());

						//if(tp.find("init -> taskloader")==0 && threads[i].state!=5)
						//{
							xml.node("task", [&]()
							{
							xml.attribute("id", std::to_string(threads[i].id).c_str());
							xml.attribute("foc_id", std::to_string(threads[i].foc_id).c_str());
							xml.attribute("execution-time", std::to_string(threads[i].execution_time.value/1000).c_str());
							xml.attribute("priority", std::to_string(threads[i].prio).c_str());
							xml.attribute("core", std::to_string(threads[i].affinity.xpos()).c_str());
							xml.attribute("policy-id", std::to_string(threads[i].policy_id.id).c_str());
							xml.attribute("state", std::to_string(threads[i].state).c_str());
							xml.attribute("arrival-time", std::to_string(threads[i].arrival_time/1000).c_str());
							xml.attribute("start-time", std::to_string(threads[i].start_time/1000).c_str());
							xml.attribute("session", threads[i].session_label.string());
							xml.attribute("thread", threads[i].thread_name.string());
							xml.attribute("ram_quota", std::to_string(threads[i].ram_quota/1024).c_str());
							xml.attribute("ram_used", std::to_string(threads[i].ram_used/1024).c_str());
							});
						//}
					}
			//Genode::printf("run %d\n",j);
			_mon_manager.update_info(mon_ds_cap);
			//}
		});
	});

	xml.node("RIP_table", [&]()
	{
		xml.attribute("RIP_table_size", rip[0]);
		for(unsigned int z=1;z<rip[0]*2+1;z++){
			xml.node("dead_task", [&]()
			{
				xml.attribute("foc_id", rip[z++]);
				xml.attribute("time", rip[z]/1000);
			});
		}
	});

	PINF("killed size %d", killed_tasks.size());
	Genode::env()->ram_session()->free(mon_ds_cap);
	Genode::env()->ram_session()->free(dead_ds_cap);
	return _live_data.cap();
}


Genode::Ram_dataspace_capability Parser_session_component::profile_data()
{
	mon_ds_cap = Genode::env()->ram_session()->alloc(100*sizeof(Mon_manager::Monitoring_object));
	Mon_manager::Monitoring_object *threads=Genode::env()->rm_session()->attach(mon_ds_cap);
	_mon_manager.update_info(mon_ds_cap);
	
	Genode::Xml_generator xml(_live_data.local_addr<char>(), _live_data.size(), "live", [&]()
	{
		xml.node("task-descriptions", [&]()
		{
					for (size_t i = 0; i < 100; i++) {
						xml.node("task", [&]()
						{
			       			xml.attribute("id", std::to_string(threads[i].id).c_str());
						xml.attribute("foc_id", std::to_string(threads[i].foc_id).c_str());
						xml.attribute("execution-time", std::to_string(threads[i].execution_time.value/1000).c_str());
			       			xml.attribute("priority", std::to_string(threads[i].prio).c_str());
						xml.attribute("core", std::to_string(threads[i].affinity.xpos()).c_str());
						xml.attribute("policy id", std::to_string(threads[i].policy_id.id).c_str());
						xml.attribute("state", std::to_string(threads[i].state).c_str());
						xml.attribute("arrival time", std::to_string(threads[i].arrival_time).c_str());
						xml.attribute("start time", std::to_string(threads[i].start_time).c_str());
						xml.attribute("session", threads[i].session_label.string());
						xml.attribute("thread", threads[i].thread_name.string());
			      	 		xml.attribute("ram_quota", std::to_string(threads[i].ram_quota/1024).c_str());
						xml.attribute("ram_used", std::to_string(threads[i].ram_used/1024).c_str());
						});
					}
			//Genode::printf("run %d\n",j);
			_mon_manager.update_info(mon_ds_cap);
		});
	});
	Genode::env()->ram_session()->free(mon_ds_cap);
	Genode::env()->ram_session()->free(dead_ds_cap);
	return _live_data.cap();




/*
	Genode::printf("run parser!\n");

	static Genode::Trace::Connection trace(1024*4096, 64*4096, 0);

	Genode::Trace::Subject_id subjects[32];
	size_t num_subjects = trace.subjects(subjects, 32);
*/

	//Task::log_profile_data(Task::Event::EXTERNAL, -1, _shared);

	// Xml_generator directly writes XML data into the buffer on construction, explaining the heavy recursion here.
	//PDBG("Generating event log. %d events have occurred.", _shared.event_log.size());
/*
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
					xml.attribute("binary", ram_info.thread_name().string());
				});
			}
		});
*/

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
	//});

	//_shared.event_log.clear();

	//return _profile_data.cap();
}

Genode::Number_of_bytes Parser_session_component::_profile_ds_size()
{
	Genode::Xml_node launchpad_node = Genode::config()->xml_node().sub_node("profile");
	return launchpad_node.attribute_value<Genode::Number_of_bytes>("ds-size", 128 * 1024);
}
