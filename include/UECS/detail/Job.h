#pragma once

#include "_deps/taskflow/taskflow.hpp"

namespace Ubpa {
	using Job = tf::Taskflow;
	using JobHandle = tf::Task;

	using JobExecutor = tf::Executor;
}
