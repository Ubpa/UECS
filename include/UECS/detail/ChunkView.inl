#pragma once

namespace Ubpa::UECS {
	template<typename Cmpt>
	Span<Cmpt> ChunkView::GetCmptArray() const {
		auto* ptr = GetCmptArray(CmptType::Of<Cmpt>);
		if(ptr)
		return { static_cast<Cmpt*>(GetCmptArray(CmptType::Of<Cmpt>)), EntityNum() };
	}
}
