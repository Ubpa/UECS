#pragma once

#include <type_traits>

namespace Ubpa::CmptTag {
	template<typename TagedCmpt>
	struct RemoveTag;

	template<typename Cmpt>
	struct RemoveTag<const Cmpt*> {
		using type = Cmpt;
	};

	template<typename Cmpt>
	struct RemoveTag<Cmpt*> {
		using type = Cmpt;
	};

	template<typename Cmpt>
	struct RemoveTag<LastFrame<Cmpt>> {
		using type = Cmpt;
	};

	template<typename TagedCmpt>
	struct IsLastFrame : std::false_type {};

	template<typename Cmpt>
	struct IsLastFrame<LastFrame<Cmpt>> : std::true_type {};

	template<typename TagedCmpt>
	struct IsWrite : std::false_type {};

	template<typename Cmpt>
	struct IsWrite<const Cmpt*> : std::false_type {};

	template<typename Cmpt>
	struct IsWrite<Cmpt*> : std::true_type {};

	template<typename TagedCmpt>
	struct IsNewest : std::false_type {};

	template<typename Cmpt>
	struct IsNewest<const Cmpt*> : std::true_type {};
}
