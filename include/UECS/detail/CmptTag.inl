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

	template<typename TagedCmpt> struct IsBefore : std::false_type {};
	template<typename... Cmpts> struct IsBefore<Before<Cmpts...>> : std::true_type {};

	template<typename TagedCmpt> struct IsAfter : std::false_type {};
	template<typename... Cmpts> struct IsAfter<After<Cmpts...>> : std::true_type {};
}
