﻿#pragma once


#include <Logger/Logger.h>

#include <File/TextFile.h>

#include <ctime>
#include <filesystem>
#include <string>
#include <cstdarg>


namespace fs
{
	inline Logger::Logger()
		: _basePathOffset{ 0 }
	{
		__noop;
	}

	inline Logger::~Logger()
	{
		std::lock_guard<std::mutex> scopeLock{ _mutex };
		if (_outputFileName.empty() == false)
		{
			fs::TextFileWriter textFileWriter;
			textFileWriter.write(_history.c_str());
			textFileWriter.save(_outputFileName.c_str());
		}
	}

	FS_INLINE void Logger::setOutputFileName(const char* const fileName)
	{
		Logger& logger = getInstance();
		{
			std::lock_guard<std::mutex> scopeLock{ logger._mutex };

			logger._outputFileName = fileName;
		}
	}

	FS_INLINE void Logger::log(const char* const logTag, const char* const author, const char* const functionName, const char* const fileName, const uint32 lineNumber, const char* const format, ...)
	{
		static char finalBuffer[kFinalBufferSize]{};
		static char content[kFinalBufferSize]{};
		
		// variadic arguments
		{
			va_list vl;
			va_start(vl, format);
			vsprintf_s(content, kFinalBufferSize, format, vl);
			va_end(vl);
		}

		logInternal(logTag, author, content, functionName, fileName, lineNumber, finalBuffer);

		printf(finalBuffer);
	}

	FS_INLINE void Logger::logError(const char* const logTag, const char* const author, const char* const functionName, const char* const fileName, const uint32 lineNumber, const char* const format, ...)
	{
		static constexpr int32 kErrorExitCode = -1;
		static char finalBuffer[kFinalBufferSize]{};
		static char content[kFinalBufferSize]{};

		// variadic arguments
		{
			va_list vl;
			va_start(vl, format);
			vsprintf_s(content, kFinalBufferSize, format, vl);
			va_end(vl);
		}

		logInternal(logTag, author, content, functionName, fileName, lineNumber, finalBuffer);

		printf(finalBuffer);
		::MessageBoxA(nullptr, finalBuffer, "LOG ERROR", MB_ICONERROR);
		
#if defined FS_DEBUG
		DebugBreak();
#else
		exit(kErrorExitCode);
#endif
	}

	FS_INLINE void Logger::logInternal(const char* const logTag, const char* const author, const char* const content, const char* const functionName, const char* const fileName, const uint32 lineNumber, char (&outBuffer)[kFinalBufferSize])
	{
		static char timeBuffer[kTimeBufferSize]{};
		Logger& logger = getInstance();
		{
			std::lock_guard<std::mutex> scopeLock{ logger._mutex };

			const time_t now = time(nullptr);
			tm localNow;
			localtime_s(&localNow, &now);
			strftime(timeBuffer, kTimeBufferSize, "%Y-%m-%d-%H:%M:%S", &localNow);

			sprintf_s(outBuffer, kFinalBufferSize, "[%s] %s [%s] %s : %s[%d] %s()\n", logTag, timeBuffer, author, content, fileName + logger._basePathOffset, lineNumber, functionName);

			OutputDebugStringA(outBuffer);

			logger._history.append(outBuffer);
		}
	}

	FS_INLINE Logger& Logger::getInstance() noexcept
	{
		static bool isFirstTime = true;
		static Logger logger = Logger();
		if (true == isFirstTime)
		{
			isFirstTime = false;

			std::filesystem::path currentPath = std::filesystem::current_path();
			if (currentPath.has_parent_path())
			{
				currentPath = currentPath.parent_path();
			}
			if (currentPath.has_parent_path())
			{
				currentPath = currentPath.parent_path();
			}
			logger._basePathOffset = static_cast<uint32>(currentPath.string().length());
		}
		return logger;
	}
}
