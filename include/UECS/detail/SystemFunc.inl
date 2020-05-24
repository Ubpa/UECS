#pragma once

#include <_deps/nameof.hpp>

namespace Ubpa::detail::System_ {
	template<typename Func>
	auto Pack(Func&& func) noexcept;
}

namespace Ubpa {
	template<typename Func>
	SystemFunc::SystemFunc(Func&& func, std::string name, EntityLocator locator, EntityFilter filter)
		: isJob{ IsEmpty_v<FuncTraits_ArgList<Func>> },
		func{ detail::System_::Pack(std::forward<Func>(func)) },
		name{ std::move(name) },
		hashCode{ HashCode(this->name) },
		query{ std::move(filter), std::move(locator) }
	{
		using ArgList = FuncTraits_ArgList<Func>;

		static_assert(ContainTs_v<ArgList, RTDCmptViewer>,
			"<Func>'s argument must contain RTDCmptViewer");
	}

	template<typename Func>
	SystemFunc::SystemFunc(Func&& func, std::string name, EntityFilter filter)
		: SystemFunc(std::forward<Func>(func), std::move(name), std::move(filter), FuncTraits_ArgList<Func>{})
	{
	}


	template<typename Func, typename ArgList>
	SystemFunc::SystemFunc(Func&& func, std::string name, EntityFilter filter, ArgList)
		: isJob{ IsEmpty_v<ArgList> },
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
			return [func = std::forward<Func>(func)](Entity e, size_t entityIndexInQuery, RTDCmptViewer rtdcmpts) {
				auto unsorted_arg_tuple = std::make_tuple(e, entityIndexInQuery, rtdcmpts, reinterpret_cast<Cmpts*>(rtdcmpts.Components()[Find_v<CmptList, Cmpts>])...);
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
