#pragma once

namespace Ubpa::UECS {
	template<typename Cmpt>
	Span<Cmpt> ChunkView::GetCmptArray() const {
		auto* ptr = GetCmptArray(CmptType::Of<Cmpt>);
		if (!ptr)
			return {};
		return { static_cast<Cmpt*>(ptr), EntityNum() };
	}

	inline Span<const Entity> ChunkView::GetEntityArray() const {
		auto* ptr = GetCmptArray(CmptType::Of<Entity>);
		if (!ptr)
			return {};
		return { static_cast<const Entity*>(ptr), EntityNum() };
	}
}
