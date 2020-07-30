#pragma once

#include <UTemplate/Typelist.h>

namespace Ubpa::UECS {

	// LastFrame -> Write -> Latest

	enum class Mode {
		LAST_FRAME,
		WRITE,
		LATEST
	};

	template<typename Cmpt>
	class LastFrame {
	public:
		LastFrame(const Cmpt* cmpt) noexcept : cmpt{ cmpt } {}
		const Cmpt* Get() const noexcept { return cmpt; }
		operator const Cmpt* () const noexcept { return cmpt; }
		const Cmpt* operator->() const noexcept { return cmpt; }
	private:
		const Cmpt* cmpt;
	};

	template<typename Cmpt>
	using Write = Cmpt*;

	template<typename Cmpt>
	using Latest = const Cmpt*;

	// <Cmpt>
	template<typename TaggedCmpt>
	struct RemoveTag;
	template<typename TaggedCmpt>
	using RemoveTag_t = typename RemoveTag<TaggedCmpt>::type;

	// <Cmpt>*
	template<typename TaggedCmpt>
	struct DecayTag;
	template<typename TaggedCmpt>
	using DecayTag_t = typename DecayTag<TaggedCmpt>::type;

	template<typename TaggedCmpt>
	struct IsLastFrame;
	template<typename TaggedCmpt>
	static constexpr bool IsLastFrame_v = IsLastFrame<TaggedCmpt>::value;

	template<typename TaggedCmpt>
	struct IsWrite;
	template<typename TaggedCmpt>
	static constexpr bool IsWrite_v = IsWrite<TaggedCmpt>::value;

	template<typename TaggedCmpt>
	struct IsLatest;
	template<typename TaggedCmpt>
	static constexpr bool IsLatest_v = IsLatest<TaggedCmpt>::value;

	template<typename T>
	struct IsTaggedCmpt : IValue<bool, IsLastFrame_v<T> || IsWrite_v<T> || IsLatest_v<T>> {};
	template<typename T>
	static constexpr bool IsTaggedCmpt_v = IsTaggedCmpt<T>::value;
}

#include "detail/CmptTag.inl"
