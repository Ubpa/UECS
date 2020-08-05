#pragma once

#include <_deps/nameof.hpp>

namespace Ubpa::UECS::detail::System_ {
	template<typename Func>
	auto Pack(Func&& func) noexcept;
}

namespace Ubpa::UECS {
	template<typename Func>
	SystemFunc::SystemFunc(Func&& func, std::string name, EntityLocator locator, EntityFilter filter)
		: mode{ Mode::Entity },
		func{ detail::System_::Pack(std::forward<Func>(func)) },
		name{ std::move(name) },
		hashCode{ HashCode(this->name) },
		query{ std::move(filter), std::move(locator) }
	{
		using ArgList = FuncTraits_ArgList<Func>;
		using DecayedArgList = Transform_t<ArgList, DecayTag>;

		static_assert(Contain_v<DecayedArgList, CmptsView>,
			"(Mode::Entity) <Func>'s argument list must contain [const] CmptsView");

		static_assert(!Contain_v<DecayedArgList, ChunkView>,
			"(Mode::Entity) <Func>'s argument list must not contain [const] ChunkView");
	}

	template<typename Func>
	SystemFunc::SystemFunc(Func&& func, std::string name, EntityFilter filter)
		: SystemFunc(std::forward<Func>(func), std::move(name), std::move(filter), FuncTraits_ArgList<Func>{})
	{
	}


	template<typename Func, typename ArgList>
	SystemFunc::SystemFunc(Func&& func, std::string name, EntityFilter filter, ArgList)
		:
		func{ detail::System_::Pack(std::forward<Func>(func)) }, 
		name{ std::move(name) },
		hashCode{ HashCode(this->name) },
		query{ std::move(filter), EntityLocator{Filter_t<ArgList, IsTaggedCmpt>{}} }
	{
		using DecayedArgList = Transform_t<ArgList, DecayTag>;

		static_assert(!Contain_v<DecayedArgList, CmptsView>,
			"<Func>'s argument list contains CmptsView, so you should use the constructor of the run-time dynamic version");

		if constexpr (
			IsEmpty_v<DecayedArgList>
			|| Length_v<DecayedArgList> == 1 && Contain_v<DecayedArgList, World*>
		) {
			// [[const] World*]
			mode = Mode::Job;
		}
		else if constexpr (
			Contain_v<DecayedArgList, ChunkView>
			&& (
				Length_v<DecayedArgList> == 1
				|| Length_v<DecayedArgList> == 2 && Contain_v<DecayedArgList, World*>
			)
		) {
			// [[const] World*]
			// [const] ChunkView
			mode = Mode::Chunk;
		}
		else {
			// default
			static_assert(!Contain_v<ArgList, ChunkView>,
				"(Mode::Entity) <Func>'s argument list must not contain ChunkView");
			mode = Mode::Entity;
		}
	}
}

namespace Ubpa::UECS::detail::System_ {
	template<typename DecayedArgList, typename SortedCmptList>
	struct Packer;

	template<typename... DecayedArgs, typename... Cmpts>
	struct Packer<TypeList<DecayedArgs...>, TypeList<Cmpts...>> {
		using CmptList = TypeList<Cmpts...>; // sorted
		template<typename Func>
		static auto run(Func&& func) noexcept {
			return [func = std::forward<Func>(func)](World* w, Entity e, size_t entityIndexInQuery, CmptsView rtdcmpts, ChunkView chunkView) {
				auto unsorted_arg_tuple = std::make_tuple(
					w,
					e,
					entityIndexInQuery,
					rtdcmpts,
					chunkView,
					reinterpret_cast<Cmpts*>(rtdcmpts.Components()[Find_v<CmptList, Cmpts>])...
				);
				func(std::get<DecayedArgs>(unsorted_arg_tuple)...);
			};
		}
	};

	template<typename Func>
	auto Pack(Func&& func) noexcept {
		using ArgList = FuncTraits_ArgList<Func>;

		using DecayedArgList = Transform_t<ArgList, DecayTag>;
		static_assert(IsSet_v<DecayedArgList>, "detail::System_::Pack: <Func>'s argument types must be a set");

		using TaggedCmptList = Filter_t<ArgList, IsTaggedCmpt>;

		using CmptList = Transform_t<TaggedCmptList, RemoveTag>;
		using SortedCmptList = QuickSort_t<CmptList, TypeID_Less>;

		return Packer<DecayedArgList, SortedCmptList>::run(std::forward<Func>(func));
	}
}
