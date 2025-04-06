#pragma once

#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
// #include <logger.h>
#include <time_manager.h>
#include <waterlevel.h>
#include <ESPAsyncWebServer.h>

class Database
{
private:
    const char *baseDir = "/waterlevel";
    const char *logDir = "/waterlevel/logs";
    const char *entriesDir = "/waterlevel/entries";

    // Parameters for deletion
    String _lastDeleteDate = "";
    File _dir;
    File _currentFile;
    int _dirIndex = 0;

    String getFileName(String date, bool isEntry = true)
    {
        return String(isEntry ? entriesDir : logDir) + "/" + date + ".csv";
    }

    // Helper function to check if the file should be deleted (older than 30 days)
    bool shouldDeleteFile(String path, bool isEntry)
    {
        String _path = getFileName(TimeManager::getDateDaysAgoString(30), isEntry);

        return path.equals(_path);
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
            LOGL("SD: Card initialization failed!");
            return;
        }

        LOGL(" --- SD: Card initialized --- \n\r Size: " + String(SD.cardSize()) + " bytes, Used: " + String(SD.usedBytes()) + " bytes");

        if (!SD.exists(baseDir))
        {
            if (!SD.mkdir(baseDir))
            {
                LOGL("SD: Failed to create base directory!");
                return;
            }
        }

        if (!SD.exists(logDir))
        {
            if (!SD.mkdir(logDir))
            {
                LOGL("SD: Failed to create log directory!");
                return;
            }
        }

        if (!SD.exists(entriesDir))
        {
            if (!SD.mkdir(entriesDir))
            {
                LOGL("SD: Failed to create entries directory!");
                return;
            }
        }
    }

    // Method to log data for today in CSV format
    void saveLevelEntry(WaterLevelData *levelData)
    {
        if (levelData == nullptr)
        {
            LOGL("SD: Null water level data received.");
            return;
        }

        String currentDate = TimeManager::getDateString();
        String filename = getFileName(currentDate);

        // Open the file for appending
        File file = SD.open(filename, FILE_APPEND);

        if (file)
        {
            // Write the data in CSV format: timestamp, level, distance, isPumpOn
            file.printf("%s,%d,%.2f,%d\n",
                        TimeManager::getDateTimeString().c_str(),
                        levelData->level,
                        levelData->distance,
                        levelData->isPumpOn ? 1 : 0);
            file.close();
        }
        else
        {
            LOGL("SD: Error opening file for writing, filename: " + filename + ", Base exists: " + String(SD.exists(entriesDir)));
        }
    }

    void saveLogEntry(String message)
    {
        String currentDate = TimeManager::getDateString();
        String filename = getFileName(currentDate, false);

        // Open the file for appending
        File file = SD.open(filename, FILE_APPEND);

        if (file)
        {
            file.print(TimeManager::getDateTimeString());
            file.print(",");
            file.print(message);
        }
        else
        {
            LOGL("SD: Error opening file for writing");
        }
    }

    void streamFile(AsyncWebServerRequest *request)
    {
        bool isEntry = true;
        String date = TimeManager::getDateString();

        if (request->hasParam("loggs", false))
        {
            AsyncWebParameter *p = request->getParam("loggs", false);
            isEntry = p->value() != "true";
        }

        if (request->hasParam("date", false))
        {
            AsyncWebParameter *p = request->getParam("date", false);
            date = p->value();
        }

        if (!TimeManager::isValidDate(date))
        {
            request->send(400, "text/plain", "Invalid date format");
            return;
        }

        String filename = getFileName(date, isEntry);

        if (!SD.exists(filename))
        {
            request->send(404, "text/plain", "File not found");
            return;
        }

        String _filename = getFileName(TimeManager::getDateString(), isEntry);

        AsyncWebServerResponse *resp = request->beginResponse(
            SD,
            filename,
            "text/csv");

        if (_filename.equals(filename))
        {
            // today’s file → always re‑validate
            resp->addHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");

            resp->addHeader("Pragma", "no-cache");
            resp->addHeader("Expires", "0");
        }
        else
        {
            // file is older than today → cache for 1 day
            resp->addHeader("Cache-Control", "public, max-age=86400");
        }

        request->send(resp);
    }

    void listFiles(AsyncWebServerRequest *request)
    {
        String fileList;
        bool isEntry = true;

        if (request->hasParam("loggs", false))
        {
            AsyncWebParameter *p = request->getParam("loggs", false);
            isEntry = p->value() != "true";
        }

        File dir = SD.open(isEntry ? entriesDir : logDir);

        if (!dir)
        {
            request->send(500, "text/plain", "Failed to open directory");
            return;
        }

        String fileNames;

        while (true)
        {
            File entry = dir.openNextFile();

            if (!entry)
            {
                break; // No more files
            }

            fileNames += String(entry.name()) + "\n";

            entry.close();
        }

        dir.close();

        request->send(200, "text/plain", fileNames.c_str());
    }

    // Method to delete files older than a month
    void deleteOldFiles()
    {
        if (TimeManager::getDateString().equals(_lastDeleteDate))
        {
            return;
        }

        bool isEntry = _dirIndex == 0;

        // If we haven't opened the directory yet, do it now
        if (!_dir)
        {
            _dir = SD.open(isEntry ? entriesDir : logDir);
            if (!_dir)
            {
                LOGL("SD: Failed to open directory for deletion");

                if (isEntry)
                {
                    _dirIndex++;
                }
                else
                {
                    _dirIndex = 0;
                    _lastDeleteDate = TimeManager::getDateString();
                }

                return;
            }
            else
            {
                LOGL("SD: Opened directory for deletion");
            }
        }

        // If we haven't opened a file yet, grab the next one
        if (!_currentFile)
        {
            _currentFile = _dir.openNextFile();
            if (!_currentFile)
            {
                // All files have been processed
                _dir.close();
                _currentFile = File();

                if (isEntry)
                {
                    _dirIndex++;
                }
                else
                {
                    _dirIndex = 0;
                    _lastDeleteDate = TimeManager::getDateString();
                }
                return;
            }
        }

        // Process the current file
        String filePath = _currentFile.path();

        if (shouldDeleteFile(filePath, isEntry))
        {
            LOGL("SD: Deleting old file: " + filePath);
            if (!SD.remove(filePath))
            {
                LOGL("SD: Failed to delete file: " + filePath);
            }
        }

        _currentFile.close();
        _currentFile = File(); // Move to the next file on the next loop

        if (isEntry)
        {
            _dirIndex++;
        }
        else
        {
            _dirIndex = 0;
            _lastDeleteDate = TimeManager::getDateString();
        }
    }
};
