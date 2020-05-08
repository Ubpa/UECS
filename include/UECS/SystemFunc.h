#pragma once

#include "EntityQuery.h"

#include <functional>

namespace Ubpa {
	class Entity;

	class SystemFunc {
	public:
		EntityQuery query;
		bool IsNeedEntity() const noexcept { return needEntity; }
		
		template<typename Func>
		SystemFunc(Func&& func, std::string name, EntityFilter filter = EntityFilter{});

		template<typename Func>
		SystemFunc(Func&& func, EntityFilter filter = EntityFilter{});
		
		const std::string& Name() const noexcept { return name; }

		static size_t HashCode(std::string_view name) { return std::hash<std::string_view>{}(name); }

		size_t HashCode() const noexcept { return hashCode; }

		void operator()(Entity* e, void** cmptArr) { return func(e, cmptArr); }

	private:
		template<typename Func, typename ArgList>
		SystemFunc(Func&& func, std::string name, EntityFilter filter, ArgList);

		std::function<void(Entity*, void**)> func;

		std::string name;
		size_t hashCode; // after name

		bool needEntity;
	};
}

#include "detail/SystemFunc.inl"
