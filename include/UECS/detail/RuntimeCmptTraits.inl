#pragma once

namespace Ubpa {
	template<typename Cmpt>
	void RuntimeCmptTraits::Register() {
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
	}

	template<typename Cmpt>
	void RuntimeCmptTraits::Deregister() {
		constexpr CmptType type = CmptType::Of<Cmpt>();

		sizeofs.erase(type);
		alignments.erase(type);

		if constexpr (!std::is_trivially_destructible_v<Cmpt>)
			destructors.erase(type);
		if constexpr (!std::is_trivially_move_constructible_v<Cmpt>)
			move_constructors.erase(type);
	}
}