#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <sqlite3.h>
#include <telnet_logger.h>

#pragma once

class Database
{
private:
    const char *dbPath = "/sd/logs.db";

    sqlite3 *db;
    char *errMsg = nullptr;
    int rc;
    bool dbInitialized = false;

    void executeSQL(const char *sql)
    {
        rc = sqlite3_exec(db, sql, 0, 0, &errMsg);

        if (rc != SQLITE_OK)
        {
            LOGL("SQL error: " + errMsg);
            sqlite3_free(errMsg);
        }
    }

public:
    ~Database()
    {
        close();
    }

    void setup()
    {
        if (!SD.begin())
        {
            LOGL("SD card initialization failed!");
            return;
        }

        LOGL("SD card initialized.");

        rc = sqlite3_open(dbPath, &db);

        if (rc != SQLITE_OK)
        {
            LOGL("Can't open database: " + sqlite3_errmsg(db));
            return;
        }

        LOGL("Database opened successfully.");

        const char *createTableSQL = "CREATE TABLE IF NOT EXISTS logs ("
                                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                     "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                                     "level INTEGER, "
                                     "distance REAL, "
                                     "status INTEGER);";
        executeSQL(createTableSQL);

        close();

        dbInitialized = true;
    }

    void logData(int level, double distance, bool status)
    {
        if (!open())
            return;

        String insertSQL = "INSERT INTO logs (level, distance, status) VALUES (";
        insertSQL += String(level) + ", ";
        insertSQL += String(distance, 2) + ", ";
        insertSQL += String(status ? 1 : 0) + ");";

        executeSQL(insertSQL.c_str());

        LOGL("Data logged successfully.");

        close();
    }

    String getDataForPastDay()
    {
        return retrieveData("SELECT * FROM logs WHERE timestamp >= datetime('now', '-1 day');");
    }

    String getDataForLast7Days()
    {
        return retrieveData("SELECT * FROM logs WHERE timestamp >= datetime('now', '-7 day');");
    }

    String retrieveData(const char *query)
    {
        if (!open())
        {
            return "{}";
        }

        sqlite3_stmt *stmt;
        String jsonResult = "";

        rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        if (rc == SQLITE_OK)
        {
            DynamicJsonDocument doc(1024);
            JsonArray array = doc.to<JsonArray>();

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                JsonObject obj = array.createNestedObject();
                obj["id"] = sqlite3_column_int(stmt, 0);
                obj["timestamp"] = (const char *)sqlite3_column_text(stmt, 1);
                obj["level"] = sqlite3_column_int(stmt, 2);
                obj["distance"] = sqlite3_column_double(stmt, 3);
                obj["status"] = sqlite3_column_int(stmt, 4) == 1;

                doc.garbageCollect();
            }

            serializeJson(doc, jsonResult);
        }
        else
        {
            LOGL("SQL error: " + sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);

        close();

        return jsonResult;
    }

    bool open()
    {
        if (!dbInitialized)
        {
            LOGL("DB is not initialized.");
        }

        int rc = sqlite3_open(dbPath, &db);

        if (rc != SQLITE_OK)
        {
            LOGL("Failed to open the database.");
            return false;
        }

        return true;
    }

    void close()
    {
        if (db)
        {
            sqlite3_close(db);
            db = nullptr;
            LOGL("Database closed.");
        }
    }
};
