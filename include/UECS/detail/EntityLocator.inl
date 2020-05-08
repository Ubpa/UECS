#pragma once

#include "CmptTag.inl"
#include <UTemplate/Func.h>

namespace Ubpa {
	template<typename TaggedCmptList>
	EntityLocator::EntityLocator(TaggedCmptList)
		: EntityLocator{ Filter_t<TaggedCmptList, CmptTag::IsLastFrame>{},
		Filter_t<TaggedCmptList, CmptTag::IsWrite>{},
		Filter_t<TaggedCmptList, CmptTag::IsLatest>{} } {}

	template<typename... LastFrameCmpts, typename... WriteCmpts, typename... LatestCmpts>
	EntityLocator::EntityLocator(TypeList<LastFrameCmpts...>, TypeList<WriteCmpts...>, TypeList<LatestCmpts...>)
		: lastFrameCmptTypes{ CmptType::Of<LastFrameCmpts>()... },
		writeCmptTypes{ CmptType::Of<WriteCmpts>()... },
		latestCmptTypes{ CmptType::Of<LatestCmpts>()... },
		cmptTypes{ CmptType::Of<LastFrameCmpts>()..., CmptType::Of<WriteCmpts>()...,CmptType::Of<LatestCmpts>()... },
		hashCode{ GenHashCode() }
	{}
}
