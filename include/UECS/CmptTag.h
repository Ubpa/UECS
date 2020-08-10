#pragma once

#include <UTemplate/Typelist.h>

namespace Ubpa::UECS {

	// read/write tag : LastFrame -> Write -> Latest
	// singleton tag  : Singleton
	// ====
	// 1. LastFrame<Cmpt>
	// 2. Write<Cmpt> == Cmpt*
	// 3. Latest<Cmpt> == const Cmpt*
	// 4. LastFrame<Singleton<Cmpt>>
	// 5. Write<Singleton<Cmpt>> == Singleton<Cmpt>
	// 6. Latest<Singleton<Cmpt>>

	enum class AccessMode : size_t {
		LAST_FRAME = 0,           // 000, LastFrame<Cmpt>
		WRITE = 1,                // 001, Write<Cmpt> / Cmpt*
		LATEST = 2,               // 010, Latest<Cmpt> / const Cmpt*
		LAST_FRAME_SINGLETON = 4, // 100, LastFrame<Singleton<Cmpt>>
		WRITE_SINGLETON = 5,      // 101, Write<Singleton<Cmpt>> / Singleton<Cmpt>
		LATEST_SINGLETON = 6,     // 110, Latest<Singleton<Cmpt>>
	};

	constexpr bool AccessMode_IsSingleton(AccessMode mode) noexcept {
		return (static_cast<size_t>(mode) & 4) != 0;
	}

	template<typename Cmpt>
	class Singleton {
	public:
		Singleton(Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		Singleton(const Cmpt* cmpt) noexcept : cmpt{ const_cast<Cmpt*>(cmpt) } {}

		Cmpt* Get() noexcept { return cmpt; }
		const Cmpt* Get() const noexcept { return cmpt; }

		operator Cmpt* () noexcept { return cmpt; }
		operator const Cmpt* () const noexcept { return cmpt; }

		Cmpt* operator->() noexcept { return cmpt; }
		const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		Cmpt* cmpt;
	};

	template<typename Cmpt>
	class LastFrame {
	public:
		LastFrame(const Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		const Cmpt* Get() const noexcept { return cmpt; }
		operator const Cmpt* () const noexcept { return cmpt; }
		const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		const Cmpt* cmpt;
	};

	template<typename Cmpt>
	class LastFrame<Singleton<Cmpt>> {
	public:
		LastFrame(const Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		const Cmpt* Get() const noexcept { return cmpt; }
		operator const Cmpt* () const noexcept { return cmpt; }
		const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		const Cmpt* cmpt;
	};

	template<typename Cmpt>
	class Write {
	public:
		Write(Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		Cmpt* Get() const noexcept { return cmpt; }
		operator Cmpt* () const noexcept { return cmpt; }
		Cmpt* operator->() const noexcept { return cmpt; }
	private:
		Cmpt* cmpt;
	};

	template<typename Cmpt>
	class Write<Singleton<Cmpt>> {
	public:
		Write(Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		Cmpt* Get() const noexcept { return cmpt; }
		operator Cmpt* () const noexcept { return cmpt; }
		Cmpt* operator->() const noexcept { return cmpt; }
	private:
		Cmpt* cmpt;
	};

	template<typename Cmpt>
	class Latest {
	public:
		Latest(const Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		const Cmpt* Get() const noexcept { return cmpt; }
		operator const Cmpt* () const noexcept { return cmpt; }
		const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		const Cmpt* cmpt;
	};

	template<typename Cmpt>
	class Latest<Singleton<Cmpt>> {
	public:
		Latest(const Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		const Cmpt* Get() const noexcept { return cmpt; }
		operator const Cmpt* () const noexcept { return cmpt; }
		const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		const Cmpt* cmpt;
	};

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

	template<typename T>
	static constexpr AccessMode AccessModeOf =
		IsLastFrame_v<T> ? AccessMode::LAST_FRAME : (
		IsWrite_v<T> ? AccessMode::WRITE : (
		IsLatest_v<T> ? AccessMode::LATEST : (
		IsLastFrameSingleton_v<T> ? AccessMode::LAST_FRAME_SINGLETON : (
		IsWriteSingleton_v<T> ? AccessMode::WRITE_SINGLETON : (
		IsLatestSingleton_v<T> ? AccessMode::LATEST_SINGLETON :
		AccessMode::WRITE // default
		)))));
}

#include "detail/CmptTag.inl"
