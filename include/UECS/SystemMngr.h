#pragma once

#include "SystemTraits.h"

#include <unordered_set>

namespace Ubpa::UECS {
	class Schedule;

	class SystemMngr {
	public:
		SystemTraits systemTraits;
		
		SystemMngr(World* w) : w{w}{}
		SystemMngr(const SystemMngr& mngr, World* w);
		SystemMngr(SystemMngr&& mngr, World* w) noexcept;
		~SystemMngr();

		// not alive -> create
		void Create(size_t systemID);
		
		// 1. not alive create -> create and activate
		// 2. not active -> then activate
		void Activate(size_t systemID);

		// active -> deactavate
		void Deactivate(size_t systemID);
		
		// 1. active -> deactavite
		// 2. alive -> destroy
		void Destroy(size_t systemID);

		bool IsAlive(size_t systemID) const;
		bool IsActive(size_t systemID) const;

		const auto& GetAliveSystemIDs() const noexcept { return aliveSystemIDs; }
		const auto& GetActiveSystemsIDs() const noexcept { return activeSystemIDs; }

		SystemMngr(const SystemMngr&) = delete;
		SystemMngr(SystemMngr&&) noexcept = delete;
		SystemMngr& operator=(const SystemMngr&) = delete;
		SystemMngr& operator=(SystemMngr&&) noexcept = delete;
		
	private:
		friend class World;
		World* w;
		void Update(size_t systemID, Schedule&) const;
		void Clear();
		
		std::unordered_set<size_t> aliveSystemIDs;
		std::unordered_set<size_t> activeSystemIDs;
	};
}
