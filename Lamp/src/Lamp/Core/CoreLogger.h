#pragma once

#include <spdlog/fmt/fmt.h>

namespace Lamp
{
	enum class LogLevel
	{
		Trace,
		Info,
		Warn,
		Error,
		Critical
	};

	struct Message
	{
		Message(const std::string& msg, LogLevel level)
			: Msg(msg), Level(level)
		{}
		std::string Msg;
		LogLevel Level;
	};

	class CoreLogger
	{
	public:
		using string_view_t = fmt::basic_string_view<char>;
		using memory_buf_t = fmt::basic_memory_buffer<char, 250>;

		template<typename... Args>
		void trace(string_view_t fmt, const Args&... args)
		{
			Log("[Trace]", LogLevel::Trace, fmt, args...);
		}

		template<typename... Args>
		void info(string_view_t fmt, const Args&... args)
		{
			Log("[Info]", LogLevel::Info, fmt, args...);
		}

		template<typename... Args>
		void warn(string_view_t fmt, const Args&... args)
		{
			Log("[Warning]", LogLevel::Warn, fmt, args...);
		}

		template<typename... Args>
		void error(string_view_t fmt, const Args&... args)
		{
			Log("[Error]", LogLevel::Error, fmt, args...);
		}

		template<typename... Args>
		void critical(string_view_t fmt, const Args&... args)
		{
			Log("[Critical]", LogLevel::Critical, fmt, args...);
		}

		template<typename... Args>
		void Log(const std::string& prefix, LogLevel level, string_view_t fmt, const Args&... args)
		{
			memory_buf_t buf;
			fmt::format_to(buf, fmt, args...);
			std::string d = std::string(buf.data());

			auto v = d.find_first_of("Ì");
			d = d.substr(0, v);

			if (m_Messages.size() > 1000)
			{
				m_Messages.clear();
			}

			m_Messages.emplace_back(prefix + " " + d, level);
		}

		inline std::vector<Message>& GetMessages() { return m_Messages; }

	private:
		std::vector<Message> m_Messages;
	};
}