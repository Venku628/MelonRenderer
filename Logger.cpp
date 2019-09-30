#include "Logger.h"

namespace MelonRenderer
{

	Logger& MelonRenderer::Logger::Get()
	{
		static Logger instance;
		return instance;
	}

	void MelonRenderer::Logger::Log(std::string input)
	{
		input.append("\n");
		Get().m_log.append(input);
	}

	void MelonRenderer::Logger::Print()
	{
		std::cout << m_log;
		Clear();
	}

	void Logger::Clear()
	{
		m_log = "";
	}

	Logger::Logger()
	{

	}

	Logger::~Logger()
	{
		Print();
	}

}