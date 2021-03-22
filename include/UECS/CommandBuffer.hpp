#pragma once

#include <functional>
#include <vector>

namespace Ubpa::UECS {
	class CommandBuffer {
	public:
		void AddCommand(std::function<void()> command) { commands.push_back(std::move(command)); }
		void AddCommandBuffer(CommandBuffer cb) {
			commands.reserve(commands.size() + cb.commands.size());
			for (auto& cmd : cb.commands)
				commands.push_back(std::move(cmd));
		}
		bool Empty() const noexcept { return commands.empty(); }
		void Clear() { commands.clear(); }

		auto& GetCommands() noexcept { return commands; }
		const auto& GetCommands() const noexcept { return commands; }
	private:
		std::vector<std::function<void()>> commands;
	};

	class CommandBufferView {
	public:
		CommandBufferView(CommandBuffer* cb = nullptr) : commandBuffer{ cb } {}
		CommandBuffer* GetCommandBuffer() const noexcept { return commandBuffer; }
		CommandBuffer* operator->()const noexcept { return commandBuffer; }
	private:
		CommandBuffer* commandBuffer;
	};
}
