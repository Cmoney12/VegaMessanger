//
// Created by coreylovette on 2/17/21.
//

#include <sqlite3.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>

#ifndef CLIENTCHATAPP_DATABASE_HANDLER_H
#define CLIENTCHATAPP_DATABASE_HANDLER_H

class database_handler {
public:
    bool login(const std::string &user_name, const std::string &pass_word) {

        if (sqlite3_open("/home/coreylovette/CLionProjects/ClientChatApp/messanger_db.sqlite", &db) == SQLITE_OK) {
            //rc = sqlite3_open("/home/coreylovette/CLionProjects/ClientChatApp/messanger_db.sqlite", &db);
            std::string sql = "SELECT * FROM Login WHERE username='" +
                              user_name + "' and password='" + pass_word + "'";
            struct sqlite3_stmt *selectstmt;
            int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &selectstmt, nullptr);
            if (result == SQLITE_OK) {
                sqlite3_free(db);
                if (sqlite3_step(selectstmt) == SQLITE_ROW) {
                    // record found
                    sqlite3_finalize(selectstmt);
                    return true;
                } else {

                    return false;
                }
            }
        } else {
            return false;
        }
        sqlite3_close(db);
    }

    bool register_user(const std::string &user_name, const std::string &pass_word) {
        rc = sqlite3_open("/home/coreylovette/CLionProjects/ClientChatApp/messanger_db.sqlite", &db);
        std::string sql = "INSERT INTO Login VALUES('" + user_name + "','" + pass_word + "')";
        int exit = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        if (exit != SQLITE_OK) {
            return true;
        }
        else {
            return false;
        }
    }

    bool valid_database() {
        rc = sqlite3_open("messanger_db.sqlite", &db);
        if (!rc) {
            std::string create_table = "CREATE TABLE Login (username TEXT, password TEXT)";
            rc = sqlite3_exec(db, create_table.c_str(), nullptr, nullptr, &zErrMsg);
            if (rc != SQLITE_OK) {
                sqlite3_free(zErrMsg);
                return false;
            } else {
                return true;
            }
        } else {
            return false;
        }
    }

    int insert_message(const std::string& message) {

        rc = sqlite3_open("/home/coreylovette/CLionProjects/ClientChatApp/messanger_db.sqlite", &db);
        //rc = sqlite3_open("messanger_db.sqlite", &db);
        if (rc != SQLITE_OK) {

            //fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));

            return 1;
        }

        std::string sql = "INSERT INTO Messages VALUES('" + message + "','" + get_current_datetime() +"')";

        rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);

        if (rc != SQLITE_OK ) {

            //fprintf(stderr, "SQL error: %s\n", zErrMsg);

            sqlite3_close(db);

            return 1;
        }
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
    }

    static std::string get_current_datetime() {
        time_t t = time(nullptr);
        struct tm *tm = localtime(&t);
        std::string datetime = asctime(tm);
        return datetime;

    }

    void clear_messages() {
        rc = sqlite3_open("/home/coreylovette/CLionProjects/ClientChatApp/messanger_db.sqlite", &db);
        std::string sql = "DELETE FROM Messages";
        rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);
    }

    int rc{};
    sqlite3 *db{};
    char *zErrMsg = nullptr;
};

#endif //CLIENTCHATAPP_DATABASE_HANDLER_H
