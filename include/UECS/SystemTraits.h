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
		using OnCreate     = std::function<void(World*   )>;
		using OnActivate   = std::function<void(World*   )>;
		using OnUpdate     = std::function<void(Schedule&)>;
		using OnDeactivate = std::function<void(World*   )>;
		using OnDestroy    = std::function<void(World*   )>;

		SystemTraits() = default;
		SystemTraits(const SystemTraits&);
		SystemTraits(SystemTraits&&) noexcept = default;

		size_t Register(std::string name);
		void RegisterOnCreate(size_t ID, OnCreate);
		void RegisterOnActivate(size_t ID, OnActivate);
		void RegisterOnUpdate(size_t ID, OnUpdate);
		void RegisterOnDeactivate(size_t ID, OnDeactivate);
		void RegisterOnDestroy(size_t ID, OnDestroy);

		bool IsRegistered(size_t ID) const noexcept;
		size_t GetID(std::string_view name) const;
		std::string_view Nameof(size_t ID) const noexcept;
		const auto& GetNameIDMap() const noexcept { return name2id; }

		template<typename... Systems>
		std::array<size_t, sizeof...(Systems)> Register();
		template<typename System>
		static std::string_view StaticNameof() noexcept;
		
	private:
		friend class SystemMngr;
		
		void Create(size_t ID, World*) const;
		void Activate(size_t ID, World*) const;
		void Update(size_t ID, Schedule&) const;
		void Deactivate(size_t ID, World*) const;
		void Destroy(size_t ID, World*) const;

		std::vector<std::string> names;
		std::unordered_map<std::string_view, size_t> name2id;

		std::unordered_map<size_t, OnCreate>     createMap;
		std::unordered_map<size_t, OnActivate>   activateMap;
		std::unordered_map<size_t, OnUpdate>     updateMap;
		std::unordered_map<size_t, OnDeactivate> deactivateMap;
		std::unordered_map<size_t, OnDestroy>    destroyMap;
	};
}

#include "detail/SystemTraits.inl"
