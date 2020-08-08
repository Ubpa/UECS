#pragma once

#include <_deps/nameof.hpp>

#include <UTemplate/Func.h>

namespace Ubpa::UECS::detail {
	template<typename Func>
	auto Pack(Func&& func) noexcept;
}

namespace Ubpa::UECS {
	template<typename Func>
	SystemFunc::SystemFunc(
		Func&& func,
		std::string name,
		ArchetypeFilter archetypeFilter,
		SingletonLocator singletonLocator
	) :
		func{ detail::Pack(std::forward<Func>(func)) },
		entityQuery{ std::move(archetypeFilter), CmptLocator::Generate<decltype(func)>() },
		singletonLocator{ std::move(singletonLocator.Combine<decltype(func)>()) },
		name{ std::move(name) },
		hashCode{ HashCode(this->name) }
	{
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;
		static_assert(!Contain_v<ArgList, CmptsView>);

		if constexpr (Length_v<Filter_t<ArgList, IsNonSingleton>> > 0) {
			static_assert(!Contain_v<ArgList, ChunkView>);
			mode = Mode::Entity;
		}
		else {
			static_assert(!Contain_v<ArgList, Entity> && !Contain_v<ArgList, size_t>);
			if constexpr (Contain_v<ArgList, ChunkView>)
				mode = Mode::Chunk;
			else
				mode = Mode::Job;
		}
	}

	template<typename Func>
	SystemFunc::SystemFunc(
		Func&& func,
		std::string name,
		CmptLocator cmptLocator,
		ArchetypeFilter archetypeFilter,
		SingletonLocator singletonLocator
	) :
		mode{ Mode::Entity },
		func{ detail::Pack(std::forward<Func>(func)) },
		entityQuery{ std::move(archetypeFilter), std::move(cmptLocator.Combine<decltype(func)>()) },
		singletonLocator{ std::move(singletonLocator.Combine<decltype(func)>()) },
		name{ std::move(name) },
		hashCode{ HashCode(this->name) }
	{
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;
		static_assert(!Contain_v<ArgList, ChunkView>);
		assert(!entityQuery.locator.CmptTypes().empty());
	}
}

namespace Ubpa::UECS::detail {
	template<typename DecayedArgList, typename SortedSingletonList, typename SortedNonSingletonList>
	struct Packer;

	template<typename... DecayedArgs, typename... Singletons, typename... NonSingletons>
	struct Packer<TypeList<DecayedArgs...>, TypeList<Singletons...>, TypeList<NonSingletons...>> {
		using SingletonList = TypeList<Singletons...>; // sorted
		using NonSingletonList = TypeList<NonSingletons...>; // sorted
		template<typename Func>
		static auto run(Func&& func) noexcept {
			return [func = std::forward<Func>(func)](
				World* w,
				SingletonsView singletons,
				Entity e,
				size_t entityIndexInQuery,
				CmptsView cmpts,
				ChunkView chunkView)
			{
				auto args = std::tuple{
					w,
					reinterpret_cast<Singletons*>(singletons.Singletons()[Find_v<SingletonList, Singletons>].Ptr())...,
					e,
					entityIndexInQuery,
					cmpts,
					chunkView,
					reinterpret_cast<NonSingletons*>(cmpts.Components()[Find_v<NonSingletonList, NonSingletons>].Ptr())...
				};
				func(std::get<DecayedArgs>(args)...);
			};
		}
	};

	template<typename Func>
	auto Pack(Func&& func) noexcept {
		using ArgList = FuncTraits_ArgList<Func>;

		using DecayedArgList = Transform_t<ArgList, DecayTag>;
		static_assert(IsSet_v<DecayedArgList>, "detail::System_::Pack: <Func>'s argument types must be a set");

		using TaggedCmptList = Filter_t<ArgList, IsTaggedCmpt>;

		using TaggedSingletonList = Filter_t<TaggedCmptList, IsSingleton>;
		using TaggedNonSingletonList = Filter_t<TaggedCmptList, IsNonSingleton>;

		using SingletonList = Transform_t<TaggedSingletonList, RemoveTag>;
		using NonSingletonList = Transform_t<TaggedNonSingletonList, RemoveTag>;

		using SortedSingletonList = QuickSort_t<SingletonList, TypeID_Less>;
		using SortedNonSingletonList = QuickSort_t<NonSingletonList, TypeID_Less>;

		return Packer<DecayedArgList, SortedSingletonList, SortedNonSingletonList>::run(std::forward<Func>(func));
	}
}
