#pragma once

#include <functional>
#include <vector>
#include <span>

namespace Ubpa::UECS {
	class CommandBuffer {
	public:
		void AddCommand(std::function<void()> command);
		void AddCommandBuffer(CommandBuffer cb);
		bool Empty() const noexcept;
		void Clear() noexcept;

		std::span<std::function<void()>> GetCommands() noexcept;
		std::span<const std::function<void()>> GetCommands() const noexcept;

		void Run();
	private:
		std::vector<std::function<void()>> commands;
	};

	class CommandBufferPtr {
	public:
		CommandBufferPtr(CommandBuffer* cb = nullptr);
		CommandBuffer* Get() const noexcept;
		CommandBuffer* operator->()const noexcept;
	private:
		CommandBuffer* commandBuffer;
	};
}
