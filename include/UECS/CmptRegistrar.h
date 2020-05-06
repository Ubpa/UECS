#pragma once

#include <unordered_set>

namespace Ubpa {
	class CmptRegistrar {
	public:
		static CmptRegistrar& Instance() noexcept {
			static CmptRegistrar instance;
			return instance;
		}

		template<typename... Cmpts>
		void Register();

		template<typename... Cmpts>
		bool IsRegistered() const noexcept;

	private:
		template<typename Cmpt>
		void RegisterOne();

		template<typename Cmpt>
		bool IsRegisteredOne() const noexcept;

		std::unordered_set<size_t> registeredCmpts;
	};
}

#include "detail/CmptRegistrar.inl"
