#pragma once

#include <cinttypes>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

constexpr int k_max_msg_len = 256;
constexpr int k_buf_len = 4096;

static_assert(2 * k_max_msg_len < k_buf_len, "Need bigger buffers");
