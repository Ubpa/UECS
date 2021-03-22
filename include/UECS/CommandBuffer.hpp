#pragma once

#include <functional>
#include <vector>
#include <map>

namespace Ubpa::UECS {
	class CommandBuffer {
	public:
		void AddCommand(std::function<void()> command, int layer) { lcommands[layer].push_back(std::move(command)); }
		void AddCommandBuffer(CommandBuffer cb) {
			for (auto& [layer, cmds] : cb.lcommands) {
				auto& dst = lcommands[layer];
				dst.reserve(dst.size() + cmds.size());
				for (auto& cmd : cmds)
					dst.push_back(std::move(cmd));
			}
		}
		bool Empty() const noexcept {
			for (const auto& [layer, cmds] : lcommands) {
				if (!cmds.empty())
					return false;
			}
			return true;
		}
		void Clear() { lcommands.clear(); }

		auto& GetCommands() noexcept { return lcommands; }
		const auto& GetCommands() const noexcept { return lcommands; }
	private:
		std::map<int, std::vector<std::function<void()>>> lcommands;
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
