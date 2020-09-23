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
		const std::vector<void(*)(Schedule&)>& GetSystems() const noexcept { return systems; }
		const std::set<size_t>& GetActiveSystemIndices() const noexcept { return activeSystemIndices; }
		const std::map<std::string, size_t, std::less<>>& GetNameToIndexMap() const noexcept { return name2idx; }

		// if unregister, return static_cast<size_t>(-1)
		size_t GetIndex(std::string_view name) const;
		// name: nameof::nameof_type<System>
		template<typename System>
		size_t GetIndex() const;

		void Clear();

		size_t Register(std::string name, void(*)(Schedule&));
		// name: nameof::nameof_type<System>
		// func: static void System::OnUpdate(Schedule&);
		template<typename System>
		size_t Register();

		void Activate(size_t index);
		void Deactivate(size_t index);
	private:
		std::vector<void(*)(Schedule&)> systems;
		std::map<std::string, size_t, std::less<>> name2idx;
		std::set<size_t> activeSystemIndices;
	};
}

#include "detail/SystemMngr.inl"
