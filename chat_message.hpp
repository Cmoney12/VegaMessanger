//
// Created by corey on 3/17/21.
//

#ifndef CLIENTCHATAPP_CHAT_MESSAGE_HPP
#define CLIENTCHATAPP_CHAT_MESSAGE_HPP

#include <iostream>
#include <cstring>
#include <bson.h>
#include <zstd.h>
#include <chrono>
#include <fstream>


class chat_message {
public:

    chat_message() = default;

    ~chat_message() {
        delete[] file_buffer;
        delete[] cc_buff;
        delete[] data_;
        bson_free((void *) bson);
        bson_destroy(&document);
    }

    uint8_t* data() const
    {
        return data_;
    }

    const uint8_t* data()
    {
        return data_;
    }

    std::size_t length() const
    {
        return HEADER_LENGTH + body_length_;
    }

    uint8_t* body()
    {
        return data_ + HEADER_LENGTH;
    }

    uint8_t* body() const
    {
        return data_ + HEADER_LENGTH;
    }

    std::size_t body_length() const
    {
        return body_length_;
    }

    void set_size(int size) {
        body_length_ = size;
        data_ = new uint8_t[size + HEADER_LENGTH];
    }

    void read_file(const char* file_name) {

        FILE* in_file = fopen(file_name, "rb");

        if (in_file == nullptr) {
            return;
        }

        fseek(in_file, 0L, SEEK_END);

        file_size = ftell(in_file);

        rewind(in_file);

        file_buffer = new unsigned char[file_size];
        size_t read_size = fread(file_buffer, 1, file_size, in_file);

        if (!read_size) {
            return;
        }

        fclose(in_file);

        size_t const c_buff_size = ZSTD_compressBound(file_size);

        cc_buff = new unsigned char [c_buff_size];
        c_size = ZSTD_compress(cc_buff, c_buff_size, file_buffer, file_size, 5);

    }

    unsigned char* decompress(const uint8_t *data, const uint32_t& csize) {

        unsigned long long const rSize = ZSTD_getFrameContentSize(data, csize);
        auto* decompressed = new unsigned char[rSize];

        dSize = ZSTD_decompress(decompressed, rSize, data, csize);

        return decompressed;
    }

    const uint8_t* create_bson(char* receiver, char* deliverer, char* type, char* text = nullptr) {

        bson_init(&document);
        bson_append_utf8(&document, "Receiver", -1, receiver, -1);
        bson_append_utf8(&document, "Deliverer", -1, deliverer, -1);
        bson_append_utf8(&document, "Type", -1, type, -1);
        if(cc_buff != nullptr) {
            bson_append_binary(&document, "Data", -1, BSON_SUBTYPE_BINARY, cc_buff, c_size);
        }

        else if(text != nullptr)
            bson_append_utf8(&document, "Data", -1, text, -1);

        body_length_ = (int)document.len;

        bool steal = true;
        uint32_t size1;

        bson = bson_destroy_with_steal(&document, steal, &size1);

        return bson;

    }

    bool decode_header() {
        std::memcpy(&body_length_, header, sizeof body_length_);
        set_size(body_length_);
        //std::memcpy(body_length_, header, HEADER_LENGTH);
        data_[3] = (body_length_>>24) & 0xFF;
        data_[2] = (body_length_>>16) & 0xFF;
        data_[1] = (body_length_>>8) & 0xFF;
        data_[0] = body_length_ & 0xFF;
        if(body_length_ > MAX_MESSAGE_SIZE) {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    void parse_bson(const uint8_t *bson_data, std::size_t size) {

        const bson_t *received;
        bson_reader_t *reader;
        bson_iter_t iter;
        bson_subtype_t binary_type = BSON_SUBTYPE_BINARY;
        char text_type[] = "Text";

        reader = bson_reader_new_from_data(bson_data, size);

        received = bson_reader_read(reader, nullptr);

        if (bson_iter_init_find(&iter, received, "Receiver") && BSON_ITER_HOLDS_UTF8(&iter)) {
            Receiver = bson_iter_utf8(&iter, nullptr);
        }

        if (bson_iter_init_find(&iter, received, "Deliverer") && BSON_ITER_HOLDS_UTF8(&iter)) {
            Deliverer = bson_iter_utf8(&iter, nullptr);
        }

        if (bson_iter_init_find(&iter, received, "Type") && BSON_ITER_HOLDS_UTF8(&iter)) {
            Content_Type = bson_iter_utf8(&iter, nullptr);
        }

        if (bson_iter_init_find(&iter, received, "Data") && BSON_ITER_HOLDS_UTF8(&iter)) {
            if (Content_Type && std::strcmp(Content_Type, text_type) == 0)
                Text_Message = bson_iter_utf8(&iter, nullptr);
        }
        else if (bson_iter_init_find(&iter, received, "Data") && BSON_ITER_HOLDS_BINARY(&iter)) {
            bson_iter_binary(&iter, &binary_type, &Content_Size, &Content_Buff);
        }

        bson_reader_destroy(reader);

    }

    bool encode_header() const {
        if (body_length_ <= MAX_MESSAGE_SIZE && body_length_) {
            data_[3] = (body_length_>>24) & 0xFF;
            data_[2] = (body_length_>>16) & 0xFF;
            data_[1] = (body_length_>>8) & 0xFF;
            data_[0] = body_length_ & 0xFF;
            int newNu;
            memcpy(&newNu, data_, sizeof newNu);
            return true;
        }
        return false;
    }

    const char* Deliverer{};
    const char* Receiver{};
    const char* Content_Type{};
    uint32_t Content_Size{};
    const uint8_t *Content_Buff{};
    const char* Text_Message{};
    bson_t document{};
    std::size_t file_size{};
    const uint8_t *bson{};
    uint8_t* data_{};
    int body_length_{};
    uint8_t* cc_buff = nullptr;
    unsigned char* file_buffer{};
    enum { MAX_MESSAGE_SIZE = 99999 };
    enum { HEADER_LENGTH = 4 };
    uint8_t header[HEADER_LENGTH]{};
    std::size_t dSize{};
    std::size_t c_size{};
};

#endif //CLIENTCHATAPP_CHAT_MESSAGE_HPP
