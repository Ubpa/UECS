#pragma once

#include <UTemplate/Util.hpp>

namespace Ubpa::UECS {
	struct Entity;
	class CmptLocator;
	class World;
	class Chunk;
}

namespace Ubpa::UECS {
	template<typename TaggedCmpt>
	void* CastToVoidPointer(TaggedCmpt p) noexcept {
		return const_cast<void*>(reinterpret_cast<const void*>(p));
	}

	// <Cmpt> (without read/write and singleton tag)
	template<typename TaggedCmpt>
	struct RemoveTag;
	template<typename TaggedCmpt>
	using RemoveTag_t = typename RemoveTag<TaggedCmpt>::type;

	// <Cmpt> / Singleton<Cmpt>
	template<typename TaggedCmpt>
	struct RemoveRWTag;
	template<typename TaggedCmpt>
	using RemoveRWTag_t = typename RemoveRWTag<TaggedCmpt>::type;

	// LastFrame<Cmpt>
	// Write<Cmpt> / Cmpt*
	// Latest<Cmpt> / const Cmpt*
	template<typename TaggedCmpt>
	struct RemoveSingletonTag;
	template<typename TaggedCmpt>
	using RemoveSingletonTag_t = typename RemoveSingletonTag<TaggedCmpt>::type;

	// <Cmpt>*
	template<typename TaggedCmpt>
	struct DecayTag;
	template<typename TaggedCmpt>
	using DecayTag_t = typename DecayTag<TaggedCmpt>::type;

	// <Cmpt>*, World*
	template<typename Arg>
	struct DecayArg;
	template<typename Arg>
	using DecayArg_t = typename DecayArg<Arg>::type;

	template<typename TaggedCmpt>
	struct IsLastFrame;
	template<typename TaggedCmpt>
	static constexpr bool IsLastFrame_v = IsLastFrame<TaggedCmpt>::value;

	template<typename TaggedCmpt>
	struct IsWrite;
	template<typename TaggedCmpt>
	static constexpr bool IsWrite_v = IsWrite<TaggedCmpt>::value;

	template<typename TaggedCmpt>
	struct IsLatest;
	template<typename TaggedCmpt>
	static constexpr bool IsLatest_v = IsLatest<TaggedCmpt>::value;

	template<typename TaggedCmpt>
	struct IsLastFrameSingleton;
	template<typename TaggedCmpt>
	static constexpr bool IsLastFrameSingleton_v = IsLastFrameSingleton<TaggedCmpt>::value;

	template<typename TaggedCmpt>
	struct IsWriteSingleton;
	template<typename TaggedCmpt>
	static constexpr bool IsWriteSingleton_v = IsWriteSingleton<TaggedCmpt>::value;

	template<typename TaggedCmpt>
	struct IsLatestSingleton;
	template<typename TaggedCmpt>
	static constexpr bool IsLatestSingleton_v = IsLatestSingleton<TaggedCmpt>::value;

	template<typename TaggedCmpt>
	struct IsSingleton : IValue<bool,
		IsLastFrameSingleton_v<TaggedCmpt>
		|| IsWriteSingleton_v<TaggedCmpt>
		|| IsLatestSingleton_v<TaggedCmpt>
	> {};
	template<typename TaggedCmpt>
	static constexpr bool IsSingleton_v = IsSingleton<TaggedCmpt>::value;

	template<typename TaggedCmpt>
	struct IsNonSingleton : IValue<bool,
		IsLastFrame_v<TaggedCmpt>
		|| IsWrite_v<TaggedCmpt>
		|| IsLatest_v<TaggedCmpt>
	> {};
	template<typename TaggedCmpt>
	static constexpr bool IsNonSingleton_v = IsNonSingleton<TaggedCmpt>::value;

	template<typename T>
	struct IsTaggedCmpt : IValue<bool, IsNonSingleton_v<T> || IsSingleton_v<T>> {};
	template<typename T>
	static constexpr bool IsTaggedCmpt_v = IsTaggedCmpt<T>::value;

	template<typename T, AccessMode defaultMode>
	static constexpr AccessMode AccessMode_of_default =
		IsLastFrame_v<T> || IsLastFrameSingleton_v<T> ? AccessMode::LAST_FRAME : (
			IsWrite_v<T> || IsWriteSingleton_v<T> ? AccessMode::WRITE : (
				IsLatest_v<T> || IsLatestSingleton_v<T> ? AccessMode::LATEST :
				defaultMode
			)
		);

	template<typename T>
	static constexpr AccessMode AccessMode_of = AccessMode_of_default<T, AccessMode::WRITE>;

	template<typename Cmpt> struct RemoveTag : std::type_identity<Cmpt> {}; // default

	template<typename Cmpt> struct RemoveTag<LastFrame<Cmpt>> : std::type_identity<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Write<Cmpt>> : std::type_identity<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Latest<Cmpt>> : std::type_identity<Cmpt> {};

	template<typename Cmpt> struct RemoveTag<LastFrame<Singleton<Cmpt>>> : std::type_identity<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Write<Singleton<Cmpt>>> : std::type_identity<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Latest<Singleton<Cmpt>>> : std::type_identity<Cmpt> {};

	template<typename Cmpt> struct RemoveTag<Cmpt*> : std::type_identity<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<const Cmpt*> : std::type_identity<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<Singleton<Cmpt>> : std::type_identity<Cmpt> {};
	template<typename Cmpt> struct RemoveTag<const Singleton<Cmpt>> : std::type_identity<Cmpt> {};

	// ====

	template<typename T> struct RemoveRWTag : std::type_identity<T> {}; // default

	template<typename T> struct RemoveRWTag<LastFrame<T>> : std::type_identity<T> {};
	template<typename T> struct RemoveRWTag<Write<T>> : std::type_identity<T> {};
	template<typename T> struct RemoveRWTag<Latest<T>> : std::type_identity<T> {};

	template<typename Cmpt> struct RemoveRWTag<Cmpt*> : std::type_identity<Cmpt> {};
	template<typename Cmpt> struct RemoveRWTag<const Cmpt*> : std::type_identity<Cmpt> {};
	template<typename Cmpt> struct RemoveRWTag<const Singleton<Cmpt>> : std::type_identity<Singleton<Cmpt>> {};

	// ====

	template<typename T> struct RemoveSingletonTag : std::type_identity<T> {}; // default

	template<typename Cmpt> struct RemoveSingletonTag<LastFrame<Singleton<Cmpt>>> : std::type_identity<LastFrame<Cmpt>> {};
	template<typename Cmpt> struct RemoveSingletonTag<Write<Singleton<Cmpt>>> : std::type_identity<Write<Cmpt>> {};
	template<typename Cmpt> struct RemoveSingletonTag<Latest<Singleton<Cmpt>>> : std::type_identity<Latest<Cmpt>> {};

	template<typename Cmpt> struct RemoveSingletonTag<Singleton<Cmpt>> : std::type_identity<Cmpt*> {};
	template<typename Cmpt> struct RemoveSingletonTag<const Singleton<Cmpt>> : std::type_identity<const Cmpt*> {};

	// ====

	template<typename T> struct DecayTag : std::type_identity<T> {};

	template<typename Cmpt> struct DecayTag<LastFrame<Cmpt>> : std::type_identity<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Write<Cmpt>> : std::type_identity<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Latest<Cmpt>> : std::type_identity<Cmpt*> {};

	template<typename Cmpt> struct DecayTag<LastFrame<Singleton<Cmpt>>> : std::type_identity<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Write<Singleton<Cmpt>>> : std::type_identity<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Latest<Singleton<Cmpt>>> : std::type_identity<Cmpt*> {};

	template<typename Cmpt> struct DecayTag<const Cmpt*> : std::type_identity<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<Singleton<Cmpt>> : std::type_identity<Cmpt*> {};
	template<typename Cmpt> struct DecayTag<const Singleton<Cmpt>> : std::type_identity<Cmpt*> {};

	// ====
	
	template<typename Arg>
	struct DecayArg : DecayTag<Arg> {};
	template<>
	struct DecayArg<const World*> : std::type_identity<World*> {};
	template<>
	struct DecayArg<const Chunk*> : std::type_identity<const Chunk*> {};

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
	template<> struct IsWrite<Chunk*> : std::false_type { static_assert("you should use const Chunk*"); };

	template<typename T> struct IsLatest : std::false_type {};
	template<typename Cmpt> struct IsLatest<Latest<Cmpt>> : std::true_type {};
	template<typename Cmpt> struct IsLatest<Latest<Singleton<Cmpt>>> : std::false_type {};
	template<typename Cmpt> struct IsLatest<const Cmpt*> : std::true_type {};
	template<> struct IsLatest<const World*> : std::false_type {};
	template<> struct IsLatest<const Chunk*> : std::false_type {};

	template<typename T> struct IsLastFrameSingleton : std::false_type {};
	template<typename Cmpt> struct IsLastFrameSingleton<LastFrame<Singleton<Cmpt>>> : std::true_type {};

	template<typename T> struct IsWriteSingleton : std::false_type {};
	template<typename Cmpt> struct IsWriteSingleton<Write<Singleton<Cmpt>>> : std::true_type {};
	template<typename Cmpt> struct IsWriteSingleton<Singleton<Cmpt>> : std::true_type {};

	template<typename T> struct IsLatestSingleton : std::false_type {};
	template<typename Cmpt> struct IsLatestSingleton<Latest<Singleton<Cmpt>>> : std::true_type {};
}
