#pragma once

namespace Ubpa::UECS {
	template<typename T, typename... Args>
	T* Schedule::CreateFrameObject(Args&&... args) const {
		void* buffer = frame_rsrc.allocate(sizeof(T), alignof(T));
		return new(buffer)T(std::forward<Args>(args)...);
	}

	template<typename Func>
	const SystemFunc* Schedule::RegisterEntityJob(
		Func&& func,
		std::string_view name,
		bool isParallel,
		ArchetypeFilter filter,
		CmptLocator cmptLocator,
		SingletonLocator singletonLocator,
		RandomAccessor randomAccessor,
		ChangeFilter changeFilter,
		int layer
	) {
		return Request(
			layer,
			std::forward<Func>(func),
			RegisterFrameString(name),
			std::move(filter),
			std::move(cmptLocator),
			std::move(singletonLocator),
			std::move(randomAccessor),
			std::move(changeFilter),
			isParallel
		);
	}

	template<typename Func>
	const SystemFunc* Schedule::RegisterChunkJob(
		Func&& func,
		std::string_view name,
		ArchetypeFilter filter,
		bool isParallel,
		SingletonLocator singletonLocator,
		RandomAccessor randomAccessor,
		ChangeFilter changeFilter,
		int layer
	) {
		return Request(
			layer,
			std::forward<Func>(func),
			RegisterFrameString(name),
			std::move(filter),
			std::move(singletonLocator),
			std::move(randomAccessor),
			std::move(changeFilter),
			isParallel
		);
	}

	template<typename Func>
	const SystemFunc* Schedule::RegisterJob(
		Func&& func,
		std::string_view name,
		SingletonLocator singletonLocator,
		RandomAccessor randomAccessor,
		int layer
	) {
		return Request(
			layer,
			std::forward<Func>(func),
			RegisterFrameString(name),
			std::move(singletonLocator),
			std::move(randomAccessor)
		);
	}

	template<typename Func>
	const SystemFunc* Schedule::RegisterEntityJob(
		Func&& func,
		std::string_view name,
		EntityJobConfig config
	) {
		return RegisterEntityJob(
			std::forward<Func>(func),
			RegisterFrameString(name),
			config.isParallel,
			std::move(config.archetypeFilter),
			std::move(config.cmptLocator),
			std::move(config.singletonLocator),
			std::move(config.randomAccessor),
			std::move(config.changeFilter),
			config.layer
		);
	}

	template<typename Func>
	const SystemFunc* Schedule::RegisterChunkJob(
		Func&& func,
		std::string_view name,
		ChunkJobConfig config
	) {
		return RegisterChunkJob(
			std::forward<Func>(func),
			RegisterFrameString(name),
			config.isParallel,
			std::move(config.archetypeFilter),
			std::move(config.singletonLocator),
			std::move(config.randomAccessor),
			std::move(config.changeFilter),
			config.layer
		);
	}

	template<typename... Args>
	const SystemFunc* Schedule::Request(int layer, Args&&... args) {
		SystemFunc* sysFunc = (SystemFunc*)frame_rsrc.allocate(sizeof(SystemFunc), alignof(SystemFunc));
		new(sysFunc)SystemFunc(std::forward<Args>(args)...);
		layerInfos[layer].sysFuncs.emplace(sysFunc->GetValue(), sysFunc);
		return sysFunc;
	}
}
