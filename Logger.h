#pragma once

#include <iostream>
#include <string>

namespace MelonRenderer
{
	class Logger
	{
	public:
		// singleton
		static Logger& Get();
		Logger(Logger const&) = delete;
		void operator=(Logger const&) = delete;

		static void Log(std::string input);
		void Print();
		void Clear();
		// TODO: ToFile(std::string path);

		std::string m_log;
		
	private:
		Logger();
		~Logger();
	};
}
