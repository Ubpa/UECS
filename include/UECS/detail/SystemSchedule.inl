#pragma once

#include "../SystemSchedule.h"

#include <UTemplate/Func.h>

namespace Ubpa::detail::SystemSchedule_ {
	template<typename Func>
	struct GenSystem;
}

namespace Ubpa {
	template<SysType type>
	template<typename Func>
	SystemSchedule<type>& SystemSchedule<type>::Register(const std::string& name, Func&& func) {
		detail::SystemSchedule_::Schedule<type, FuncTraits_ArgList<std::remove_reference_t<Func>>>::run(this, std::forward<Func>(func), name);
		return *this;
	}

	template<SysType type>
	template<typename Cmpt, typename Func>
	SystemSchedule<type>& SystemSchedule<type>::Register(const std::string& name, Func Cmpt::* func) {
		Register(name, detail::SystemSchedule_::GenSystem<Func Cmpt::*>::run(func));
		return *this;
	}

	template<SysType type>
	template<typename Cmpt, typename Func>
	SystemSchedule<type>& SystemSchedule<type>::Register(Func Cmpt::* func) {
		Register(std::string(nameof::nameof_type<Func Cmpt::*>()), func);
		return *this;
	}

	template<SysType type>
	template<typename Cmpt>
	SystemSchedule<type>& SystemSchedule<type>::Register() {
		static_assert(HaveCmptSys<Cmpt, type>, "<Cmpt> have no corresponding System (OnStart/OnUpdate/OnStop)");
		Register(GetCmptSys<Cmpt, type>());
		return *this;
	}

	template<SysType type>
	SystemSchedule<type>& SystemSchedule<type>::Order(std::string_view first, const std::string& second) {
		auto target = sysOrderMap.find(first);
		if (target != sysOrderMap.end())
			target->second.insert(second);
		else
			sysOrderMap.emplace(std::string(first), std::set<std::string>{second});

		return *this;
	}

	template<SysType type>
	template<typename CmptFirst, typename CmptSecond>
	SystemSchedule<type>& SystemSchedule<type>::Order() {
		return Order(DefaultSysName<CmptFirst, type>(), std::string(DefaultSysName<CmptSecond, type>()));
	}

	template<SysType type>
	template<typename Cmpt>
	SystemSchedule<type>& SystemSchedule<type>::Before(std::string_view name) {
		return Order(name, std::string(DefaultSysName<Cmpt, type>()));
	}

	template<SysType type>
	template<typename Cmpt>
	SystemSchedule<type>& SystemSchedule<type>::After(const std::string& name) {
		return Order(DefaultSysName<Cmpt, type>(), name);
	}

	// ==============================================================================================

	template<SysType type>
	Job* SystemSchedule<type>::RequestJob(const std::string& name) {
		Job* sys = jobPool.Request(name);
		jobs[name] = sys;
		return sys;
	}

	template<SysType type>
	SystemSchedule<type>::SystemSchedule(ArchetypeMngr* mngr)
		: mngr{ mngr } {}

	template<SysType type>
	SystemSchedule<type>::~SystemSchedule() {
		Clear();
	}

	template<SysType type>
	void SystemSchedule<type>::Clear() {
		for (auto [name, sys] : jobs)
			jobPool.Recycle(sys);
		jobs.clear();
		id2rw.clear();
		sysOrderMap.clear();
	}

	template<SysType type>
	bool SystemSchedule<type>::GenTaskflow(tf::Taskflow& taskflow) const {
		if (!IsDAG())
			return false;

		std::unordered_map<Job*, tf::Task> sys2task;

		for (auto& [ID, rw] : id2rw) {
			// toposort
			auto writers = rw.writers; // copy
			std::vector<Job*> sortedSystems; // last to first
			while (!writers.empty()) {
				auto iter_begin = writers.begin();
				std::deque<Job*> childrenQueue; // FIFO, push_back, pop_front
				childrenQueue.push_back(*iter_begin);

				std::vector<Job*> waitedSystems;

				while (!childrenQueue.empty()) {
					auto writer = childrenQueue.front();
					childrenQueue.pop_front();

					if (writers.find(writer) == writers.end())
						continue;

					waitedSystems.push_back(writer);
					writers.erase(writer); // directly change writers

					auto target_children = sysOrderMap.find(writer->name());
					if (target_children == sysOrderMap.end())
						continue;

					for (const auto& sys_name : target_children->second)
						childrenQueue.push_back(jobs.find(sys_name)->second);
				}

				for (auto iter = waitedSystems.rbegin(); iter != waitedSystems.rend(); ++iter)
					sortedSystems.push_back(*iter);
			}

			std::vector<tf::Task> writerTasks;
			for (size_t i = 0; i < sortedSystems.size(); i++) {
				tf::Task task;

				auto target = sys2task.find(sortedSystems[i]);
				if (target == sys2task.end()) {
					task = taskflow.composed_of(*sortedSystems[i]);
					sys2task[sortedSystems[i]] = task;
				}
				else
					task = target->second;

				if (!writerTasks.empty())
					task.precede(writerTasks.back());

				writerTasks.push_back(task);
			}

			for (auto pre_reader : rw.pre_readers) {
				tf::Task task;
				auto target = sys2task.find(pre_reader);
				if (target == sys2task.end()) {
					task = taskflow.composed_of(*pre_reader);
					sys2task[pre_reader] = task;
				}
				else
					task = target->second;

				if (!writerTasks.empty())
					task.precede(writerTasks.back());
			}

			for (auto post_reader : rw.post_readers) {
				tf::Task task;
				auto target = sys2task.find(post_reader);
				if (target == sys2task.end()) {
					task = taskflow.composed_of(*post_reader);
					sys2task[post_reader] = task;
				}
				else
					task = target->second;

				if (!writerTasks.empty())
					task.succeed(writerTasks.front());
			}
		}

		return true;
	}

	template<SysType type>
	bool SystemSchedule<type>::IsDAG() const noexcept {
		std::map<Job*, std::set<Job*>> graph;
		for (const auto& [ID, rw] : id2rw) {
			for (const auto& writer : rw.writers) {
				for (auto post_reader : rw.post_readers) {
					if (post_reader != writer)
						graph[writer].insert(post_reader);
				}
				for (auto pre_reader : rw.pre_readers) {
					if (pre_reader != writer)
						graph[pre_reader].insert(writer);
				}
			}
		}

		std::set<Job*> visited;
		for (const auto& [parent, children] : graph) {
			if (visited.find(parent) != visited.end())
				continue;
			std::stack<Job*> sysStack;
			sysStack.push(parent);
			while (!sysStack.empty()) {
				auto curSys = sysStack.top();
				sysStack.pop();
				visited.insert(curSys);
				auto target = graph.find(curSys);
				if (target != graph.end()) {
					for (const auto& child : target->second) {
						if (child == parent)
							return false;
						if (visited.find(curSys) == visited.end())
							sysStack.push(child);
					}
				}
			}
		}
		return true;
	}
}

namespace Ubpa::detail::SystemSchedule_ {
	template<SysType type, typename... TagedCmpts>
	struct Schedule<type, TypeList<TagedCmpts...>> {
		template<typename Func>
		static auto run(SystemSchedule<type>* sysSchedule, Func&& func, const std::string& name) noexcept {
			auto system = sysSchedule->RequestJob(name);
			sysSchedule->mngr->GenTaskflow(system, func);
			if(!system->empty())
				(Register<TagedCmpts>(sysSchedule, system), ...);
		}

		template<typename TagedCmpt>
		static void Register(SystemSchedule<type>* sysSchedule, tf::Taskflow* system) {
			if constexpr (CmptTag::IsLastFrame_v<TagedCmpt>)
				sysSchedule->id2rw[Ubpa::TypeID<CmptTag::RemoveTag_t<TagedCmpt>>].pre_readers.push_back(system);
			else if constexpr (CmptTag::IsWrite_v<TagedCmpt>)
				sysSchedule->id2rw[Ubpa::TypeID<CmptTag::RemoveTag_t<TagedCmpt>>].writers.insert(system);
			else if constexpr (CmptTag::IsNewest_v<TagedCmpt>)
				sysSchedule->id2rw[Ubpa::TypeID<CmptTag::RemoveTag_t<TagedCmpt>>].post_readers.push_back(system);
			else if constexpr (CmptTag::IsBefore_v<TagedCmpt>)
				RegisterBefore(sysSchedule, system, typename TagedCmpt::CmptList{});
			else // if constexpr (CmptTag::IsAfter_v<TagedCmpt>)
				RegisterAfter(sysSchedule, system, typename TagedCmpt::CmptList{});
		}

		template<typename... Cmpts>
		static void RegisterBefore(SystemSchedule<type>* sysSchedule, tf::Taskflow* system, TypeList<Cmpts...>) {
			(sysSchedule->sysOrderMap[system->name()].insert(std::string(DefaultSysName<Cmpts, type>())), ...);
		}

		template<typename... Cmpts>
		static void RegisterAfter(SystemSchedule<type>* sysSchedule, tf::Taskflow* system, TypeList<Cmpts...>) {
			(sysSchedule->sysOrderMap[std::string(DefaultSysName<Cmpts, type>())].insert(system->name()), ...);
		}
	};
}

namespace Ubpa::detail::SystemSchedule_ {
	template<typename Cmpt, typename ArgList>
	struct GenSystemHelper;

	template<typename Cmpt, typename... Cmpts>
	struct GenSystemHelper<Cmpt, TypeList<Cmpts...>> {
		template<typename Func>
		static auto run(Func func) noexcept {
			if constexpr (FuncTraits<std::decay_t<Func>>::is_const) {
				return [func](const Cmpt* cmpt, Cmpts... cmpts) {
					(cmpt->*func)(cmpts...);
				};
			}
			else {
				return [func](Cmpt* cmpt, Cmpts... cmpts) {
					(cmpt->*func)(cmpts...);
				};
			}
		}
	};

	template<typename Func>
	struct GenSystem {
		static auto run(Func func) noexcept {
			return GenSystemHelper<FuncTraits_Obj<std::decay_t<Func>>, FuncTraits_ArgList<std::decay_t<Func>>>::run(std::forward<Func>(func));
		}
	};
}
