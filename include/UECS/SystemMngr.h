#pragma once

#include <vector>
#include <map>
#include <set>
#include <functional>
#include <string>

namespace Ubpa::UECS {
	class Schedule;

	class SystemMngr {
	public:
		SystemMngr() = default;
		SystemMngr(const SystemMngr&);

		using Func = std::function<void(Schedule&)>;
		struct SystemInfo {
			Func func;
			std::string name;
		};
		const std::vector<SystemInfo>& GetSystems() const noexcept { return systems; }
		const std::set<size_t>& GetActiveSystemIndices() const noexcept { return activeSystemIndices; }
		const std::map<std::string_view, size_t>& GetNameToIndexMap() const noexcept { return name2idx; }

		// if unregister, return static_cast<size_t>(-1)
		size_t GetIndex(std::string_view name) const;
		// name: nameof::nameof_type<System>
		template<typename System>
		size_t GetIndex() const;

		void Clear();

		size_t Register(std::string name, Func);
		// name: nameof::nameof_type<System>
		// func: static void System::OnUpdate(Schedule&);
		template<typename... Systems>
		std::array<size_t, sizeof...(Systems)> Register();
		void Unregister(size_t idx);
		void Unregister(std::string_view name);
		// name: nameof::nameof_type<System>
		template<typename System>
		void Unregister();

		void Activate(size_t index);
		void Deactivate(size_t index);
	private:
		std::vector<SystemInfo> systems;
		std::vector<size_t> frees;
		std::map<std::string_view, size_t> name2idx;
		std::set<size_t> activeSystemIndices;
	};
}

#include "detail/SystemMngr.inl"
