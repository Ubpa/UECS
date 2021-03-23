#pragma once

#include "SystemTraits.hpp"

#include <unordered_set>

namespace Ubpa::UECS {
	class Schedule;

	class SystemMngr {
	public:
		SystemTraits systemTraits;

		const auto& GetAliveSystemIDs() const noexcept { return aliveSystemIDs; }
		const auto& GetActiveSystemIDs() const noexcept { return activeSystemIDs; }

		// not alive -> create
		void Create(NameID);
		
		// 1. not alive create -> create and activate
		// 2. not active -> then activate
		void Activate(NameID);

		// active -> deactavate
		void Deactivate(NameID);
		
		// 1. active -> deactavite
		// 2. alive -> destroy
		void Destroy(NameID);

		bool IsAlive(NameID) const;
		bool IsActive(NameID) const;

		//
		// [ Template ] Functions
		///////////////////////////

		template<typename... Systems> void Create();
		template<typename... Systems> void Activate();
		template<typename... Systems> void Deactivate();
		template<typename... Systems> void Destroy();
		template<typename System> bool IsAlive() const;
		template<typename System> bool IsActive() const;

		template<typename... Systems>
		std::array<Name, sizeof...(Systems)> RegisterAndCreate();

		template<typename... Systems>
		std::array<Name, sizeof...(Systems)> RegisterAndActivate();

		SystemMngr(const SystemMngr&) = delete;
		SystemMngr(SystemMngr&&) noexcept = delete;
		SystemMngr& operator=(const SystemMngr&) = delete;
		SystemMngr& operator=(SystemMngr&&) noexcept = delete;
		
	private:
		friend class World;

		SystemMngr(World* w) : w{ w } {}
		SystemMngr(const SystemMngr& mngr, World* w);
		SystemMngr(SystemMngr&& mngr, World* w) noexcept;
		~SystemMngr();

		World* w;
		void Update(NameID, Schedule&) const;
		void Clear();
		
		std::unordered_set<NameID> aliveSystemIDs;
		std::unordered_set<NameID> activeSystemIDs;
	};
}

#include "details/SystemMngr.inl"
