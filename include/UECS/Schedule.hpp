#pragma once

#include "SystemFunc.hpp"
#include "details/Job.hpp"

#include <map>
#include <memory>
#include <memory_resource>

#include <USmallFlat/pmr/small_vector.hpp>

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
			std::string_view name,
			bool isParallel = true,
			ArchetypeFilter = {},
			CmptLocator = {},
			SingletonLocator = {},
			RandomAccessor = {},
			ChangeFilter = {}
		);

		// Func's argument list:
		// [const] World*
		// {LastFrame|Latest}<Singleton<Cmpt>>
		// SingletonsView
		// ChunkView (necessary)
		template<typename Func>
		const SystemFunc* RegisterChunkJob(
			Func&&,
			std::string_view name,
			ArchetypeFilter = {},
			bool isParallel = true,
			SingletonLocator = {},
			RandomAccessor = {},
			ChangeFilter = {}
		);

		// Func's argument list:
		// [const] World*
		// {LastFrame|Write|Latest}<Singleton<Cmpt>>
		// SingletonsView
		template<typename Func>
		const SystemFunc* RegisterJob(
			Func&&,
			std::string_view name,
			SingletonLocator = {},
			RandomAccessor = {}
		);

		struct EntityJobConfig {
			bool isParallel{ true };
			ArchetypeFilter archetypeFilter;
			CmptLocator cmptLocator;
			SingletonLocator singletonLocator;
			RandomAccessor randomAccessor;
			ChangeFilter changeFilter;
		};

		struct ChunkJobConfig {
			bool isParallel{ true };
			ArchetypeFilter archetypeFilter;
			SingletonLocator singletonLocator;
			RandomAccessor randomAccessor;
			ChangeFilter changeFilter;
		};

		template<typename Func>
		const SystemFunc* RegisterEntityJob(
			Func&&,
			std::string_view name,
			EntityJobConfig config
		);

		template<typename Func>
		const SystemFunc* RegisterChunkJob(
			Func&&,
			std::string_view name,
			ChunkJobConfig config
		);

		Schedule& RegisterCommand(std::function<void(World*)> command, int layer = 0) {
			commandBuffer[layer].push_back(std::move(command));
			return *this;
		}

		Schedule& Order(std::string_view x, std::string_view y);

		Schedule& AddNone(std::string_view sys, TypeID);
		Schedule& Disable(std::string_view sys);

		// clear every frame
		std::pmr::monotonic_buffer_resource* GetFrameMonotonicResource() { return &frame_rsrc; }
		template<typename T, typename... Args>
		T* CreateFrameObject(Args&&... args) const;

		~Schedule();
	private:
		template<typename... Args>
		const SystemFunc* Request(Args&&...);

		void Clear();

		struct CmptSysFuncs {
			CmptSysFuncs(std::pmr::memory_resource* rsrc) :
				lastFrameSysFuncs{ std::pmr::vector<SystemFunc*>(std::pmr::polymorphic_allocator<SystemFunc*>{rsrc}) },
				writeSysFuncs{ std::pmr::vector<SystemFunc*>(std::pmr::polymorphic_allocator<SystemFunc*>{rsrc}) },
				latestSysFuncs{ std::pmr::vector<SystemFunc*>(std::pmr::polymorphic_allocator<SystemFunc*>{rsrc}) } {}

			pmr::small_vector<SystemFunc*> lastFrameSysFuncs;
			pmr::small_vector<SystemFunc*> writeSysFuncs;
			pmr::small_vector<SystemFunc*> latestSysFuncs;
		};
		friend struct details::Compiler;

		using CmptSysFuncsMap = std::pmr::unordered_map<TypeID, CmptSysFuncs>;

		// use frame_rsrc, so no need to delete
		CmptSysFuncsMap* GenCmptSysFuncsMap() const;

		// use frame_rsrc, so no need to delete
		SysFuncGraph* GenSysFuncGraph() const;

		// SystemFunc's hashcode to pointer of SystemFunc
		std::unordered_map<std::size_t, SystemFunc*> sysFuncs;

		std::unordered_set<std::size_t> disabledSysFuncs;

		// SystemFunc's hashcode to SystemFunc's hashcode
		// parent to children
		std::unordered_map<std::size_t, std::size_t> sysFuncOrder;

		std::unordered_map<std::size_t, small_vector<TypeID>> sysNones;

		std::map<int, std::vector<std::function<void(World*)>>> commandBuffer;

		mutable std::pmr::monotonic_buffer_resource frame_rsrc; // release in every frame
		std::string_view RegisterFrameString(std::string_view str);

		friend class World;
	};
}

#include "details/Schedule.inl"
