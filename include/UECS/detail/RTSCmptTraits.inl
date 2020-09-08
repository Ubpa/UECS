#pragma once

#include <stdexcept>

namespace Ubpa::UECS {
	inline size_t RTSCmptTraits::Sizeof(CmptType type) const {
		assert(sizeofs.find(type) != sizeofs.end());
		return sizeofs.find(type)->second;
	}

	inline size_t RTSCmptTraits::Alignof(CmptType type) const {
		assert(alignments.find(type) != alignments.end());
		return alignments.find(type)->second;
	}

	inline void RTSCmptTraits::CopyConstruct(CmptType type, void* dst, void* src) const {
		auto target = copy_constructors.find(type);

		if (target != copy_constructors.end())
			target->second(dst, src);
		else
			memcpy(dst, src, Sizeof(type));
	}

	inline void RTSCmptTraits::MoveConstruct(CmptType type, void* dst, void* src) const {
		auto target = move_constructors.find(type);

		if (target != move_constructors.end())
			target->second(dst, src);
		else
			memcpy(dst, src, Sizeof(type));
	}

	inline void RTSCmptTraits::MoveAssign(CmptType type, void* dst, void* src) const {
		auto target = move_assignments.find(type);

		if (target != move_assignments.end())
			target->second(dst, src);
		else
			memcpy(dst, src, Sizeof(type));
	}

	inline void RTSCmptTraits::Destruct(CmptType type, void* cmpt) const {
		auto target = destructors.find(type);
		if (target != destructors.end())
			target->second(cmpt);
	}

	template<typename Cmpt>
	void RTSCmptTraits::Register() {
		static_assert(!IsTaggedCmpt_v<Cmpt>, "<Cmpt> should not be tagged");
		static_assert(std::is_copy_constructible_v<Cmpt>, "<Cmpt> must be copy-constructible");
		static_assert(std::is_move_constructible_v<Cmpt>, "<Cmpt> must be move-constructible");
		static_assert(std::is_move_assignable_v<Cmpt>, "<Cmpt> must be move-assignable");
		static_assert(std::is_destructible_v<Cmpt>, "<Cmpt> must be destructible");

		constexpr CmptType type = CmptType::Of<Cmpt>;

		sizeofs.emplace(type, sizeof(Cmpt));
		alignments.emplace(type, alignof(Cmpt));

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
		if constexpr (!std::is_trivially_copy_assignable_v<Cmpt>) {
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
	void RTSCmptTraits::Deregister() {
		constexpr CmptType type = CmptType::Of<Cmpt>;

		sizeofs.erase(type);
		alignments.erase(type);

		if constexpr (!std::is_trivially_destructible_v<Cmpt>)
			destructors.erase(type);
		if constexpr (!std::is_trivially_copy_constructible_v<Cmpt>)
			copy_constructors.erase(type);
		if constexpr (!std::is_trivially_move_constructible_v<Cmpt>)
			move_constructors.erase(type);
		if constexpr (!std::is_trivially_move_assignable_v<Cmpt>)
			move_assignments.erase(type);
	}

	inline void RTSCmptTraits::Register(const RTDCmptTraits& rtdct, CmptType type) {
		auto size_target = rtdct.GetSizeofs().find(type);
		if (size_target == rtdct.GetSizeofs().end())
			throw std::logic_error("RTSCmptTraits::Register: RTDCmptTraits hasn't registered <CmptType>");
		sizeofs[type] = size_target->second;
		
		auto alignment_target = rtdct.GetAlignments().find(type);
		if (alignment_target == rtdct.GetAlignments().end())
			alignments[type] = RTDCmptTraits::default_alignment;
		else
			alignments[type] = alignment_target->second;

		auto destructor_target = rtdct.GetDestructors().find(type);
		auto copy_constructor_target = rtdct.GetCopyConstructors().find(type);
		auto move_constructor_target = rtdct.GetMoveConstructors().find(type);
		auto move_assignments_target = rtdct.GetMoveAssignments().find(type);

		if (destructor_target != rtdct.GetDestructors().end())
			destructors.emplace(type, destructor_target->second);
		if (copy_constructor_target != rtdct.GetCopyConstructors().end())
			copy_constructors.emplace(type, copy_constructor_target->second);
		if (move_constructor_target != rtdct.GetMoveConstructors().end())
			move_constructors.emplace(type, move_constructor_target->second);
		if (move_assignments_target != rtdct.GetMoveAssignments().end())
			move_assignments.emplace(type, move_assignments_target->second);
	}

	inline void RTSCmptTraits::Deregister(CmptType type) noexcept {
		sizeofs.erase(type);
		alignments.erase(type);
		copy_constructors.erase(type);
		move_constructors.erase(type);
		move_assignments.erase(type);
		destructors.erase(type);
	}
}
