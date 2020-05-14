#pragma once

#include "../RTDCmptTraits.h"

namespace Ubpa {
	template<typename Cmpt>
	void RTSCmptTraits::Register() {
		static_assert(std::is_copy_constructible_v<Cmpt>, "<Cmpt> must be copy-constructible");
		static_assert(std::is_move_constructible_v<Cmpt>, "<Cmpt> must be move-constructible");
		static_assert(std::is_destructible_v<Cmpt>, "<Cmpt> must be destructible");

		constexpr CmptType type = CmptType::Of<Cmpt>();

		sizeofs[type] = sizeof(Cmpt);
		alignments[type] = alignof(Cmpt);

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
	void RTSCmptTraits::Deregister() {
		constexpr CmptType type = CmptType::Of<Cmpt>();

		sizeofs.erase(type);
		alignments.erase(type);

		if constexpr (!std::is_trivially_destructible_v<Cmpt>)
			destructors.erase(type);
		if constexpr (!std::is_trivially_move_constructible_v<Cmpt>)
			move_constructors.erase(type);
		if constexpr (!std::is_trivially_copy_constructible_v<Cmpt>)
			copy_constructors.erase(type);
	}

	inline void RTSCmptTraits::Register(CmptType type) {
		const auto& rtdct = RTDCmptTraits().Instance();
		auto size_target = rtdct.sizeofs.find(type);
		if (size_target == rtdct.sizeofs.end())
			throw std::logic_error("RTSCmptTraits::Register: RTDCmptTraits hasn't registered <CmptType>");
		sizeofs[type] = size_target->second;
		
		auto alignment_target = rtdct.alignments.find(type);
		if (alignment_target == rtdct.alignments.end())
			alignments[type] = RTDCmptTraits::default_alignment;
		else
			alignments[type] = alignment_target->second;

		auto destructor_target = rtdct.destructors.find(type);
		auto copy_constructor_target = rtdct.copy_constructors.find(type);
		auto move_constructor_target = rtdct.move_constructors.find(type);

		if (destructor_target != rtdct.destructors.end())
			destructors[type] = destructor_target->second;
		if (copy_constructor_target != rtdct.copy_constructors.end())
			copy_constructors[type] = copy_constructor_target->second;
		if (move_constructor_target != rtdct.move_constructors.end())
			move_constructors[type] = move_constructor_target->second;
	}

	inline void RTSCmptTraits::Deregister(CmptType type) {
		sizeofs.erase(type);
		alignments.erase(type);
		copy_constructors.erase(type);
		move_constructors.erase(type);
		destructors.erase(type);
	}
}
