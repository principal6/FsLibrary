#pragma once


#ifndef FS_SCOPED_CPU_PROFILER_H
#define FS_SCOPED_CPU_PROFILER_H


#include <CommonDefinitions.h>


namespace fs
{
	namespace Profiler
	{
		class ScopedCpuProfiler
		{
		public:
			struct Log
			{
			public:
								Log() = default;
								Log(const std::string& content, const uint64 startTimepointMs, const uint64 durationMs);
								~Log() = default;

			public:
				std::string		_content;
				uint64			_startTimepointMs;
				uint64			_durationMs;
			};

		private:
			class ScopedCpuProfilerLogger
			{
				

			private:
												ScopedCpuProfilerLogger() = default;
												~ScopedCpuProfilerLogger() = default;

			public:
				static ScopedCpuProfilerLogger&	getInstance() noexcept;
				static const uint64				getCurrentTimeMs() noexcept;

			public:
				void							log(const ScopedCpuProfiler& profiler, const uint64 durationMs) noexcept;
				const std::vector<Log>&			getLogArray() const noexcept;

			private:
				std::vector<Log>				_logArray;
			};

		public:
												ScopedCpuProfiler(const std::string& content);
												~ScopedCpuProfiler();

		public:
			static const std::vector<Log>&		getEntireLogArray() noexcept;

		private:
			uint64								_startTimepointMs;
			std::string							_content;
		};
	}
}


#endif // !FS_SCOPED_CPU_PROFILER_H
