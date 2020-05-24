#pragma once

#include "CmptPtr.h"
#include "CmptType.h"

#include <set>

namespace Ubpa {
	class EntityLocator;

	// use RTDCmptViewer::Iterator to read CmptPtr
	// no read/write control
	class RTDCmptViewer {
	public:
		// forward
		class Iterator /*: public std::iterator<std::forward_iterator_tag, CmptPtr>*/ {
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = CmptPtr;
			using difference_type = std::ptrdiff_t;
			using pointer = CmptPtr*;
			using reference = CmptPtr&;

			Iterator(std::set<CmptType>::iterator typeIter = std::set<CmptType>::iterator{}, void* const* ptr_cmpt = nullptr)
				: typeIter(typeIter), ptr_cmpt{ ptr_cmpt } {}
			bool operator==(const Iterator& rhs) const noexcept {
				return ptr_cmpt == rhs.ptr_cmpt;
			}
			bool operator!=(const Iterator& rhs) const noexcept {
				return !this->operator==(rhs);
			}
			CmptPtr operator*() const noexcept { return cmptptr; }
			const CmptPtr* operator->() const noexcept {
				cmptptr = { *typeIter, *ptr_cmpt }; 
				return &cmptptr;
			}
			Iterator& operator++() {
				typeIter++;
				ptr_cmpt++;
				return *this;
			}
		private:
			std::set<CmptType>::iterator typeIter;
			void* const* ptr_cmpt;
			mutable CmptPtr cmptptr{ CmptType::Invalid(), nullptr };
		};

		RTDCmptViewer(EntityLocator* locator, void** cmpts)
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
