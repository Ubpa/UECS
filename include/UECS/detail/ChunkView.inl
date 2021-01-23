#pragma once

namespace Ubpa::UECS {
	template<typename Cmpt>
	std::span<Cmpt> ChunkView::GetCmptArray() const {
		auto* ptr = GetCmptArray(TypeID_of<Cmpt>);
		if (!ptr)
			return {};
		return { static_cast<Cmpt*>(ptr), EntityNum() };
	}

	inline std::span<const Entity> ChunkView::GetEntityArray() const {
		auto* ptr = GetCmptArray(TypeID_of<Entity>);
		if (!ptr)
			return {};
		return { static_cast<const Entity*>(ptr), EntityNum() };
	}
}
