#pragma once

#include <_deps/nameof.hpp>

namespace Ubpa::detail::System_ {
	template<typename Func>
	auto Pack(Func&& func) noexcept;
}

namespace Ubpa {
	template<typename Func>
	SystemFunc::SystemFunc(Func&& func, EntityFilter filter)
		: SystemFunc{ std::string(nameof::nameof_type<Func>()), std::forward<Func>(func), std::move(filter) } {}

	template<typename Func>
	SystemFunc::SystemFunc(Func&& func, std::string name, EntityFilter filter)
		: SystemFunc(std::forward<Func>(func), std::move(name), std::move(filter), FuncTraits_ArgList<Func>{})
	{
	}

	template<typename Func, typename ArgList>
	SystemFunc::SystemFunc(Func&& func, std::string name, EntityFilter filter, ArgList)
		: needEntity{ Contain_v<ArgList, Entity*>|| Contain_v<ArgList, const Entity*> },
		func{ detail::System_::Pack(std::forward<Func>(func)) }, 
		name{ std::move(name) },
		hashCode{ HashCode(this->name) },
		query{ std::move(filter), EntityLocator{Filter_t<ArgList, CmptTag::IsTaggedCmpt>{}} }
	{
	}
}

namespace Ubpa::detail::System_ {
	template<typename DecayedArgList, typename SortedCmptList>
	struct Packer;

	template<typename... DecayedArgs, typename... Cmpts>
	struct Packer<TypeList<DecayedArgs...>, TypeList<Cmpts...>> {
		using CmptList = TypeList<Cmpts...>;
		template<typename Func>
		static auto run(Func&& func) noexcept {
			return [func = std::forward<Func>(func)](Entity* e, void** cmpt_arr) {
				auto unsorted_arg_tuple = std::make_tuple(reinterpret_cast<Cmpts*>(cmpt_arr[Find_v<CmptList, Cmpts>])..., e);
				func(std::get<DecayedArgs>(unsorted_arg_tuple)...);
			};
		}
	};

	template<typename Func>
	auto Pack(Func&& func) noexcept {
		using ArgList = FuncTraits_ArgList<Func>;

		using DecayedArgList = Transform_t<ArgList, CmptTag::DecayTag>;
		static_assert(IsSet_v<DecayedArgList>, "detail::System_::Pack: <Func>'s argument types must be a set");

		using TaggedCmptList = Filter_t<ArgList, CmptTag::IsTaggedCmpt>;
		using CmptList = Transform_t<TaggedCmptList, CmptTag::RemoveTag>;
		using SortedCmptList = QuickSort_t<CmptList, TypeID_Less>;

		return Packer<DecayedArgList, SortedCmptList>::run(std::forward<Func>(func));
	}
}