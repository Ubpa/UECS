#include <UECS/RTDCmptsView.h>

#include <UECS/EntityLocator.h>

using namespace Ubpa;
using namespace std;

RTDCmptsView::Iterator RTDCmptsView::begin() const noexcept {
	return { locator, locator->CmptTypes().begin(), cmpts };
}

RTDCmptsView::Iterator RTDCmptsView::end() const noexcept {
	return { locator, locator->CmptTypes().end(), cmpts + locator->CmptTypes().size() };
}

const set<CmptType>& RTDCmptsView::CmptTypes() const noexcept {
	return locator->CmptTypes();
}

RTDCmptsView::CmptHandle RTDCmptsView::Iterator::operator*() const {
	return { *typeIter, *ptr_cmpt, locator->GetCmptTagMode(*typeIter) };
}

const RTDCmptsView::CmptHandle* RTDCmptsView::Iterator::operator->() const noexcept {
	handle = { *typeIter, *ptr_cmpt, locator->GetCmptTagMode(*typeIter) };
	return &handle;
}
