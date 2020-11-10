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
		LAST_FRAME           = 0b000, // LastFrame<Cmpt>
		WRITE                = 0b001, // Write<Cmpt> / Cmpt*
		LATEST               = 0b010, // Latest<Cmpt> / const Cmpt*
	};

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
