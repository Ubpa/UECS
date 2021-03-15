#pragma once

#include <stdexcept>

namespace Ubpa::UECS {
	inline std::size_t ArchetypeCmptTraits::Sizeof(TypeID type) const {
		for (const auto& [t, s] : sizeofs) {
			if (t == type)
				return s;
		}
		assert(false);
		return 0;
	}

	inline std::size_t ArchetypeCmptTraits::Alignof(TypeID type) const {
		for (const auto& [t, a] : alignments) {
			if (t == type)
				return a;
		}
		return RTDCmptTraits::default_alignment;
	}

	inline void ArchetypeCmptTraits::CopyConstruct(TypeID type, void* dst, void* src) const {
		for (const auto& [t, copy_ctor] : copy_constructors) {
			if (t == type) {
				copy_ctor(dst, src);
				return;
			}
		}
	}

	inline void ArchetypeCmptTraits::MoveConstruct(TypeID type, void* dst, void* src) const {
		for (const auto& [t, move_ctor] : move_constructors) {
			if (t == type) {
				move_ctor(dst, src);
				return;
			}
		}
	}

	inline void ArchetypeCmptTraits::MoveAssign(TypeID type, void* dst, void* src) const {
		for (const auto& [t, move_assign] : move_constructors) {
			if (t == type) {
				move_assign(dst, src);
				return;
			}
		}
	}

	inline void ArchetypeCmptTraits::Destruct(TypeID type, void* cmpt) const {
		for (const auto& [t, dtor] : destructors) {
			if (t == type) {
				dtor(cmpt);
				return;
			}
		}
	}

	inline const std::function<void(void*, void*)>* ArchetypeCmptTraits::GetCopyConstruct(TypeID type) const {
		for (const auto& [t, copy_ctor] : copy_constructors) {
			if (t == type)
				return &copy_ctor;
		}
		return nullptr;
	}

	inline const std::function<void(void*, void*)>* ArchetypeCmptTraits::GetMoveConstruct(TypeID type) const {
		for (const auto& [t, move_ctor] : move_constructors) {
			if (t == type)
				return &move_ctor;
		}
		return nullptr;

	}

	inline const std::function<void(void*, void*)>* ArchetypeCmptTraits::GetMoveAssign(TypeID type) const {
		for (const auto& [t, move_assign] : move_constructors) {
			if (t == type)
				return &move_assign;
		}
		return nullptr;
	}

	inline const std::function<void(void*)>* ArchetypeCmptTraits::GetDestruct(TypeID type) const {
		for (const auto& [t, dtor] : destructors) {
			if (t == type)
				return &dtor;
		}
		return nullptr;
	}


	template<typename Cmpt>
	void ArchetypeCmptTraits::Register() {
		static_assert(!IsTaggedCmpt_v<Cmpt>, "<Cmpt> should not be tagged");
		static_assert(std::is_copy_constructible_v<Cmpt> || std::is_constructible_v<Cmpt, Cmpt&>,
			"<Cmpt> must be copy-constructible or constructible with <Cmpt&>");
		static_assert(std::is_move_constructible_v<Cmpt>, "<Cmpt> must be move-constructible");
		static_assert(std::is_move_assignable_v<Cmpt>, "<Cmpt> must be move-assignable");
		static_assert(std::is_destructible_v<Cmpt>, "<Cmpt> must be destructible");

		constexpr TypeID type = TypeID_of<Cmpt>;

		sizeofs.emplace_back(type, sizeof(Cmpt));
		alignments.emplace_back(type, alignof(Cmpt));

		if constexpr (!std::is_trivially_destructible_v<Cmpt>) {
			destructors.emplace_back(type, [](void* cmpt) {
				static_cast<Cmpt*>(cmpt)->~Cmpt();
			});
		}
		if constexpr (!std::is_trivially_move_constructible_v<Cmpt>) {
			move_constructors.emplace_back(type, [](void* dst, void* src) {
				new(dst)Cmpt(std::move(*static_cast<Cmpt*>(src)));
			});
		}
		if constexpr (!std::is_trivially_copy_assignable_v<Cmpt>) {
			move_assignments.emplace_back(type, [](void* dst, void* src) {
				*static_cast<Cmpt*>(dst) = std::move(*static_cast<Cmpt*>(src));
			});
		}
		if constexpr (!std::is_trivially_copy_constructible_v<Cmpt>) {
			copy_constructors.emplace_back(type, [](void* dst, void* src) {
				new(dst)Cmpt(*static_cast<Cmpt*>(src));
			});
		}
	}

	inline void ArchetypeCmptTraits::Register(const RTDCmptTraits& rtdct, TypeID type) {
		auto size_target = rtdct.GetSizeofs().find(type);
		if (size_target == rtdct.GetSizeofs().end())
			throw std::logic_error("ArchetypeCmptTraits::Register: RTDCmptTraits hasn't registered <TypeID>");
		sizeofs.emplace_back(type, size_target->second);
		
		auto alignment_target = rtdct.GetAlignments().find(type);
		if (alignment_target != rtdct.GetAlignments().end())
			alignments.emplace_back(type, alignment_target->second);

		auto destructor_target = rtdct.GetDestructors().find(type);
		auto copy_constructor_target = rtdct.GetCopyConstructors().find(type);
		auto move_constructor_target = rtdct.GetMoveConstructors().find(type);
		auto move_assignments_target = rtdct.GetMoveAssignments().find(type);

		if (destructor_target != rtdct.GetDestructors().end())
			destructors.emplace_back(type, destructor_target->second);
		if (copy_constructor_target != rtdct.GetCopyConstructors().end())
			copy_constructors.emplace_back(type, copy_constructor_target->second);
		if (move_constructor_target != rtdct.GetMoveConstructors().end())
			move_constructors.emplace_back(type, move_constructor_target->second);
		if (move_assignments_target != rtdct.GetMoveAssignments().end())
			move_assignments.emplace_back(type, move_assignments_target->second);
	}

	template<typename Cmpt>
	void ArchetypeCmptTraits::Deregister() noexcept {
		constexpr TypeID type = TypeID_of<Cmpt>;

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

	inline void ArchetypeCmptTraits::Deregister(TypeID type) noexcept {
		for (auto iter = sizeofs.begin(); iter != sizeofs.end(); ++iter) {
			const auto& [t, e] = *iter;
			if (t == type) {
				sizeofs.erase(iter);
				break;
			}
		}
		for (auto iter = alignments.begin(); iter != alignments.end(); ++iter) {
			const auto& [t, e] = *iter;
			if (t == type) {
				alignments.erase(iter);
				break;
			}
		}
		for (auto iter = copy_constructors.begin(); iter != copy_constructors.end(); ++iter) {
			const auto& [t, e] = *iter;
			if (t == type) {
				copy_constructors.erase(iter);
				break;
			}
		}
		for (auto iter = move_constructors.begin(); iter != move_constructors.end(); ++iter) {
			const auto& [t, e] = *iter;
			if (t == type) {
				move_constructors.erase(iter);
				break;
			}
		}
		for (auto iter = move_assignments.begin(); iter != move_assignments.end(); ++iter) {
			const auto& [t, e] = *iter;
			if (t == type) {
				move_assignments.erase(iter);
				break;
			}
		}
		for (auto iter = destructors.begin(); iter != destructors.end(); ++iter) {
			const auto& [t, e] = *iter;
			if (t == type) {
				destructors.erase(iter);
				break;
			}
		}
	}
}
