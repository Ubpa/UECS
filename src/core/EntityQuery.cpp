#include <UECS/EntityQuery.h>

#include <UECS/detail/Util.h>

using namespace Ubpa;

EntityQuery::EntityQuery(EntityFilter filter, EntityLocator locator)
	: filter{ std::move(filter) }, locator{ std::move(locator) }, hashCode{ hash_combine(filter.HashCode(), locator.HashCode()) }
{
}
