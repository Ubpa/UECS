#pragma once

#include "RuntimeCmptTraits.h"

#include <UTemplate/Concept.h>

namespace Ubpa::detail::CmptRegistrar_ {
	template<typename T>
	Concept(HaveOnRegister, &T::OnRegister);
}

namespace Ubpa {
	template<typename... Cmpts>
	void CmptRegistrar::Register() {
		(RegisterOne<Cmpts>(), ...);
	}

	template<typename... Cmpts>
	bool CmptRegistrar::IsRegistered() const noexcept {
		return (IsRegisteredOne<Cmpts>() && ...);
	}

	template<typename Cmpt>
	bool CmptRegistrar::IsRegisteredOne() const noexcept {
		return registeredCmpts.find(TypeID<Cmpt>) != registeredCmpts.end();
	}

	template<typename Cmpt>
	void CmptRegistrar::RegisterOne() {
		if (IsRegistered<Cmpt>())
			return;

		if constexpr (Require<detail::CmptRegistrar_::HaveOnRegister, Cmpt>)
			Cmpt::OnRegister();

		RuntimeCmptTraits::Instance().Register<Cmpt>();
		
		registeredCmpts.insert(TypeID<Cmpt>);
	}
}
