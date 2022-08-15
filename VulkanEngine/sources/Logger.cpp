#include "Logger.h"
#include <fstream>
#include <iostream>
#include <chrono>

namespace sge {
	static std::ofstream out_stream;

	std::stringstream& Logger::log() noexcept
	{
		return m_ss;
	}

	void Logger::flush_error() noexcept
	{
		const time_t current_time = time(0);
		const std::tm* now = std::localtime(&current_time);
		char buf[35];
		strftime(buf, sizeof(buf), "%F %T Error: ", now);
		std::cerr  << buf << m_ss.str() << std::endl;
		out_stream << buf << m_ss.rdbuf() << std::endl;
		m_ss.str("");
	}
	void Logger::flush_info() noexcept
	{
		const time_t current_time = time(0);
		const std::tm* now = std::localtime(&current_time);
		char buf[35];
		strftime(buf, sizeof(buf), "%F %T Info: ", now);
		std::cout << buf << m_ss.str() << "\n";
		out_stream << buf << m_ss.rdbuf() << "\n";
		m_ss.str("");
	}
	void Logger::flush() const noexcept
	{
		std::cout << std::flush;
		out_stream << std::flush;
	}
	Logger::~Logger()
	{
		out_stream.close();
	}

	Logger::Logger()
	{
		out_stream.open("Log.txt");
		if (!out_stream.is_open())
			std::cerr << "Failed to open log file!" << std::endl;
	}

	/*static*/ Logger& Logger::Instance() noexcept
	{
		static Logger log;
		return log;
	}

}