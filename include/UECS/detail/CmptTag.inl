#pragma once

#include <type_traits>

namespace Ubpa::UECS {
	class Entity;
	class CmptLocator;
	class World;
}

namespace Ubpa::UECS {
	template<typename Cmpt> struct RemoveTag : IType<Cmpt> {}; // default

	template<typename Cmpt> struct RemoveTag<LastFrame<Cmpt>> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Write<Cmpt>> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Latest<Cmpt>> : IType<Cmpt> {};

	template<typename Cmpt> struct RemoveTag<LastFrame<Singleton<Cmpt>>> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Write<Singleton<Cmpt>>> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Latest<Singleton<Cmpt>>> : IType<Cmpt> {};

	template<typename Cmpt> struct RemoveTag<Cmpt*> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<const Cmpt*> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Singleton<Cmpt>> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<const Singleton<Cmpt>> : IType<Cmpt> {};

	// ====

	template<typename T> struct RemoveRWTag : IType<T> {}; // default

	template<typename T> struct RemoveRWTag<LastFrame<T>> : IType<T> {};
	template<typename T> struct RemoveRWTag<Write<T>> : IType<T> {};
	template<typename T> struct RemoveRWTag<Latest<T>> : IType<T> {};

	template<typename Cmpt> struct RemoveRWTag<Cmpt*> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveRWTag<const Cmpt*> : IType<Cmpt> {};
	template<typename Cmpt> struct RemoveRWTag<const Singleton<Cmpt>> : IType<Singleton<Cmpt>> {};

	// ====

	template<typename T> struct RemoveSingletonTag : IType<T> {}; // default

	template<typename Cmpt> struct RemoveSingletonTag<LastFrame<Singleton<Cmpt>>> : IType<LastFrame<Cmpt>> {};
	template<typename Cmpt> struct RemoveSingletonTag<Write<Singleton<Cmpt>>> : IType<Write<Cmpt>> {};
	template<typename Cmpt> struct RemoveSingletonTag<Latest<Singleton<Cmpt>>> : IType<Latest<Cmpt>> {};

	template<typename Cmpt> struct RemoveSingletonTag<Singleton<Cmpt>> : IType<Cmpt*> {};
	template<typename Cmpt> struct RemoveSingletonTag<const Singleton<Cmpt>> : IType<const Cmpt*> {};

	// ====

	template<typename T> struct DecayTag : IType<T> {};

	template<typename Cmpt> struct DecayTag<LastFrame<Cmpt>> : IType<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Write<Cmpt>> : IType<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Latest<Cmpt>> : IType<Cmpt*> {};

	template<typename Cmpt> struct DecayTag<LastFrame<Singleton<Cmpt>>> : IType<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Write<Singleton<Cmpt>>> : IType<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Latest<Singleton<Cmpt>>> : IType<Cmpt*> {};

	template<typename Cmpt> struct DecayTag<const Cmpt*> : IType<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Singleton<Cmpt>> : IType<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<const Singleton<Cmpt>> : IType<Cmpt*> {};

	// ====

	template<typename T> struct IsLastFrame : std::false_type {};
	template<typename Cmpt> struct IsLastFrame<LastFrame<Cmpt>> : std::true_type {};
	template<typename Cmpt> struct IsLastFrame<LastFrame<Singleton<Cmpt>>> : std::false_type {};

	template<typename T> struct IsWrite : std::false_type {};
	template<typename Cmpt> struct IsWrite<Write<Cmpt>> : std::true_type {};
	template<typename Cmpt> struct IsWrite<Write<Singleton<Cmpt>>> : std::false_type {};
	template<typename Cmpt> struct IsWrite<Cmpt*> : std::true_type {};
	template<typename Cmpt> struct IsWrite<const Cmpt*> : std::false_type {};
	template<> struct IsWrite<World*> : std::false_type {};

	template<typename T> struct IsLatest : std::false_type {};
	template<typename Cmpt> struct IsLatest<Latest<Cmpt>> : std::true_type {};
	template<typename Cmpt> struct IsLatest<Latest<Singleton<Cmpt>>> : std::false_type {};
	template<typename Cmpt> struct IsLatest<const Cmpt*> : std::true_type {};

	template<typename T> struct IsLastFrameSingleton : std::false_type {};
	template<typename Cmpt> struct IsLastFrameSingleton<LastFrame<Singleton<Cmpt>>> : std::true_type {};

	template<typename T> struct IsWriteSingleton : std::false_type {};
	template<typename Cmpt> struct IsWriteSingleton<Write<Singleton<Cmpt>>> : std::true_type {};
	template<typename Cmpt> struct IsWriteSingleton<Singleton<Cmpt>> : std::true_type {};

	template<typename T> struct IsLatestSingleton : std::false_type {};
	template<typename Cmpt> struct IsLatestSingleton<Latest<Singleton<Cmpt>>> : std::true_type {};
}
