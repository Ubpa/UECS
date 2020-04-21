#pragma once

namespace Ubpa {
	namespace CmptTag {
		// LastFrame -> Write -> Newest

		template<typename Cmpt>
		class LastFrame {
		public:
			LastFrame(const Cmpt* cmpt) : cmpt{ cmpt } {}
			Cmpt* get() const noexcept { return cmpt; }
			operator Cmpt* () const noexcept { return cmpt; }
			Cmpt* operator->() const noexcept { return cmpt; }
		private:
			const Cmpt* cmpt;
		};

		template<typename Cmpt>
		using Write = Cmpt*;

		template<typename Cmpt>
		using Newest = const Cmpt*;

		template<typename TagedCmpt>
		struct RemoveTag;
		template<typename TagedCmpt>
		using RemoveTag_t = typename RemoveTag<TagedCmpt>::type;

		template<typename TagedCmpt>
		struct IsLastFrame;
		template<typename TagedCmpt>
		static constexpr bool IsLastFrame_v = IsLastFrame<TagedCmpt>::value;

		template<typename TagedCmpt>
		struct IsWrite;
		template<typename TagedCmpt>
		static constexpr bool IsWrite_v = IsWrite<TagedCmpt>::value;

		template<typename TagedCmpt>
		struct IsNewest;
		template<typename TagedCmpt>
		static constexpr bool IsNewest_v = IsNewest<TagedCmpt>::value;
	}
}

#include "detail/CmptTag.inl"
