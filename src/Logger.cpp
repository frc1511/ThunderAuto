#include <ThunderAuto/Logger.hpp>
#include <ThunderAuto/Error.hpp>
#include <ThunderAuto/Platform/Platform.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <mutex>

static const std::string kThunderAutoLoggerName = "ThunderAuto";

static std::shared_ptr<spdlog::logger> s_thunderAutoLogger;
static std::mutex s_loggerMutex;

static std::filesystem::path InitLogger(const std::filesystem::path& logsDir) {
  std::vector<spdlog::sink_ptr> sinks;

  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

  std::filesystem::path logFile;
  if (!logsDir.empty()) {
    logFile = MakeLogFilePath(logsDir, "ThunderAuto");
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile.string(), true));
    CleanupLogsDirectory(logsDir, 10);
  }

  s_thunderAutoLogger = std::make_shared<spdlog::logger>(kThunderAutoLoggerName, sinks.begin(), sinks.end());
  ThunderLibCoreLogger::make(sinks.begin(), sinks.end());  // ThunderLibCoreLogger gets the same sinks

  return logFile;
}

spdlog::logger* ThunderAutoLogger::get() {
  std::lock_guard<std::mutex> lock(s_loggerMutex);
  if (!s_thunderAutoLogger) {
    std::filesystem::path logsDir, appDataDir = getPlatform().getAppDataDirectory();
    if (appDataDir.empty()) {
      logsDir = std::filesystem::current_path() / "logs";
    }
    else {
      logsDir = appDataDir / "logs";
    }
    std::filesystem::path logFile = InitLogger(logsDir);
    s_thunderAutoLogger->info("Logging to file: {}", logFile.string());
  }

  return s_thunderAutoLogger.get();
}
