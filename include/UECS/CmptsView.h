#pragma once

#include "CmptPtr.h"
#include "CmptType.h"
#include "CmptTag.h"

#include <set>

namespace Ubpa::UECS {
	class EntityLocator;

	// use CmptsView::Iterator to read CmptPtr
	class CmptsView {
	public:
		CmptsView(EntityLocator* locator, void** cmpts)
			: locator{ locator }, cmpts{ cmpts }{}

		// check AccessMode
		CmptPtr GetCmpt(CmptType) const;

		template<typename Cmpt>
		LastFrame<Cmpt> GetCmptAsLastFrame() const {
			return GetCmpt(CmptType::Of<LastFrame<Cmpt>>).AsLastFrame<Cmpt>();
		}

		template<typename Cmpt>
		Write<Cmpt> GetCmptAsWrite() const {
			return GetCmpt(CmptType::Of<Write<Cmpt>>).AsWrite<Cmpt>();
		}

		template<typename Cmpt>
		Latest<Cmpt> GetCmptAsLatest() const {
			return GetCmpt(CmptType::Of<Latest<Cmpt>>).AsLatest<Cmpt>();
		}

		void* const* Components() const noexcept { return cmpts; }
	private:
		EntityLocator* locator;
		void* const* cmpts;
	};
}
