#pragma once

#include "details/SystemFunc.h"
#include "details/Job.h"

#include <map>
#include <memory>
#include <memory_resource>

namespace Ubpa::UECS::details {
	struct Compiler;
}

namespace Ubpa::UECS {
	class EntityMngr;
	class SystemMngr;
	class SysFuncGraph;

	// [description]
	// system infomation record
	// - SystemFunc
	// - orders
	// - dynamic filter changes
	// [detail]
	// schedule will be clear at the beginning of the next World::Update()
	class Schedule {
	public:
		// Func's argument list:
		// [const] World*
		// {LastFrame|Latest}<Singleton<Cmpt>>
		// SingletonsView
		// Entity
		// std::size_t indexInQuery
		// <tagged-components>: {LastFrame|Write|Latest}<Cmpt>...
		// CmptsView
		template<typename Func>
		const SystemFunc* RegisterEntityJob(
			Func&&,
			std::string name,
			bool isParallel = true,
			ArchetypeFilter = {},
			CmptLocator = {},
			SingletonLocator = {},
			RandomAccessor = {}
		);

		// Func's argument list:
		// [const] World*
		// {LastFrame|Latest}<Singleton<Cmpt>>
		// SingletonsView
		// ChunkView (necessary)
		template<typename Func>
		const SystemFunc* RegisterChunkJob(
			Func&&,
			std::string name,
			ArchetypeFilter = {},
			bool isParallel = true,
			SingletonLocator = {},
			RandomAccessor = {}
		);

		// Func's argument list:
		// [const] World*
		// {LastFrame|Write|Latest}<Singleton<Cmpt>>
		// SingletonsView
		template<typename Func>
		const SystemFunc* RegisterJob(
			Func&&,
			std::string name,
			SingletonLocator = {},
			RandomAccessor = {}
		);

		void RegisterCommand(std::function<void(World*)> command, int layer = 0) {
			commandBuffer[layer].push_back(std::move(command));
		}

		Schedule& Order(std::string_view x, std::string_view y);

		Schedule& InsertNone(std::string_view sys, TypeID);
		Schedule& EraseNone(std::string_view sys, TypeID);

		std::pmr::monotonic_buffer_resource* GetFrameMonotonicResource() { return &frame_rsrc; }

		~Schedule();
	private:
		template<typename... Args>
		const SystemFunc* Request(Args&&...);

		void Clear();

		struct CmptSysFuncs {
			std::vector<SystemFunc*> lastFrameSysFuncs;
			std::vector<SystemFunc*> writeSysFuncs;
			std::vector<SystemFunc*> latestSysFuncs;
		};
		friend struct details::Compiler;
		std::unordered_map<TypeID, CmptSysFuncs> GenCmptSysFuncsMap() const;

		SysFuncGraph GenSysFuncGraph() const;

		// SystemFunc's hashcode to pointer of SystemFunc
		std::unordered_map<std::size_t, SystemFunc*> sysFuncs;

		// SystemFunc's hashcode to SystemFunc's hashcode
		// parent to children
		std::unordered_map<std::size_t, std::size_t> sysFuncOrder;

		struct FilterChange {
			std::set<TypeID> insertNones;
			std::set<TypeID> eraseNones;
		};
		std::unordered_map<std::size_t, FilterChange> sysFilterChange;

		std::map<int, std::vector<std::function<void(World*)>>> commandBuffer;

		std::pmr::monotonic_buffer_resource frame_rsrc; // release in every frame

		std::pmr::unsynchronized_pool_resource sysfuncRsrc;
		std::pmr::polymorphic_allocator<SystemFunc> GetSysFuncAllocator();

		friend class World;
	};
}

#include "details/Schedule.inl"
