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
		size_t Register(std::string name);

		// ID must exist
		void RegisterOnCreate    (size_t ID, std::function<OnCreate>);
		void RegisterOnActivate  (size_t ID, std::function<OnActivate>);
		void RegisterOnUpdate    (size_t ID, std::function<OnUpdate>);
		void RegisterOnDeactivate(size_t ID, std::function<OnDeactivate>);
		void RegisterOnDestroy   (size_t ID, std::function<OnDestroy>);

		std::string_view Nameof(size_t ID) const noexcept;
		size_t GetID(std::string_view name) const;
		bool IsRegistered(size_t ID) const noexcept;
		const auto& GetNameIDMap() const noexcept { return name2id; }

		// [ Template ] functions
		///////////////////////////

		template<typename System>
		static std::string_view StaticNameof() noexcept;

		// for each <System> in <Systems...>
		// 1. Register(StaticNameof<System>())
		// 2. RegisterOn{Create|Activate|Update|Deactivate|Destroy} if <System> has them
		template<typename... Systems>
		std::array<size_t, sizeof...(Systems)> Register();

		template<typename System>
		size_t GetID() const { return GetID(StaticNameof<System>()); }

		template<typename System>
		bool IsRegistered() const { return GetID<System>(); }
		
	private:
		friend class SystemMngr;
		
		void Create    (size_t ID, World*) const;
		void Activate  (size_t ID, World*) const;
		void Update    (size_t ID, Schedule&) const;
		void Deactivate(size_t ID, World*) const;
		void Destroy   (size_t ID, World*) const;

		std::vector<std::string> names;
		std::unordered_map<std::string_view, size_t> name2id;

		std::unordered_map<size_t, std::function<OnCreate    >> createMap;
		std::unordered_map<size_t, std::function<OnActivate  >> activateMap;
		std::unordered_map<size_t, std::function<OnUpdate    >> updateMap;
		std::unordered_map<size_t, std::function<OnDeactivate>> deactivateMap;
		std::unordered_map<size_t, std::function<OnDestroy   >> destroyMap;
	};
}

#include "detail/SystemTraits.inl"
