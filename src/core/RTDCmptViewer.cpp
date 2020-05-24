#include <UECS/RTDCmptViewer.h>

#include <UECS/EntityLocator.h>

using namespace Ubpa;
using namespace std;

RTDCmptViewer::Iterator RTDCmptViewer::begin() const noexcept {
	return { locator->CmptTypes().begin(), cmpts };
}

RTDCmptViewer::Iterator RTDCmptViewer::end() const noexcept {
	return { locator->CmptTypes().end(), cmpts + locator->CmptTypes().size() };
}

const set<CmptType>& RTDCmptViewer::CmptTypes() const noexcept {
	return locator->CmptTypes();
}
