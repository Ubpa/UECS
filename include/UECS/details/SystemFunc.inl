#pragma once

#include <UTemplate/Func.h>

namespace Ubpa::UECS::details {
	template<typename Func>
	auto Pack(Func&& func) noexcept;
}

namespace Ubpa::UECS {
	// Mode::Entity
	template<typename Func>
	SystemFunc::SystemFunc(
		Func&& func,
		std::string name,
		ArchetypeFilter archetypeFilter,
		CmptLocator cmptLocator,
		SingletonLocator singletonLocator,
		RandomAccessor randomAccessor,
		bool isParallel
	) :
		entityQuery{ std::move(archetypeFilter), std::move(cmptLocator.Combine<decltype(func)>()) },
		singletonLocator{ std::move(singletonLocator.Combine<decltype(func)>()) },
		randomAccessor{std::move(randomAccessor)},
		mode{ Mode::Entity },
		name{ std::move(name) },
		hashCode{ GetValue(this->name) },
		isParallel{ isParallel },
		func{ details::Pack(std::forward<Func>(func)) }
	{
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;

		static_assert(Length_v<Filter_t<ArgList, IsWriteSingleton>> == 0,
			"(Mode::Entity) SystemFunc can't write singletons, use {Latest|LastFrame}Singleton<Cmpt> instead");

		static_assert(!Contain_v<ArgList, ChunkView>);

		assert("(Mode::Entity) SystemFunc can't write singletons, use {Latest|LastFrame}Singleton<Cmpt> instead"
			&& !singletonLocator.HasWriteSingletonType());
	}

	// Mode::Chunk
	template<typename Func>
	SystemFunc::SystemFunc(
		Func&& func,
		std::string name,
		ArchetypeFilter archetypeFilter,
		SingletonLocator singletonLocator,
		RandomAccessor randomAccessor,
		bool isParallel
	) :
		entityQuery{ std::move(archetypeFilter) },
		singletonLocator{ std::move(singletonLocator.Combine<decltype(func)>()) },
		randomAccessor{ std::move(randomAccessor) },
		mode{ Mode::Chunk },
		name{ std::move(name) },
		hashCode{ GetValue(this->name) },
		isParallel{ isParallel },
		func{ details::Pack(std::forward<Func>(func)) }
	{
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;

		static_assert(Length_v<Filter_t<ArgList, IsWriteSingleton>> == 0,
			"(Mode::Chunk) SystemFunc can't write singletons, use {Latest|LastFrame}Singleton<Cmpt>");

		static_assert(Contain_v<ArgList, ChunkView>);

		static_assert(!Contain_v<ArgList, Entity>,
			"(Mode::Chunk) SystemFunc can't use Entity directly, use ChunkView::GetEntityArray()");

		static_assert(!Contain_v<ArgList, CmptsView>,
			"(Mode::Chunk) SystemFunc's argument list cann't have CmptsView");
		
		static_assert(Length_v<Filter_t<ArgList, IsNonSingleton>> == 0,
			"(Mode::Chunk) SystemFunc can't directly access entities' components");

		assert("(Mode::Chunk) SystemFunc can't write singletons, use {Latest|LastFrame}Singleton<Cmpt>"
			&& !singletonLocator.HasWriteSingletonType());
	}

	// Mode::Job
	template<typename Func>
	SystemFunc::SystemFunc(Func&& func, std::string name, SingletonLocator singletonLocator, RandomAccessor randomAccessor)
		:
		singletonLocator{ std::move(singletonLocator.Combine<decltype(func)>()) },
		randomAccessor {std::move(randomAccessor)},
		mode{ Mode::Job },
		name{ std::move(name) },
		hashCode{ GetValue(this->name) },
		isParallel{ false },
		func{ details::Pack(std::forward<Func>(func)) }
	{
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;

		static_assert(Length_v<Filter_t<ArgList, IsNonSingleton>> == 0,
			"(Mode::Job) SystemFunc can't access entities' components");

		static_assert(
			!Contain_v<ArgList, Entity>
			&& !Contain_v<ArgList, std::size_t>
			&& !Contain_v<ArgList, CmptsView>
			&& !Contain_v<ArgList, ChunkView>,
			"(Mode::Job) SystemFunc's argument list cann't have Entity, indexInQuery CmptsView or ChunkView"
			);
	}
}

namespace Ubpa::UECS::details {
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
				std::size_t entityIndexInQuery,
				CmptsView cmpts,
				ChunkView chunkView)
			{
				auto args = std::tuple{
					w,
					singletons,
					reinterpret_cast<Singletons*>(singletons.Singletons()[Find_v<SingletonList, Singletons>].Ptr())...,
					e,
					entityIndexInQuery,
					cmpts,
					reinterpret_cast<NonSingletons*>(cmpts.Components()[Find_v<NonSingletonList, NonSingletons>].Ptr())...,
					chunkView,
				};
				func(std::get<DecayedArgs>(args)...);
			};
		}
	};

	template<typename Func>
	auto Pack(Func&& func) noexcept {
		using ArgList = FuncTraits_ArgList<Func>;

		using DecayedArgList = Transform_t<ArgList, DecayArg>;
		static_assert(IsUnique_v<DecayedArgList>, "details::System_::Pack: <Func>'s argument types must be a set");

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
