//
// Created by maciek on 4/19/23.
//

#ifndef RSO2_CHAT_DB_HELPER_H
#define RSO2_CHAT_DB_HELPER_H

#include <iostream>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

#define BUF_SIZE 40000

using namespace rapidjson;

const char *DB_FILE_NAME = "db.json";

bool add_user_to_json_file(const std::string &username, const std::string &password, const std::string &token) {
    FILE *file_pointer = fopen(DB_FILE_NAME, "r+");
    if (!file_pointer) {
        std::cerr << "Failed to open " << DB_FILE_NAME << std::endl;
        return false;
    }

    char r_buf[BUF_SIZE];
    FileReadStream is(file_pointer, r_buf, sizeof(r_buf));
    Document doc;
    doc.ParseStream(is);
    if (!doc.IsArray()) {
        std::cerr << "Invalid user database file format: expected an array at the top level" << std::endl;
        fclose(file_pointer);
        return false;
    }


    for (const auto &user: doc.GetArray()) {
        const auto &user_username = user["user"].GetString();
        if (strcmp(user_username, username.c_str()) == 0) {
            std::cerr << "User already exists" << std::endl;
            fclose(file_pointer);
            return false;
        }
    }


    Value newUser(kObjectType);
    newUser.AddMember("user", Value().SetString(username.c_str(), username.length(), doc.GetAllocator()),
                      doc.GetAllocator());
    newUser.AddMember("password", Value().SetString(password.c_str(), password.length(), doc.GetAllocator()),
                      doc.GetAllocator());
    newUser.AddMember("token", Value().SetString(token.c_str(), token.length()), doc.GetAllocator());


    doc.PushBack(newUser, doc.GetAllocator());


    fseek(file_pointer, 0, SEEK_SET);
    char w_buf[BUF_SIZE];
    FileWriteStream os(file_pointer, w_buf, sizeof(w_buf));
    Writer<FileWriteStream> writer(os);
    doc.Accept(writer);


    fclose(file_pointer);
    return true;
}

bool user_exists_in_json_file(const std::string &username) {

    FILE *file_pointer = fopen(DB_FILE_NAME, "r");
    if (!file_pointer) {
        std::cerr << "Failed to open " << DB_FILE_NAME << std::endl;
        return false;
    }


    char r_buf[BUF_SIZE];
    FileReadStream is(file_pointer, r_buf, sizeof(r_buf));
    Document doc;
    doc.ParseStream(is);
    if (!doc.IsArray()) {
        std::cerr << "Invalid user database file format: expected an array at the top level" << std::endl;
        fclose(file_pointer);
        return false;
    }


    for (const auto &user: doc.GetArray()) {
        const auto &user_username = user["user"].GetString();
        if (strcmp(user_username, username.c_str()) == 0) {
            fclose(file_pointer);
            return true;
        }
    }

    fclose(file_pointer);
    return false;
}

void replace_user_in_json_file(const std::string& username, const std::string& password, const std::string& token)
{
    FILE *file_pointer = fopen(DB_FILE_NAME, "r");
    if (!file_pointer) {
        std::cerr << "Failed to open " << DB_FILE_NAME << std::endl;
        return;
    }

    char r_buf[BUF_SIZE];
    FileReadStream is(file_pointer, r_buf, sizeof(r_buf));
    Document doc;
    doc.ParseStream(is);
    if (!doc.IsArray()) {
        std::cerr << "Invalid user database file format: expected an array at the top level" << std::endl;
        fclose(file_pointer);
        return;
    }


    for (Value::ValueIterator itr = doc.Begin(); itr != doc.End(); ++itr) {
        if ((*itr)["user"].GetString() == username) {

            (*itr)["password"].SetString(password.c_str(), password.size(), doc.GetAllocator());
            (*itr)["token"].SetString(token.c_str(), token.size(), doc.GetAllocator());

            FILE *tmp = tmpfile();
            if (!tmp) {
                std::cerr << "Failed to create temporary file" << std::endl;
                fclose(file_pointer);
                return;
            }
            FileWriteStream os(tmp, r_buf, sizeof(r_buf));
            Writer<FileWriteStream> writer(os);
            doc.Accept(writer);

            fclose(file_pointer);
            fseek(tmp, 0, SEEK_SET);
            FILE *newfile_pointer = fopen(DB_FILE_NAME, "w");
            if (!newfile_pointer) {
                std::cerr << "Failed to open " << DB_FILE_NAME << " for writing" << std::endl;
                fclose(tmp);
                return;
            }
            char buf[4096];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), tmp)) > 0) {
                if (fwrite(buf, 1, n, newfile_pointer) != n) {
                    std::cerr << "Failed to write to " << DB_FILE_NAME << std::endl;
                    fclose(tmp);
                    fclose(newfile_pointer);
                    return;
                }
            }
            fclose(tmp);
            fclose(newfile_pointer);
            return;
        }
    }

    std::cerr << "User not found: " << username << std::endl;
    fclose(file_pointer);
}

//std::string get_user_token_in_json_file(const std::string &username) {
//    FILE *file_pointer = fopen(DB_FILE_NAME, "r");
//    if (!file_pointer) {
//        std::cerr << "Failed to open " << DB_FILE_NAME << std::endl;
//        return "";
//    }
//
//    char r_buf[BUF_SIZE];
//    FileReadStream is(file_pointer, r_buf, sizeof(r_buf));
//    Document doc;
//    doc.ParseStream(is);
//    if (!doc.IsArray()) {
//        std::cerr << "Invalid user database file format: expected an array at the top level" << std::endl;
//        fclose(file_pointer);
//        return "";
//    }
//
//    for (const auto &user: doc.GetArray()) {
//        const auto &user_username = user["user"].GetString();
//        if (strcmp(user_username, username.c_str()) == 0) {
//            const auto &user_token = user["token"].GetString();
//            fclose(file_pointer);
//            return user_token;
//        }
//    }
//}

std::string get_user_pwd_in_json_file(const std::string &username) {
    FILE *file_pointer = fopen(DB_FILE_NAME, "r");
    if (!file_pointer) {
        std::cerr << "Failed to open " << DB_FILE_NAME << std::endl;
        return "";
    }

    char r_buf[BUF_SIZE];
    FileReadStream is(file_pointer, r_buf, sizeof(r_buf));
    Document doc;
    doc.ParseStream(is);
    if (!doc.IsArray()) {
        std::cerr << "Invalid user database file format: expected an array at the top level" << std::endl;
        fclose(file_pointer);
        return "";
    }


    for (const auto &user: doc.GetArray()) {
        const auto &user_username = user["user"].GetString();
        if (strcmp(user_username, username.c_str()) == 0) {
            const auto &user_password = user["password"].GetString();
            fclose(file_pointer);
            return user_password;
        }
    }

    fclose(file_pointer);
    return "";
}

#endif //RSO2_CHAT_DB_HELPER_H
