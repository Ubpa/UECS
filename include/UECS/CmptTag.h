#pragma once

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
}

#include "detail/CmptTag.inl"
