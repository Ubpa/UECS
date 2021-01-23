#pragma once

#include <functional>
#include <unordered_map>
#include <array>

namespace Ubpa::UECS {
	class World;
	class Schedule;
	
	class SystemTraits {
	public:
		// [life cycle]
		//     OnCreate
		//         |
		//         v
		//     OnActivate <---
		//         |         |
		//         v         |
		// --> OnUpdate      |
		// |       |         |
		// --------x         |
		//         |         |
		//         v         |
		//     OnDeactivate  |
		//         |         |
		//         x----------
		//         |
		//         v
		//     OnDestroy
		using OnCreate     = void(World*);
		using OnActivate   = void(World*);
		using OnUpdate     = void(Schedule&);
		using OnDeactivate = void(World*);
		using OnDestroy    = void(World*);

		SystemTraits() = default;
		SystemTraits(const SystemTraits&);
		SystemTraits(SystemTraits&&) noexcept = default;
		SystemTraits& operator=(SystemTraits&);
		SystemTraits& operator=(SystemTraits&&) noexcept = default;

		// register system's name and get an ID
		// if it is already registered, return it's ID directly
		std::size_t Register(std::string name);

		// ID must exist
		void RegisterOnCreate    (std::size_t ID, std::function<OnCreate>);
		void RegisterOnActivate  (std::size_t ID, std::function<OnActivate>);
		void RegisterOnUpdate    (std::size_t ID, std::function<OnUpdate>);
		void RegisterOnDeactivate(std::size_t ID, std::function<OnDeactivate>);
		void RegisterOnDestroy   (std::size_t ID, std::function<OnDestroy>);

		std::string_view Nameof(std::size_t ID) const noexcept;
		std::size_t GetID(std::string_view name) const;
		bool IsRegistered(std::size_t ID) const noexcept;
		const auto& GetNameIDMap() const noexcept { return name2id; }

		// [ Template ] functions
		///////////////////////////

		template<typename System>
		static std::string_view StaticNameof() noexcept;

		// for each <System> in <Systems...>
		// 1. Register(StaticNameof<System>())
		// 2. RegisterOn{Create|Activate|Update|Deactivate|Destroy} if <System> has them
		template<typename... Systems>
		std::array<std::size_t, sizeof...(Systems)> Register();

		template<typename System>
		std::size_t GetID() const { return GetID(StaticNameof<System>()); }

		template<typename System>
		bool IsRegistered() const { return GetID<System>(); }
		
	private:
		friend class SystemMngr;
		
		void Create    (std::size_t ID, World*) const;
		void Activate  (std::size_t ID, World*) const;
		void Update    (std::size_t ID, Schedule&) const;
		void Deactivate(std::size_t ID, World*) const;
		void Destroy   (std::size_t ID, World*) const;

		std::vector<std::string> names;
		std::unordered_map<std::string_view, std::size_t> name2id;

		std::unordered_map<std::size_t, std::function<OnCreate    >> createMap;
		std::unordered_map<std::size_t, std::function<OnActivate  >> activateMap;
		std::unordered_map<std::size_t, std::function<OnUpdate    >> updateMap;
		std::unordered_map<std::size_t, std::function<OnDeactivate>> deactivateMap;
		std::unordered_map<std::size_t, std::function<OnDestroy   >> destroyMap;
	};
}

#include "details/SystemTraits.inl"
