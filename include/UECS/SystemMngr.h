#pragma once

#include "Schedule.h"

namespace Ubpa{
	// System Manager
	// System is a struct with specific function
	// signature: static void OnUpdate(Schedule&)
	class SystemMngr {
	public:
		template<typename... Systems>
		void Register();
		template<typename System>
		bool IsRegistered() const;
		template<typename System>
		void Deregister() noexcept;

	private:
		template<typename System>
		void RegisterOne();

		struct SystemLifecycle {
			void(*OnUpdate)(Schedule&);
		};

		std::unordered_map<size_t, SystemLifecycle> lifecycleMap;

		friend class World;
	};
}

#include "detail/SystemMngr.inl"
