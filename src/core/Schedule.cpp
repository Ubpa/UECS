#include <UECS/Schedule.hpp>

#include "SysFuncGraph.hpp"

using namespace Ubpa;
using namespace Ubpa::UECS;
using namespace std;

Schedule::Schedule(World* w)
	: world{w} {}

Schedule::~Schedule() { Clear(); }

Schedule::Schedule(const Schedule& other, World* w) :
	world{ w },
	layerInfos{ other.layerInfos }
{
	for (auto& [layer, info] : layerInfos) {
		for (auto& [id, sys] : info.sysFuncs) {
			sys = world->UnsyncNewFrameObject<SystemFunc>(*sys);
			sys->name = RegisterFrameString(sys->Name());
		}
	}
}

Schedule::Schedule(Schedule&& other, World* w) noexcept :
	world{ w },
	layerInfos{ std::move(other.layerInfos) } {}

std::pmr::memory_resource* Schedule::GetUnsyncFrameResource() const noexcept {
	return world->GetUnsyncFrameResource();
}

std::string_view Schedule::RegisterFrameString(std::string_view str) {
	auto* buffer = (char*)world->GetUnsyncFrameResource()->allocate((str.size() + 1) * sizeof(char), alignof(char));
	std::memcpy(buffer, str.data(), str.size() * sizeof(char));
	buffer[str.size()] = 0;
	return str;
}

Schedule& Schedule::Order(string_view x, string_view y, int layer) {
	assert(layer != SpecialLayer);
	layerInfos[layer].sysFuncOrder.emplace(SystemFunc::GetValue(x), SystemFunc::GetValue(y));
	return *this;
}

Schedule& Schedule::AddNone(string_view sys, TypeID type, int layer) {
	assert(layer != SpecialLayer);
	std::size_t hashcode = SystemFunc::GetValue(sys);
	layerInfos[layer].sysNones[hashcode].push_back(type);
	return *this;
}

Schedule& Schedule::Disable(std::string_view sys, int layer) {
	assert(layer != SpecialLayer);
	layerInfos[layer].disabledSysFuncs.insert(SystemFunc::GetValue(sys));
	return *this;
}

World* Schedule::GetWorld() const noexcept { return world; }

void Schedule::Clear() {
	for (auto& [layer, layerinfo] : layerInfos) {
		//auto alloc = std::pmr::polymorphic_allocator<SystemFunc>{ frame_rsrc.get() };
		for (const auto& [hash, sysFunc] : layerinfo.sysFuncs) {
			sysFunc->~SystemFunc();
			// no need to deallocate
			//alloc.deallocate(sysFunc, 1);
		}
		layerinfo.disabledSysFuncs.clear();
		layerinfo.sysFuncs.clear();
		layerinfo.sysFuncOrder.clear();
		layerinfo.sysNones.clear();
	}
	layerInfos.clear();
}

Schedule::CmptSysFuncsMap* Schedule::GenCmptSysFuncsMap(int layer) const {
	assert(layer != SpecialLayer);
	const auto& layerinfo = layerInfos.at(layer);
	const auto& sysFuncs = layerinfo.sysFuncs;
	const auto& disabledSysFuncs = layerinfo.disabledSysFuncs;

	CmptSysFuncsMap* rst = world->UnsyncNewFrameObject<CmptSysFuncsMap>();
	for (const auto& [hashcode, sysFunc] : sysFuncs) {
		if (disabledSysFuncs.contains(hashcode))
			continue;

		for (const auto& type : sysFunc->entityQuery.locator.AccessTypeIDs()) {
			auto& cmptSysFuncs = rst->try_emplace(type, world->GetUnsyncFrameResource()).first->second;
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
			auto& cmptSysFuncs = rst->try_emplace(type, world->GetUnsyncFrameResource()).first->second;
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
			auto& cmptSysFuncs = rst->try_emplace(type, world->GetUnsyncFrameResource()).first->second;
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
				auto& cmptSysFuncs = rst->try_emplace(type, world->GetUnsyncFrameResource()).first->second;
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
				auto& cmptSysFuncs = rst->try_emplace(type, world->GetUnsyncFrameResource()).first->second;
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

SysFuncGraph* Schedule::GenSysFuncGraph(int layer) const {
	assert(layer != SpecialLayer);
	const auto& layerinfo = layerInfos.at(layer);
	const auto& sysNones = layerinfo.sysNones;
	const auto& sysFuncs = layerinfo.sysFuncs;
	const auto& sysFuncOrder = layerinfo.sysFuncOrder;
	const auto& disabledSysFuncs = layerinfo.disabledSysFuncs;

	// [change func Filter]
	for (const auto& [hashcode, nones] : sysNones) {
		auto target = sysFuncs.find(hashcode);
		if (target == sysFuncs.end())
			continue;

		auto* func = target->second;

		for (const auto& type : nones)
			func->entityQuery.filter.none.insert(type);
	}

	// not contains disabled system functions
	CmptSysFuncsMap* cmptSysFuncsMap = GenCmptSysFuncsMap(layer); // use frame rsrc, no need to release

	// [gen graph]
	SysFuncGraph* graph = world->UnsyncNewFrameObject<SysFuncGraph>();

	// [gen graph] - vertex
	for (const auto& [hashcode, sysFunc] : sysFuncs) {
		if (disabledSysFuncs.contains(hashcode))
			continue;
		graph->AddVertex(sysFunc);
	}
	
	// [gen graph] - edge - order
	for (const auto& [x, y] : sysFuncOrder) {
		auto target_x = sysFuncs.find(x);
		if (target_x == sysFuncs.end())
			continue;
		auto target_y = sysFuncs.find(y);
		if (target_y == sysFuncs.end())
			continue;
		if (disabledSysFuncs.contains(x) || disabledSysFuncs.contains(y))
			continue;

		auto* sysFunc_x = target_x->second;
		auto* sysFunc_y = target_y->second;
		graph->AddEdge(sysFunc_x, sysFunc_y);
	}

	// [gen graph] - edge - last frame -> write -> latest
	for (const auto& [type, cmptSysFuncs] : *cmptSysFuncsMap) {
		if (cmptSysFuncs.writeSysFuncs.empty())
			continue;

		for (auto* sys : cmptSysFuncs.lastFrameSysFuncs) {
			for (auto* w : cmptSysFuncs.writeSysFuncs)
				graph->AddEdge(sys, w);
		}

		for (auto* sys : cmptSysFuncs.latestSysFuncs) {
			for (auto* w : cmptSysFuncs.writeSysFuncs)
				graph->AddEdge(w, sys);
		}
	}

	// [gen graph] - edge - toposort write
	for (const auto& [type, cmptSysFuncs] : *cmptSysFuncsMap) {
		if (cmptSysFuncs.writeSysFuncs.empty())
			continue;

		SysFuncGraph* subgraph = world->UnsyncNewFrameObject<SysFuncGraph>();
		graph->SubGraph(*subgraph, std::span{ cmptSysFuncs.writeSysFuncs.data(), cmptSysFuncs.writeSysFuncs.size() });
		auto [success, sorted_wirtes] = subgraph->Toposort();
		assert(success);
		for (std::size_t i = 0; i < sorted_wirtes.size() - 1; i++)
			graph->AddEdge(sorted_wirtes[i], sorted_wirtes[i + 1]);
	}

	return graph;
}
