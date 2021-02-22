#pragma once

#include <cstddef>

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

	enum class AccessMode : std::size_t {
		LAST_FRAME = 0b000, // LastFrame<Cmpt>
		WRITE      = 0b001, // Write<Cmpt> / Cmpt*
		LATEST     = 0b010, // Latest<Cmpt> / const Cmpt*
	};

	template<typename Cmpt>
	class Singleton {
	public:
		constexpr Singleton(Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		constexpr Cmpt* Get() const noexcept { return cmpt; }
		constexpr operator Cmpt* () const noexcept { return cmpt; }
		constexpr Cmpt* operator->() const noexcept { return cmpt; }
	private:
		Cmpt* cmpt;
	};

	template<typename Cmpt>
	class LastFrame {
	public:
		constexpr LastFrame(const Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		constexpr const Cmpt* Get() const noexcept { return cmpt; }
		constexpr operator const Cmpt* () const noexcept { return cmpt; }
		constexpr const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		const Cmpt* cmpt;
	};

	template<typename Cmpt>
	class LastFrame<Singleton<Cmpt>> {
	public:
		constexpr LastFrame(const Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		constexpr const Cmpt* Get() const noexcept { return cmpt; }
		constexpr operator const Cmpt* () const noexcept { return cmpt; }
		constexpr const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		const Cmpt* cmpt;
	};

	template<typename Cmpt>
	class Write {
	public:
		constexpr Write(Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		constexpr Cmpt* Get() const noexcept { return cmpt; }
		constexpr operator Cmpt* () const noexcept { return cmpt; }
		constexpr Cmpt* operator->() const noexcept { return cmpt; }
	private:
		Cmpt* cmpt;
	};

	template<typename Cmpt>
	class Write<Singleton<Cmpt>> {
	public:
		constexpr Write(Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		constexpr Cmpt* Get() const noexcept { return cmpt; }
		constexpr operator Cmpt* () const noexcept { return cmpt; }
		constexpr Cmpt* operator->() const noexcept { return cmpt; }
	private:
		Cmpt* cmpt;
	};

	template<typename Cmpt>
	class Latest {
	public:
		constexpr Latest(const Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		constexpr const Cmpt* Get() const noexcept { return cmpt; }
		constexpr operator const Cmpt* () const noexcept { return cmpt; }
		constexpr const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		const Cmpt* cmpt;
	};

	template<typename Cmpt>
	class Latest<Singleton<Cmpt>> {
	public:
		constexpr Latest(const Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		constexpr const Cmpt* Get() const noexcept { return cmpt; }
		constexpr operator const Cmpt* () const noexcept { return cmpt; }
		constexpr const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		const Cmpt* cmpt;
	};
}

#include "details/CmptTag.inl"
