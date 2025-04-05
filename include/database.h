#pragma once

#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <logger.h>
#include <time_manager.h>
#include <waterlevel.h>
#include <ESPAsyncWebServer.h>

class Database
{
private:
    const char *logDir = "/sd/logs";
    const char *entriesDir = "/sd/entries";

    // Parameters for deletion
    String _lastDeleteDate = "";
    File _dir;
    File _currentFile;

    String getFileName(String date, bool isEntry = true)
    {
        return String(isEntry ? entriesDir : logDir) + "/" + date + ".json";
    }

    // Helper function to check if the file should be deleted (older than 30 days)
    bool shouldDeleteFile(String fileDate)
    {
        return TimeManager::getDateDaysAgoString(30) > fileDate; // Simple check for files older than today
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
            LOGL("SD card initialization failed!");
            return;
        }

        LOGL("SD card initialized.");

        if (!SD.exists(logDir))
        {
            SD.mkdir(logDir);
        }

        if (!SD.exists(entriesDir))
        {
            SD.mkdir(entriesDir);
        }
    }

    // Method to log data for today in CSV format
    void saveLevelEntry(WaterLevelData *levelData)
    {
        if (levelData == nullptr)
        {
            LOGL("Error: Null water level data received.");
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
            LOGL("Error opening file for writing");
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
            LOGL("Error opening file for writing");
        }
    }

    void streamFile(AsyncWebServerRequest *request)
    {
        bool isEntry = true;
        String date = TimeManager::getDateString();

        if (request->hasParam("loggs", true))
        {
            AsyncWebParameter *p = request->getParam("loggs", true);
            isEntry = p->value() != "true";
        }

        if (request->hasParam("date", true))
        {
            AsyncWebParameter *p = request->getParam("date", true);
            date = p->value();
        }

        if (!TimeManager::isValidDate(date))
        {
            request->send(400, "text/plain", "Invalid date format");
            return;
        }

        String filename = getFileName(date, isEntry);

        AsyncWebServerResponse *resp = request->beginResponse(
            SD,
            filename,
            "text/csv");

        if (filename == TimeManager::getDateString())
        {
            // today’s file → always re‑validate
            resp->addHeader("Cache‑Control", "no-store, no-cache, must-revalidate, max-age=0");
            resp->addHeader("Pragma", "no-cache");
            resp->addHeader("Expires", "0");
        }
        else
        {
            // file is older than today → cache for 1 day
            resp->addHeader("Cache‑Control", "public, max-age=86400");
        }

        request->send(resp);
    }

    // Method to delete files older than a month
    void deleteOldFiles()
    {
        if (TimeManager::getDateString() == _lastDeleteDate)
        {
            return;
        }

        // If we haven't opened the directory yet, do it now
        if (!_dir)
        {
            _dir = SD.open(logDir);
            if (!_dir)
            {
                LOGL("Failed to open log directory");

                _lastDeleteDate = TimeManager::getDateString();

                return;
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

                _lastDeleteDate = TimeManager::getDateString();
                return;
            }
        }

        // Process the current file
        String filename = _currentFile.name();

        if (shouldDeleteFile(filename))
        {
            LOGL("Deleting old file: " + filename);
            SD.remove(filename); // Delete the file
        }

        _currentFile.close();
        _currentFile = File(); // Move to the next file on the next loop

        _lastDeleteDate = TimeManager::getDateString(); // Mark as completed for today
    }
};
