#pragma once

#include <thread>

namespace Ubpa {
	template<typename... Cmpts>
	std::tuple<Entity*, Cmpts*...> World::CreateEntity() {
		auto rst = mngr->CreateEntity<Cmpts...>();
		return {reinterpret_cast<Entity*>(std::get<0>(rst)),
			std::get<1 + Find_v<TypeList<Cmpts...>, Cmpts>>(rst)...};
	}
}

namespace Ubpa::detail::World_ {
	template<typename... Cmpts>
	struct Each<TypeList<Cmpts * ...>> {
		static_assert(sizeof...(Cmpts) > 0);
		using CmptList = TypeList<Cmpts...>;
		template<typename Sys>
		static void run(World* w, Sys&& s) {
			for (auto archetype : w->mngr->GetArchetypeWith<Cmpts...>()) {
				auto cmptsVecTuple = archetype->Locate<Cmpts...>();
				size_t num = archetype->Size();
				size_t chunkNum = archetype->ChunkNum();
				size_t chunkCapacity = archetype->ChunkCapacity();

				for (size_t i = 0; i < chunkNum; i++) {
					auto cmptsTuple = std::make_tuple(std::get<Find_v<CmptList, Cmpts>>(cmptsVecTuple)[i]...);
					size_t J = std::min(chunkCapacity, num - (i * chunkCapacity));
					for (size_t j = 0; j < J; j++)
						s((std::get<Find_v<CmptList, Cmpts>>(cmptsTuple) + j)...);
				}
			}
		}
	};

	template<typename... Cmpts>
	struct ParallelEach<TypeList<Cmpts * ...>> {
		static_assert(sizeof...(Cmpts) > 0);
		using CmptList = TypeList<Cmpts...>;
		template<typename Sys>
		static void run(World* w, Sys&& s) {
			for (auto archetype : w->mngr->GetArchetypeWith<Cmpts...>()) {
				auto cmptsVecTuple = archetype->Locate<Cmpts...>();
				size_t num = archetype->Size();
				size_t chunkNum = archetype->ChunkNum();
				size_t chunkCapacity = archetype->ChunkCapacity();

				size_t coreNum = std::thread::hardware_concurrency();
				assert(coreNum > 0);

				auto job = [=](size_t ID) {
					for (size_t i = ID; i < chunkNum; i += coreNum) {
						auto cmptsTuple = std::make_tuple(std::get<Find_v<CmptList, Cmpts>>(cmptsVecTuple)[i]...);
						size_t J = std::min(chunkCapacity, num - (i * chunkCapacity));
						for (size_t j = 0; j < J; j++)
							s((std::get<Find_v<CmptList, Cmpts>>(cmptsTuple) + j)...);
					}
				};

				std::vector<std::thread> workers;
				for (size_t i = 0; i < coreNum; i++)
					workers.emplace_back(job, i);

				for (auto& worker : workers)
					worker.join();
			}
		}
	};
}
