#include <UECS/CommandBuffer.hpp>

using namespace Ubpa::UECS;

void CommandBuffer::AddCommand(std::function<void()> command) { commands.push_back(std::move(command)); }

void CommandBuffer::AddCommandBuffer(CommandBuffer cb) {
	commands.reserve(commands.size() + cb.commands.size());
	for (auto& cmd : cb.commands)
		commands.push_back(std::move(cmd));
}

bool CommandBuffer::Empty() const noexcept { return commands.empty(); }

void CommandBuffer::Clear() noexcept { commands.clear(); }

std::span<std::function<void()>>  CommandBuffer::GetCommands() noexcept { return commands; }

std::span<const std::function<void()>>  CommandBuffer::GetCommands() const noexcept {
	return const_cast<CommandBuffer*>(this)->GetCommands();
}

void CommandBuffer::Run() {
	for (const auto& cmd : commands)
		cmd();
	commands.clear();
}

CommandBufferPtr::CommandBufferPtr(CommandBuffer * cb) : commandBuffer{cb} {}
CommandBuffer* CommandBufferPtr::Get() const noexcept { return commandBuffer; }
CommandBuffer* CommandBufferPtr::operator->()const noexcept { return commandBuffer; }
