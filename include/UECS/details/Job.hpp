#pragma once

#include <_deps/taskflow/taskflow.hpp>

namespace Ubpa::UECS {
	using Job = tf::Taskflow;
	using JobHandle = tf::Task;

	using JobExecutor = tf::Executor;
}
