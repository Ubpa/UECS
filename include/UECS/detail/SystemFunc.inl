#pragma once

#include <_deps/nameof.hpp>

#include <UTemplate/Func.h>

namespace Ubpa::UECS::detail {
	template<typename Func>
	auto Pack(Func&& func) noexcept;
	template<typename Func>
	constexpr CmptLocator GenCmptLocator() noexcept;
	template<typename Func>
	constexpr SingletonLocator GenSingletonLocator() noexcept;
}

namespace Ubpa::UECS {
	template<typename Func>
	SystemFunc::SystemFunc(Func&& func, std::string name, ArchetypeFilter archetypeFilter)
		:
		func{ detail::Pack(std::forward<Func>(func)) },
		entityQuery{ std::move(archetypeFilter), detail::GenCmptLocator<decltype(func)>() },
		singletonLocator{ detail::GenSingletonLocator<decltype(func)>() },
		name{ std::move(name) },
		hashCode{ HashCode(this->name) }
	{
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;
		static_assert(!Contain_v<ArgList, CmptsView> && !Contain_v<ArgList, SingletonsView>);

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
}

namespace Ubpa::UECS::detail {
	template<typename DecayedArgList, typename SortedSingletonList, typename SortedNonSingletonList>
	struct Packer;

	template<typename... DecayedArgs, typename... Singletons, typename... NonSingletons>
	struct Packer<TypeList<DecayedArgs...>, TypeList<Singletons...>, TypeList<NonSingletons...>> {
		using SortedSingletonList = TypeList<Singletons...>; // sorted
		using SortedNonSingletonList = TypeList<NonSingletons...>; // sorted
		template<typename Func>
		static auto run(Func&& func) noexcept {
			return [func = std::forward<Func>(func)](
				World* w,
				SingletonsView singletonsView,
				Entity e,
				size_t entityIndexInQuery,
				CmptsView cmptsView,
				ChunkView chunkView)
			{
				auto args = std::tuple{
					w,
					reinterpret_cast<Singletons*>(singletonsView.Singletons()[Find_v<SortedSingletonList, Singletons>].Ptr())...,
					e,
					entityIndexInQuery,
					cmptsView,
					chunkView,
					reinterpret_cast<NonSingletons*>(cmptsView.Components()[Find_v<SortedNonSingletonList, NonSingletons>].Ptr())...
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

	// ====================

	template<typename... Cmpts>
	constexpr CmptLocator GenCmptLocator(TypeList<Cmpts...>) noexcept {
		if constexpr (sizeof...(Cmpts) > 0) {
			constexpr std::array<CmptType, sizeof...(Cmpts)> types{ CmptType::Of<Cmpts>... };
			return CmptLocator{ types.data(), types.size() };
		}
		else
			return CmptLocator{};
	}

	template<typename Func>
	constexpr CmptLocator GenCmptLocator() noexcept {
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;
		using CmptList = Filter_t<ArgList, IsNonSingleton>;
		return GenCmptLocator(CmptList{});
	}

	// ====================

	template<typename... Singletons>
	constexpr SingletonLocator GenSingletonLocator(TypeList<Singletons...>) noexcept {
		if constexpr (sizeof...(Singletons) > 0) {
			constexpr std::array<CmptType, sizeof...(Singletons)> types{ CmptType::Of<Singletons>... };
			return SingletonLocator{ types.data(), types.size() };
		}
		else
			return SingletonLocator{};
	}

	template<typename Func>
	constexpr SingletonLocator GenSingletonLocator() noexcept {
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;
		using SingletonList = Filter_t<ArgList, IsSingleton>;
		return GenSingletonLocator(SingletonList{});
	}
}
