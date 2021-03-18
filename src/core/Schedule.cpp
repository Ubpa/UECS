#include <UECS/Schedule.h>

#include "SysFuncGraph.h"

using namespace Ubpa;
using namespace Ubpa::UECS;
using namespace std;

Schedule::~Schedule() {
	Clear();
}

std::string_view Schedule::RegisterFrameString(std::string_view str) {
	auto* buffer = (char*)frame_rsrc.allocate((str.size() + 1) * sizeof(char), alignof(char));
	std::memcpy(buffer, str.data(), str.size() * sizeof(char));
	buffer[str.size()] = 0;
	return str;
}

Schedule& Schedule::Order(string_view x, string_view y) {
	sysFuncOrder.emplace(SystemFunc::GetValue(x), SystemFunc::GetValue(y));
	return *this;
}

Schedule& Schedule::InsertNone(string_view sys, TypeID type) {
	std::size_t hashcode = SystemFunc::GetValue(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseNones.erase(type);
	change.insertNones.insert(type);
	return *this;
}

Schedule& Schedule::EraseNone(string_view sys, TypeID type) {
	std::size_t hashcode = SystemFunc::GetValue(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseNones.insert(type);
	change.insertNones.erase(type);
	return *this;
}

void Schedule::Clear() {
	//auto alloc = std::pmr::polymorphic_allocator<SystemFunc>{ &frame_rsrc };
	for (const auto& [hash, sysFunc] : sysFuncs) {
		sysFunc->~SystemFunc();
		// no need to release
		//alloc.deallocate(sysFunc, 1);
	}
	sysFuncs.clear();
	sysFuncOrder.clear();
	sysFilterChange.clear();
	frame_rsrc.release();
}

Schedule::CmptSysFuncsMap* Schedule::GenCmptSysFuncsMap() const {
	CmptSysFuncsMap* rst = CreateFrameObject<CmptSysFuncsMap>(CmptSysFuncsMap::allocator_type{ &frame_rsrc });

	for (const auto& [hashcode, sysFunc] : sysFuncs) {
		for (const auto& type : sysFunc->entityQuery.locator.AccessTypeIDs()) {
			auto& cmptSysFuncs = rst->try_emplace(type, &frame_rsrc).first->second;
			switch (type.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				cmptSysFuncs.lastFrameSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::WRITE:
				cmptSysFuncs.writeSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::LATEST:
				cmptSysFuncs.latestSysFuncs.push_back(sysFunc);
				break;
			default:
				assert(false);
				break;
			}
		}
		for (const auto& type : sysFunc->singletonLocator.SingletonTypes()) {
			auto& cmptSysFuncs = rst->try_emplace(type, &frame_rsrc).first->second;
			switch (type.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				cmptSysFuncs.lastFrameSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::WRITE:
				cmptSysFuncs.writeSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::LATEST:
				cmptSysFuncs.latestSysFuncs.push_back(sysFunc);
				break;
			default:
				assert(false);
				break;
			}
		}
		for (const auto& type : sysFunc->randomAccessor.types) {
			auto& cmptSysFuncs = rst->try_emplace(type, &frame_rsrc).first->second;
			switch (type.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				cmptSysFuncs.lastFrameSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::WRITE:
				cmptSysFuncs.writeSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::LATEST:
				cmptSysFuncs.latestSysFuncs.push_back(sysFunc);
				break;
			default:
				assert(false);
				break;
			}
		}

		if (sysFunc->GetMode() == SystemFunc::Mode::Chunk) {
			const auto& filter = sysFunc->entityQuery.filter;
			for (const auto& type : filter.all) {
				auto& cmptSysFuncs = rst->try_emplace(type, &frame_rsrc).first->second;
				switch (type.GetAccessMode())
				{
				case AccessMode::LAST_FRAME:
					cmptSysFuncs.lastFrameSysFuncs.push_back(sysFunc);
					break;
				case AccessMode::WRITE:
					cmptSysFuncs.writeSysFuncs.push_back(sysFunc);
					break;
				case AccessMode::LATEST:
					cmptSysFuncs.latestSysFuncs.push_back(sysFunc);
					break;
				default:
					assert(false);
					break;
				}
			}

			for (const auto& type : filter.any) {
				auto& cmptSysFuncs = rst->try_emplace(type, &frame_rsrc).first->second;
				switch (type.GetAccessMode())
				{
				case AccessMode::LAST_FRAME:
					cmptSysFuncs.lastFrameSysFuncs.push_back(sysFunc);
					break;
				case AccessMode::WRITE:
					cmptSysFuncs.writeSysFuncs.push_back(sysFunc);
					break;
				case AccessMode::LATEST:
					cmptSysFuncs.latestSysFuncs.push_back(sysFunc);
					break;
				default:
					assert(false);
					break;
				}
			}
		}
	}
	return rst;
}

SysFuncGraph* Schedule::GenSysFuncGraph() const {
	// [change func Filter]
	for (const auto& [hashcode, change] : sysFilterChange) {
		auto target = sysFuncs.find(hashcode);
		if (target == sysFuncs.end())
			continue;

		auto* func = target->second;

		for (const auto& type : change.insertNones)
			func->entityQuery.filter.none.insert(type);
		for (const auto& type : change.eraseNones)
			func->entityQuery.filter.none.erase(type);
	}

	CmptSysFuncsMap* cmptSysFuncsMap = GenCmptSysFuncsMap(); // use frame rsrc, no need to release

	// [gen graph]
	SysFuncGraph* graph = CreateFrameObject<SysFuncGraph>(&frame_rsrc);

	// [gen graph] - vertex
	for (const auto& [hashcode, sysFunc] : sysFuncs)
		graph->AddVertex(sysFunc);
	
	// [gen graph] - edge - order
	for (const auto& [x, y] : sysFuncOrder) {
		auto target_x = sysFuncs.find(x);
		if (target_x == sysFuncs.end())
			continue;
		auto target_y = sysFuncs.find(y);
		if (target_y == sysFuncs.end())
			continue;

		auto* sysFunc_x = target_x->second;
		auto* sysFunc_y = target_y->second;
		graph->AddEdge(sysFunc_x, sysFunc_y);
	}

	// [gen graph] - edge - last frame -> write -> latest
	for (const auto& [type, cmptSysFuncs] : *cmptSysFuncsMap) {
		if (cmptSysFuncs.writeSysFuncs.empty())
			continue;

		auto [success, sorted_wirtes] = graph->SubGraph({ cmptSysFuncs.writeSysFuncs.data(), cmptSysFuncs.writeSysFuncs.size() }).Toposort();
		assert(success);
		for (auto* sys : cmptSysFuncs.lastFrameSysFuncs)
			graph->AddEdge(sys, sorted_wirtes.front());
		for (auto* sys : cmptSysFuncs.latestSysFuncs)
			graph->AddEdge(sorted_wirtes.back(), sys);
		for (std::size_t i = 0; i < sorted_wirtes.size() - 1; i++)
			graph->AddEdge(sorted_wirtes[i], sorted_wirtes[i + 1]);
	}

	return graph;
}
