#pragma once

#include <type_traits>

namespace Ubpa {
	class Entity;
}

namespace Ubpa::CmptTag {
	template<typename Cmpt> struct RemoveTag<const Cmpt*> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Cmpt*> : IType<Cmpt> {};
	template<> struct RemoveTag<const Entity*> : IType<const Entity*> {};
	template<> struct RemoveTag<Entity*> : IType<Entity*> {};
	template<typename Cmpt> struct RemoveTag<LastFrame<Cmpt>> : IType<Cmpt> {};

	template<typename T> struct DecayTag : IType<T> {};
	template<typename Cmpt> struct DecayTag<const Cmpt*> : IType<Cmpt*> {};
	template<> struct DecayTag<const Entity> : IType<Entity> {};
	// template<typename Cmpt> struct DecayTag<Cmpt*> : IType<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<LastFrame<Cmpt>> : IType<Cmpt*> {};

	template<typename TaggedCmpt> struct IsLastFrame : std::false_type {};
	template<typename Cmpt> struct IsLastFrame<LastFrame<Cmpt>> : std::true_type {};

	template<typename TaggedCmpt> struct IsWrite : std::false_type {};
	template<typename Cmpt> struct IsWrite<const Cmpt*> : std::false_type {};
	template<typename Cmpt> struct IsWrite<Cmpt*> : std::true_type {};
	template<> struct IsWrite<const Entity*> : std::false_type {};
	template<> struct IsWrite<Entity*> : std::false_type {};

	template<typename TaggedCmpt> struct IsLatest : std::false_type {};
	template<> struct IsLatest<const Entity*> : std::false_type {};
	template<typename Cmpt> struct IsLatest<const Cmpt*> : std::true_type {};

	template<typename TaggedCmpt> struct IsTimePoint :
		IValue<bool, IsWrite_v<TaggedCmpt> || IsLastFrame_v<TaggedCmpt> || IsLatest_v<TaggedCmpt>> {};
}
