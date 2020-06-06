#pragma once

#include "../RTDCmptTraits.h"

namespace Ubpa {
	inline RTDCmptTraits& RTDCmptTraits::Instance() noexcept {
		static RTDCmptTraits instance;
		return instance;
	}

	// neccessary
	inline RTDCmptTraits& RTDCmptTraits::RegisterSize(CmptType type, size_t size) {
		sizeofs[type] = size;
		return *this;
	}

	// optional
	inline RTDCmptTraits& RTDCmptTraits::RegisterAlignment(CmptType type, size_t alignment) {
		alignments[type] = alignment;
		return *this;
	}

	// optional
	inline RTDCmptTraits& RTDCmptTraits::RegisterDefaultConstructor(CmptType type, std::function<void(void*)> f) {
		default_constructors[type] = std::move(f);
		return *this;
	}

	// optional
	inline RTDCmptTraits& RTDCmptTraits::RegisterCopyConstructor(CmptType type, std::function<void(void*, void*)> f) {
		copy_constructors[type] = std::move(f);
		return *this;
	}

	// optional
	inline RTDCmptTraits& RTDCmptTraits::RegisterMoveConstructor(CmptType type, std::function<void(void*, void*)> f) {
		move_constructors[type] = std::move(f);
		return *this;
	}

	// optional
	inline RTDCmptTraits& RTDCmptTraits::RegisterDestructor(CmptType type, std::function<void(void*)> f) {
		destructors[type] = std::move(f);
		return *this;
	}

	template<typename Cmpt>
	void RTDCmptTraits::Register() {
		static_assert(std::is_default_constructible_v<Cmpt>, "<Cmpt> must be default-constructible");
		static_assert(std::is_copy_constructible_v<Cmpt>, "<Cmpt> must be copy-constructible");
		static_assert(std::is_move_constructible_v<Cmpt>, "<Cmpt> must be move-constructible");
		static_assert(std::is_destructible_v<Cmpt>, "<Cmpt> must be destructible");

		constexpr CmptType type = CmptType::Of<Cmpt>;

		sizeofs[type] = sizeof(Cmpt);
		alignments[type] = alignof(Cmpt);

		if constexpr (!std::is_trivially_default_constructible_v<Cmpt>) {
			default_constructors[type] = [](void* cmpt) {
				new(cmpt)Cmpt;
			};
		}
		if constexpr (!std::is_trivially_destructible_v<Cmpt>) {
			destructors[type] = [](void* cmpt) {
				reinterpret_cast<Cmpt*>(cmpt)->~Cmpt();
			};
		}
		if constexpr (!std::is_trivially_move_constructible_v<Cmpt>) {
			move_constructors[type] = [](void* dst, void* src) {
				new(dst)Cmpt(std::move(*reinterpret_cast<Cmpt*>(src)));
			};
		}
		if constexpr (!std::is_trivially_copy_constructible_v<Cmpt>) {
			copy_constructors[type] = [](void* dst, void* src) {
				new(dst)Cmpt(*reinterpret_cast<Cmpt*>(src));
			};
		}
	}

	template<typename Cmpt>
	void RTDCmptTraits::Deregister() {
		constexpr CmptType type = CmptType::Of<Cmpt>;

		sizeofs.erase(type);
		alignments.erase(type);

		if constexpr (!std::is_trivially_constructible_v<Cmpt>)
			default_constructors.erase(type);
		if constexpr (!std::is_trivially_destructible_v<Cmpt>)
			destructors.erase(type);
		if constexpr (!std::is_trivially_move_constructible_v<Cmpt>)
			move_constructors.erase(type);
		if constexpr (!std::is_trivially_copy_constructible_v<Cmpt>)
			copy_constructors.erase(type);
	}

	inline RTDCmptTraits& RTDCmptTraits::Deregister(CmptType type) noexcept {
		sizeofs.erase(type);
		alignments.erase(type);
		default_constructors.erase(type);
		copy_constructors.erase(type);
		move_constructors.erase(type);
		destructors.erase(type);
		return *this;
	}
}
