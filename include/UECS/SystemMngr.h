#pragma once

#include "Schedule.h"

#include <UContainer/xSTL/xMap.h>

namespace Ubpa::UECS{
	class IListener;

	// System Manager
	// System is a struct with specific function
	// signature: static void OnUpdate(Schedule&)
	class SystemMngr {
	public:
		void Register(std::string name, std::function<void(Schedule&)> onUpdate) {
			onUpdateMap.emplace(std::move(name), std::move(onUpdate));
		}
		bool IsRegister(std::string_view name) const {
			return onUpdateMap.find(name) != onUpdateMap.end();
		}
		void Deregister(std::string_view name) {
			onUpdateMap.erase(onUpdateMap.find(name));
		}

		template<typename... Systems>
		void Register();
		template<typename System>
		bool IsRegistered() const;
		template<typename... Systems>
		void Deregister() noexcept;

		void Accept(IListener* listener) const;

	private:
		template<typename System>
		void RegisterOne();
		template<typename System>
		void DeregisterOne();

		xMap<std::string, std::function<void(Schedule&)>> onUpdateMap;

		friend class World;
	};
}

#include "detail/SystemMngr.inl"
