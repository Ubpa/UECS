#pragma once

#include <type_traits>

namespace Ubpa::CmptTag {
	template<typename Cmpt> struct RemoveTag<const Cmpt*> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Cmpt*> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<LastFrame<Cmpt>> : IType<Cmpt> {};

	template<typename TagedCmpt> struct IsLastFrame : std::false_type {};
	template<typename Cmpt> struct IsLastFrame<LastFrame<Cmpt>> : std::true_type {};

	template<typename TagedCmpt> struct IsWrite : std::false_type {};
	template<typename Cmpt> struct IsWrite<const Cmpt*> : std::false_type {};
	template<typename Cmpt> struct IsWrite<Cmpt*> : std::true_type {};

	template<typename TagedCmpt> struct IsNewest : std::false_type {};
	template<typename Cmpt> struct IsNewest<const Cmpt*> : std::true_type {};

	template<typename TagedCmpt> struct IsTimePoint :
		IValue<bool, IsWrite_v<TagedCmpt> || IsLastFrame_v<TagedCmpt> || IsNewest_v<TagedCmpt>> {};

	template<typename TagedCmpt> struct IsBefore : std::false_type {};
	template<typename... Cmpts> struct IsBefore<Before<Cmpts...>> : std::true_type {};

	template<typename TagedCmpt> struct IsAfter : std::false_type {};
	template<typename... Cmpts> struct IsAfter<After<Cmpts...>> : std::true_type {};

	template<typename T> struct IsNot : std::false_type {};
	template<typename... Cmpts> struct IsNot<Not<Cmpts...>> : std::true_type {};

	namespace detail::CmptTag_ {
		template<typename I, typename X>
		struct AccNot : IType<I> {};
		template<typename I, typename... NotCmpts>
		struct AccNot<I, Not<NotCmpts...>> : Concat<I, TypeList<NotCmpts...>> {};
	}

	template<typename ArgList>
	struct GetAllNotList : Accumulate<ArgList, detail::CmptTag_::AccNot, TypeList<>> {};
}
