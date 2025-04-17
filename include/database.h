#pragma once

#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <time_manager.h>
#include <waterlevel-data.h>
#include <ESPAsyncWebServer.h>
#include <telnet_logger.h>

enum LogType
{
    LOG,
    ENTRY,
    CRASH_LOG
};

class Database
{
private:
    const char *baseDir = "/waterlevel";
    const char *logDir = "/waterlevel/logs";
    const char *entriesDir = "/waterlevel/entries";
    const char *crashLogDir = "/waterlevel/crash_logs";
    const char *externalSwitchStateFile = "/waterlevel/external_switch_state.bin";

    bool _isSetup = false;

    // Parameters for deletion
    String _lastDeleteDate = "";
    File _dir;
    File _currentFile;
    int _dirIndex = 0;

    String getDirForType(LogType logType)
    {
        switch (logType)
        {
        case LogType::LOG:
            return logDir;
        case LogType::ENTRY:
            return entriesDir;
        case LogType::CRASH_LOG:
            return crashLogDir;
        }
        return "";
    }

    String getFilePath(const String &date, LogType logType)
    {
        return getDirForType(logType) + "/" + date + String(logType == LogType::ENTRY ? ".csv" : ".log");
    }

    // Helper function to check if the file should be deleted (older than 30 days)
    bool shouldDeleteFile(String path, LogType logType)
    {
        String _path = getFilePath(TimeManager::getDateDaysAgoString(30), logType);

        if (path.equals(_path))
        {
            return true;
        }

        int start = path.lastIndexOf("/") + 1;
        int end = path.lastIndexOf(".");

        String date = path.substring(start, end);

        return !TimeManager::isValidDate(date);
    }

    LogType getLogTypeFromString(int type)
    {
        return getLogTypeFromString(String(type));
    }

    LogType getLogTypeFromString(const String &type)
    {
        String typeLower = type;
        typeLower.toLowerCase();

        if (typeLower == "log" || typeLower == "0")
        {
            return LogType::LOG;
        }
        else if (typeLower == "entry" || typeLower == "1")
        {
            return LogType::ENTRY;
        }
        else if (typeLower == "crash_log" || typeLower == "2")
        {
            return LogType::CRASH_LOG;
        }

        return LogType::ENTRY; // Default case
    }

    void moveIndexForDeletion()
    {
        if (_dirIndex < 2)
        {
            _dirIndex++;
        }
        else
        {
            _dirIndex = 0;
            _lastDeleteDate = TimeManager::getDateString();
        }
    }

    String resetReasonToString(esp_reset_reason_t reason)
    {
        switch (reason)
        {
        case ESP_RST_UNKNOWN:
            return "Unknown reset reason";
        case ESP_RST_POWERON:
            return "Power-on reset";
        case ESP_RST_EXT:
            return "External reset";
        case ESP_RST_SW:
            return "Software reset";
        case ESP_RST_PANIC:
            return "Panic reset";
        case ESP_RST_INT_WDT:
            return "Interrupt watchdog reset";
        case ESP_RST_TASK_WDT:
            return "Task watchdog reset";
        case ESP_RST_WDT:
            return "General watchdog reset";
        case ESP_RST_DEEPSLEEP:
            return "Deep sleep wakeup reset";
        case ESP_RST_BROWNOUT:
            return "Brownout reset";
        case ESP_RST_SDIO:
            return "SDIO reset";
        default:
            return "Unknown reset reason (" + String((int)reason) + ")";
        }
    }

    void deleteDirectory(const char *dirname)
    {
        File dir = SD.open(dirname);
        if (!dir)
        {
            log("❌ Directory does not exist: " + String(dirname));
            return;
        }

        if (!dir.isDirectory())
        {
            log("❌ Not a directory: " + String(dirname));
            dir.close();
            return;
        }

        File entry;
        while ((entry = dir.openNextFile()))
        {
            String entryPath = String(dirname) + "/" + entry.name();

            if (entry.isDirectory())
            {
                entry.close();
                deleteDirectory(entryPath.c_str()); // Recursively delete subdirectories
            }
            else
            {
                entry.close();
                if (SD.remove(entryPath))
                {
                    log("🗑️ Deleted file: " + entryPath);
                }
                else
                {
                    log("❌ Failed to delete file: " + entryPath);
                }
            }

            yield(); // Let the watchdog chill
        }

        dir.close();

        if (SD.rmdir(dirname))
        {
            log("✅ Deleted directory: " + String(dirname));
        }
        else
        {
            log("❌ Failed to delete directory (still not empty?): " + String(dirname));
        }
    }

public:
    // Constructor and Destructor
    Database() {}
    ~Database() {}

    // Setup function to initialize SD card
    void setup()
    {
        if (!SD.begin())
        {
            TelnetLogger::log("SD: Card initialization failed!");
            return;
        }

        _isSetup = true;

        TelnetLogger::log(" --- SD: Card initialized --- \n\r Size: " + String(SD.cardSize()) + " bytes, Used: " + String(SD.usedBytes()) + " bytes");

        if (!SD.exists(baseDir))
        {
            if (!SD.mkdir(baseDir))
            {
                TelnetLogger::log("SD: Failed to create base directory!");
                return;
            }
        }

        if (!SD.exists(logDir))
        {
            if (!SD.mkdir(logDir))
            {
                TelnetLogger::log("SD: Failed to create log directory!");
                return;
            }
        }

        if (!SD.exists(entriesDir))
        {
            if (!SD.mkdir(entriesDir))
            {
                TelnetLogger::log("SD: Failed to create entries directory!");
                return;
            }
        }

        if (!SD.exists(crashLogDir))
        {
            if (!SD.mkdir(crashLogDir))
            {
                TelnetLogger::log("SD: Failed to create crash log directory!");
                return;
            }
        }
    }

    void logResetReason()
    {
        if (!_isSetup)
        {
            log("SD: Card not initialized (logResetReason).");
            return;
        }

        if (!TimeManager::isInitialized())
        {
            log("SD: Time not initialized (logResetReason).");
            return;
        }

        String currentDate = TimeManager::getDateString();
        String path = getFilePath(currentDate, LogType::CRASH_LOG);

        // Open the file for appending
        File file = SD.open(path, FILE_APPEND);
        yield();

        if (file)
        {
            String logMessage = "";
            esp_reset_reason_t reason = esp_reset_reason();

            logMessage += "================ Boot ================\n";
            logMessage += "Date time       : " + TimeManager::getDateTimeString() + "\n";
            logMessage += "CPU Frequency   : " + String(ESP.getCpuFreqMHz()) + " MHz\n";
            logMessage += "Sketch Size     : " + String(ESP.getSketchSize()) + " bytes\n";
            logMessage += "Flash Chip Size : " + String(ESP.getFlashChipSize()) + " bytes\n";
            logMessage += "MAC Address     : " + WiFi.macAddress() + "\n";
            logMessage += "Free Heap       : " + String(ESP.getFreeHeap()) + "\n";
            logMessage += "Uptime (ms)     : " + String(millis()) + "\n";
            logMessage += "Reset Reason    : " + resetReasonToString(reason) + "\n";
            logMessage += "======================================\n";

            file.println(logMessage);

            yield();

            file.close();
        }
        else
        {
            log("SD: Error opening file for writing, path: " + path + ", Base exists: " + String(SD.exists(crashLogDir)));
        }
    }

    // Method to log data for today in CSV format
    void saveLevelEntry(WaterLevelData *levelData)
    {
        if (!_isSetup)
        {
            log("SD: Card not initialized (saveLevelEntry).");
            return;
        }

        if (levelData == nullptr)
        {
            log("SD: Null water level data received.");
            return;
        }

        if (!TimeManager::isInitialized())
        {
            log("SD: Time not initialized (saveLevelEntry).");
            return;
        }

        String currentDate = TimeManager::getDateString();
        String path = getFilePath(currentDate, LogType::ENTRY);

        // Open the file for appending
        File file = SD.open(path, FILE_APPEND);
        yield();

        if (file)
        {
            // Write the data in CSV format: timestamp, level, distance, isPumpOn
            file.printf("%s,%d,%.2f,%d\n",
                        TimeManager::getDateTimeString().c_str(),
                        levelData->level,
                        levelData->distance,
                        levelData->isPumpOn ? 1 : 0);

            yield();

            file.close();
        }
        else
        {
            log("SD: Error opening file for writing, path: " + path + ", Base exists: " + String(SD.exists(entriesDir)));
        }
    }

    void saveLogEntry(const String &message, bool newLine = true)
    {
        if (!_isSetup)
        {
            lognd("SD: Card not initialized (saveLogEntry).");
            return;
        }

        if (!TimeManager::isInitialized())
        {
            lognd("SD: Time not initialized (saveLogEntry).");
            return;
        }

        String currentDate = TimeManager::getDateString();
        String path = getFilePath(currentDate, LogType::LOG);

        // Open the file for appending
        File file = SD.open(path, FILE_APPEND);
        yield();

        if (file)
        {
            if (newLine)
            {
                file.printf("\n[%s] %s", TimeManager::getDateTimeString().c_str(), message.c_str());
            }
            else
            {
                file.print(message.c_str());
            }

            yield();

            file.close();
        }
        else
        {
            lognd("SD: Error opening file for writing, path: " + path + ", Base exists: " + String(SD.exists(logDir)));
        }
    }

    void streamFile(AsyncWebServerRequest *request)
    {
        if (!_isSetup)
        {
            log("SD: Card not initialized (streamFile).");
            request->send(500, "text/plain", "SD card not initialized");
            return;
        }

        LogType logType = LogType::ENTRY;

        String date = TimeManager::getDateString();
        String _file = "";

        if (request->hasParam("type", false))
        {
            AsyncWebParameter *p = request->getParam("type", false);

            logType = getLogTypeFromString(p->value());
        }

        if (request->hasParam("date", false))
        {
            AsyncWebParameter *p = request->getParam("date", false);
            date = p->value();
        }

        if (request->hasParam("file", false))
        {
            AsyncWebParameter *p = request->getParam("file", false);
            _file = p->value();
        }

        if (!TimeManager::isValidDate(date))
        {
            request->send(400, "text/plain", "Invalid date format");
            return;
        }

        if (_file.length() > 0)
        {
            date = _file;
        }

        String path = getFilePath(date, logType);

        if (!SD.exists(path))
        {
            request->send(404, "text/plain", "File not found: " + path);
            return;
        }

        yield();

        AsyncWebServerResponse *response = request->beginResponse(
            SD,
            path,
            logType == LogType::ENTRY ? "text/csv" : "text/plain");

        yield();

        String _path = getFilePath(TimeManager::getDateString(), logType);

        if (_path.equals(path))
        {
            // today’s file → always re‑validate
            response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
            response->addHeader("Pragma", "no-cache");
            response->addHeader("Expires", "0");
        }
        else
        {
            // file is older than today → cache for 1 day
            response->addHeader("Cache-Control", "public, max-age=86400");
        }

        yield();

        request->send(response);
    }

    void listFiles(AsyncWebServerRequest *request)
    {
        if (!_isSetup)
        {
            log("SD: Card not initialized (listFiles).");
            request->send(500, "text/plain", "SD card not initialized");
            return;
        }

        LogType logType = LogType::ENTRY;

        if (request->hasParam("type", false))
        {
            AsyncWebParameter *p = request->getParam("type", false);

            logType = getLogTypeFromString(p->value());
        }

        File dir = SD.open(getDirForType(logType));

        yield(); // Yield to allow other tasks to run

        if (!dir)
        {
            request->send(500, "text/plain", "Failed to open directory");
            return;
        }

        String fileNames;
        int fileCount = 0;
        const int maxFiles = 100;

        AsyncResponseStream *response = request->beginResponseStream("text/plain");

        while (fileCount < maxFiles)
        {
            File entry = dir.openNextFile();
            yield(); // Yield to allow other tasks to run

            if (!entry)
            {
                break; // No more files
            }

            response->println(entry.name());
            yield();

            entry.close();
            yield(); // Yield to allow other tasks to run

            fileCount++;
        }

        dir.close();
        yield(); // Yield to allow other tasks to run

        request->send(response);
    }

    void getSDInfo(AsyncWebServerRequest *request)
    {
        if (!_isSetup)
        {
            log("SD: Card not initialized (getSDInfo).");
            request->send(500, "text/plain", "SD card not initialized");
            return;
        }

        double bytesInMb = 1024.0 * 1024.0;

        double total = SD.cardSize() / bytesInMb;
        double used = SD.usedBytes() / bytesInMb;
        double free = total - used;

        // Buffer for formatted output
        char buffer[200];

        sprintf(buffer,
                "{\"total\":%.2f,\"used\":%.2f,\"free\": %.2f}",
                total,
                used,
                free);

        request->send(200, "application/json", buffer);
    }

    // Method to delete files older than a month
    void deleteOldFiles()
    {
        if (!_isSetup)
        {
            log("SD: Card not initialized (deleteOldFiles).");
            return;
        }

        if (TimeManager::getDateString().equals(_lastDeleteDate))
        {
            // log("SD: Already deleted files today.");
            return;
        }

        LogType logType = getLogTypeFromString(_dirIndex);

        // If we haven't opened the directory yet, do it now
        if (!_dir)
        {
            _dir = SD.open(getDirForType(logType));

            yield(); // Yield to allow other tasks to run

            if (!_dir)
            {
                log("SD: Failed to open directory for deletion for type: " + String((int)logType));

                _dir = File();
                _currentFile = File();

                moveIndexForDeletion();

                return;
            }
            else
            {
                log("SD: Opened directory for deletion for type: " + String((int)logType));
            }
        }

        // If we haven't opened a file yet, grab the next one
        if (!_currentFile)
        {
            _currentFile = _dir.openNextFile();
            yield(); // Yield to allow other tasks to run

            if (!_currentFile)
            {
                // All files have been processed
                _dir.close();
                yield(); // Yield to allow other tasks to run

                _dir = File();
                _currentFile = File();

                log("SD: Finished processing files in directory for type: " + String((int)logType));

                moveIndexForDeletion();

                return;
            }
            else
            {
                log("SD: Opened file for deletion: " + String(_currentFile.path()));
            }
        }

        // Process the current file
        String filePath = _currentFile.path();

        if (shouldDeleteFile(filePath, logType))
        {
            log("SD: Deleting old file: " + filePath);

            if (!SD.remove(filePath))
            {
                log("SD: Failed to delete file: " + filePath);
            }
            else
            {
                yield(); // Yield to allow other tasks to run
            }
        }
        else
        {
            log("SD: File is not old enough to delete: " + filePath);
        }

        _currentFile.close();
        _currentFile = File(); // Move to the next file on the next loop

        yield(); // Yield to allow other tasks to run
    }

    void deleteRoot(LogType logType)
    {
        deleteDirectory(getDirForType(logType).c_str());
    }

    void saveExternalSwitchState(int state)
    {
        if (!_isSetup)
            return;

        File file = SD.open(externalSwitchStateFile, FILE_WRITE);
        yield();

        if (file)
        {
            file.write((uint8_t *)&state, sizeof(state));
            yield();

            file.close();

            log("SD: External switch state saved: " + String(state));
        }
        else
        {
            log("SD: Failed to open external switch state file for writing");
        }
    }

    int loadExternalSwitchState()
    {
        if (!_isSetup)
            return -1;

        File file = SD.open(externalSwitchStateFile, FILE_READ);
        yield();

        if (file && file.size() == sizeof(int))
        {
            int state = 0;

            file.read((uint8_t *)&state, sizeof(state));
            yield();

            file.close();

            log("SD: External switch state loaded: " + String(state));

            return state;
        }
        else
        {
            if (file)
            {
                file.close();
            }

            log("SD: Failed to open external switch state file for reading or file size is incorrect");
        }

        return -1; // or a default value
    }

    void log(const String &message, bool newLine = true, bool noDatabase = false)
    {
        TelnetLogger::log(message, newLine);

        if (!noDatabase)
            saveLogEntry(message, newLine);
    }

    void lognd(const String &message, bool newLine = true)
    {
        log(message, newLine, true);
    }
};
