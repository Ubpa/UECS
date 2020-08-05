#pragma once

#include <type_traits>

namespace Ubpa::UECS {
	class Entity;
	class EntityLocator;
	class World;
}

namespace Ubpa::UECS {
	template<typename Cmpt> struct RemoveTag : IType<Cmpt> {}; // default
	template<typename Cmpt> struct RemoveTag<const Cmpt*> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Cmpt*> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<LastFrame<Cmpt>> : IType<Cmpt> {};

	template<typename T> struct DecayTag : IType<T> {};
	template<typename Cmpt> struct DecayTag<const Cmpt*> : IType<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<LastFrame<Cmpt>> : IType<Cmpt*> {};
	template<typename T> struct DecayTag<const T> : IType<T> {};

	template<typename TaggedCmpt> struct IsLastFrame : std::false_type {};
	template<typename Cmpt> struct IsLastFrame<LastFrame<Cmpt>> : std::true_type {};

	template<typename TaggedCmpt> struct IsWrite : std::false_type {};
	template<typename Cmpt> struct IsWrite<const Cmpt*> : std::false_type {};
	template<typename Cmpt> struct IsWrite<Cmpt*> : std::true_type {};
	template<> struct IsWrite<World*> : std::false_type {};

	template<typename TaggedCmpt> struct IsLatest : std::false_type {};
	template<typename Cmpt> struct IsLatest<const Cmpt*> : std::true_type {};
	template<> struct IsLatest<const World*> : std::false_type {};

	template<typename TaggedCmpt> struct IsTimePoint :
		IValue<bool, IsWrite_v<TaggedCmpt> || IsLastFrame_v<TaggedCmpt> || IsLatest_v<TaggedCmpt>> {};
}
