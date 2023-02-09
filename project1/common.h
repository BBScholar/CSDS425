#pragma once

#include <cinttypes>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#define DEBUG

const char* k_default_port = "9090";
const char* k_default_server = "localhost";

constexpr int k_max_msg_len = 256;
constexpr int k_buf_len = 4096;

static_assert(2 * k_max_msg_len < k_buf_len, "Need bigger buffers");

enum PacketType : uint8_t {
    PacketGreeting = 0,
    PacketMessage,
    PacketIncoming
} ;

