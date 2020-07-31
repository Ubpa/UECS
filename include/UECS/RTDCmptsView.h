#pragma once

#include "CmptPtr.h"
#include "CmptType.h"
#include "CmptTag.h"

#include <set>

namespace Ubpa::UECS {
	class EntityLocator;

	// use RTDCmptsView::Iterator to read CmptPtr
	// use CmptHandle for read/write control
	// use begin() and end() to iterate
	class RTDCmptsView {
	public:
		// for read/write control
		class CmptHandle {
		public:
			CmptHandle(CmptType type, void* cmpt, AccessMode mode)
				: type{ type }, cmpt{ cmpt }, mode{ mode }{}

			CmptType GetCmptType() const noexcept { return type; }
			AccessMode GetMode() const noexcept { return mode; }
			
			CmptCPtr AsLastFrame() const noexcept {
				assert(mode == AccessMode::LAST_FRAME);
				return { type, cmpt };
			}
			CmptPtr AsWrite() const noexcept {
				assert(mode == AccessMode::WRITE);
				return { type, cmpt };
			}
			CmptCPtr AsLatest() const noexcept {
				assert(mode == AccessMode::LATEST);
				return { type, cmpt };
			}
		private:
			CmptType type;
			void* cmpt;
			AccessMode mode;
		};

		// forward
		class Iterator /*: public std::iterator<std::forward_iterator_tag, CmptPtr>*/ {
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = CmptPtr;
			using difference_type = std::ptrdiff_t;
			using pointer = CmptPtr*;
			using reference = CmptPtr&;

			Iterator(EntityLocator* locator, std::set<CmptType>::iterator typeIter = std::set<CmptType>::iterator{}, void* const* ptr_cmpt = nullptr)
				: locator{ locator }, typeIter(typeIter), ptr_cmpt{ ptr_cmpt } {}
			bool operator==(const Iterator& rhs) const noexcept {
				return ptr_cmpt == rhs.ptr_cmpt;
			}
			bool operator!=(const Iterator& rhs) const noexcept {
				return ptr_cmpt != rhs.ptr_cmpt;
			}
			CmptHandle operator*() const;
			const CmptHandle* operator->() const noexcept;
			Iterator& operator++() {
				typeIter++;
				ptr_cmpt++;
				return *this;
			}
		private:
			EntityLocator* locator;
			std::set<CmptType>::iterator typeIter;
			void* const* ptr_cmpt;
			mutable CmptHandle handle{ CmptType::Invalid(), nullptr, AccessMode{} };
		};

		RTDCmptsView(EntityLocator* locator, void** cmpts)
			: locator{ locator }, cmpts{ cmpts }{}

		Iterator begin() const noexcept;
		Iterator end() const noexcept;

		const std::set<CmptType>& CmptTypes() const noexcept;
		void* const* Components() const noexcept { return cmpts; }

	private:
		EntityLocator* locator;
		void* const* cmpts;
	};
}
