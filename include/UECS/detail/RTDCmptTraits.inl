#pragma once

#include "../RTDCmptTraits.h"

namespace Ubpa::UECS {
	inline RTDCmptTraits& RTDCmptTraits::RegisterSize(CmptType type, size_t size) {
		sizeofs.emplace(type, size);
		return *this;
	}

	inline RTDCmptTraits& RTDCmptTraits::RegisterAlignment(CmptType type, size_t alignment) {
		alignments.emplace(type, alignment);
		return *this;
	}

	inline RTDCmptTraits& RTDCmptTraits::RegisterDefaultConstructor(CmptType type, void(*f)(void*)) {
		default_constructors.emplace(type, f);
		return *this;
	}

	inline RTDCmptTraits& RTDCmptTraits::RegisterCopyConstructor(CmptType type, void(*f)(void*, void*)) {
		copy_constructors.emplace(type, f);
		return *this;
	}

	inline RTDCmptTraits& RTDCmptTraits::RegisterMoveConstructor(CmptType type, void(*f)(void*, void*)) {
		move_constructors.emplace(type, f);
		return *this;
	}

	inline RTDCmptTraits& RTDCmptTraits::RegisterMoveAssignment(CmptType type, void(*f)(void*, void*)) {
		move_assignments.emplace(type, f);
		return *this;
	}

	inline RTDCmptTraits& RTDCmptTraits::RegisterDestructor(CmptType type, void(*f)(void*)) {
		destructors.emplace(type, f);
		return *this;
	}

	inline RTDCmptTraits& RTDCmptTraits::RegisterName(CmptType type, std::string name) {
		names.emplace(type, std::move(name));
		return *this;
	}

	inline size_t RTDCmptTraits::Sizeof(CmptType type) const {
		assert(sizeofs.find(type) != sizeofs.end());
		return sizeofs.find(type)->second;
	}

	inline size_t RTDCmptTraits::Alignof(CmptType type) const {
		auto target = alignments.find(type);

		return target != alignments.end() ? target->second : default_alignment;
	}

	inline void RTDCmptTraits::DefaultConstruct(CmptType type, void* cmpt) const {
		auto target = default_constructors.find(type);

		if (target != default_constructors.end())
			target->second(cmpt);
	}

	inline void RTDCmptTraits::CopyConstruct(CmptType type, void* dst, void* src) const {
		auto target = copy_constructors.find(type);

		if (target != copy_constructors.end())
			target->second(dst, src);
		else
			memcpy(dst, src, Sizeof(type));
	}

	inline void RTDCmptTraits::MoveConstruct(CmptType type, void* dst, void* src) const {
		auto target = move_constructors.find(type);

		if (target != move_constructors.end())
			target->second(dst, src);
		else
			memcpy(dst, src, Sizeof(type));
	}

	inline void RTDCmptTraits::MoveAssign(CmptType type, void* dst, void* src) const {
		auto target = move_assignments.find(type);

		if (target != move_assignments.end())
			target->second(dst, src);
		else
			memcpy(dst, src, Sizeof(type));
	}

	inline void RTDCmptTraits::Destruct(CmptType type, void* cmpt) const {
		auto target = destructors.find(type);
		if (target != destructors.end())
			target->second(cmpt);
	}

	inline std::string_view RTDCmptTraits::Nameof(CmptType type) const {
		auto target = names.find(type);
		if (target != names.end())
			return target->second;
		else
			return {};
	}

	template<typename... Cmpts>
	void RTDCmptTraits::Register() {
		(RegisterOne<Cmpts>(), ...);
	}

	template<typename Cmpt>
	void RTDCmptTraits::RegisterOne() {
		static_assert(!IsTaggedCmpt_v<Cmpt>, "<Cmpt> should not be tagged");
		static_assert(std::is_default_constructible_v<Cmpt>, "<Cmpt> must be default-constructible");
		static_assert(std::is_copy_constructible_v<Cmpt>, "<Cmpt> must be copy-constructible");
		static_assert(std::is_move_constructible_v<Cmpt>, "<Cmpt> must be move-constructible");
		static_assert(std::is_move_assignable_v<Cmpt>, "<Cmpt> must be move-assignable");
		static_assert(std::is_destructible_v<Cmpt>, "<Cmpt> must be destructible");

		constexpr CmptType type = CmptType::Of<Cmpt>;

		sizeofs.emplace(type, sizeof(Cmpt));
		alignments.emplace(type, alignof(Cmpt));
		names.emplace(type, std::string{ nameof::nameof_type<Cmpt>() });

		if constexpr (!std::is_trivially_default_constructible_v<Cmpt>) {
			default_constructors.emplace(type, [](void* cmpt) {
				new(cmpt)Cmpt;
			});
		}
		if constexpr (!std::is_trivially_destructible_v<Cmpt>) {
			destructors.emplace(type, [](void* cmpt) {
				reinterpret_cast<Cmpt*>(cmpt)->~Cmpt();
			});
		}
		if constexpr (!std::is_trivially_move_constructible_v<Cmpt>) {
			move_constructors.emplace(type, [](void* dst, void* src) {
				new(dst)Cmpt(std::move(*reinterpret_cast<Cmpt*>(src)));
			});
		}
		if constexpr (!std::is_trivially_move_assignable_v<Cmpt>) {
			move_assignments.emplace(type, [](void* dst, void* src) {
				*reinterpret_cast<Cmpt*>(dst) = std::move(*reinterpret_cast<Cmpt*>(src));
			});
		}
		if constexpr (!std::is_trivially_copy_constructible_v<Cmpt>) {
			copy_constructors.emplace(type, [](void* dst, void* src) {
				new(dst)Cmpt(*reinterpret_cast<Cmpt*>(src));
			});
		}
	}

	template<typename Cmpt>
	void RTDCmptTraits::Deregister() {
		constexpr CmptType type = CmptType::Of<Cmpt>;

		sizeofs.erase(type);
		alignments.erase(type);
		names.erase(type);

		if constexpr (!std::is_trivially_constructible_v<Cmpt>)
			default_constructors.erase(type);
		if constexpr (!std::is_trivially_destructible_v<Cmpt>)
			destructors.erase(type);
		if constexpr (!std::is_trivially_move_constructible_v<Cmpt>)
			move_constructors.erase(type);
		if constexpr (!std::is_trivially_move_assignable_v<Cmpt>)
			move_assignments.erase(type);
		if constexpr (!std::is_trivially_copy_constructible_v<Cmpt>)
			copy_constructors.erase(type);
	}

	inline RTDCmptTraits& RTDCmptTraits::Deregister(CmptType type) noexcept {
		names.erase(type);
		sizeofs.erase(type);
		alignments.erase(type);
		default_constructors.erase(type);
		copy_constructors.erase(type);
		move_constructors.erase(type);
		move_assignments.erase(type);
		destructors.erase(type);
		return *this;
	}

	inline RTDCmptTraits& RTDCmptTraits::Clear() {
		names.clear();
		sizeofs.clear();
		alignments.clear();
		default_constructors.clear();
		copy_constructors.clear();
		move_constructors.clear();
		move_assignments.clear();
		destructors.clear();
		return *this;
	}
}
