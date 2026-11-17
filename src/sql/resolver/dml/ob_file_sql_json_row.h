// Copyright (c) 2025 OceanBase. Licensed under the Apache License, Version 2.0.
#pragma once

#include <rapidjson/memorystream.h>
#include <rapidjson/reader.h>
#include <cstring>
#include <string>
#include <vector>

namespace oceanbase {
namespace sql {
namespace file_json {

enum class Kind { Null, Boolean, Number, String };

struct Member {
  std::string key;
  std::string text;
  Kind kind;
};

class FlatObject : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, FlatObject>
{
public:
  explicit FlatObject(size_t limit) : limit_(limit) {}
  bool StartObject() { if (started_) { nested_ = true; return false; } started_ = true; return true; }
  bool EndObject(rapidjson::SizeType) { ended_ = true; return true; }
  bool StartArray() { nested_ = true; return false; }
  bool Key(const char *s, rapidjson::SizeType n, bool) {
    if (members_.size() >= limit_) { return false; }
    members_.push_back({std::string(s, n), {}, Kind::Null});
    return true;
  }
  bool String(const char *s, rapidjson::SizeType n, bool) { return add(Kind::String, s, n); }
  bool RawNumber(const char *s, rapidjson::SizeType n, bool) { return add(Kind::Number, s, n); }
  bool Bool(bool value) { return add(Kind::Boolean, value ? "true" : "false", value ? 4 : 5); }
  bool Null() { return add(Kind::Null, "", 0); }
  bool parse(const char *s, size_t n) {
    // RapidJSON treats NUL as end of input. A raw NUL is never legal JSON.
    if (std::memchr(s, 0, n) != nullptr) { return false; }
    rapidjson::MemoryStream stream(s, n);
    rapidjson::Reader reader;
    return reader.Parse<rapidjson::kParseValidateEncodingFlag | rapidjson::kParseNumbersAsStringsFlag>(stream, *this)
        && started_ && ended_ && stream.Tell() == n;
  }
  std::vector<Member> members_;
  bool nested_ = false;
private:
  bool add(Kind kind, const char *s, size_t n) {
    if (members_.empty()) { return false; }
    members_.back().kind = kind;
    members_.back().text.assign(s, n);
    return true;
  }
  size_t limit_;
  bool started_ = false;
  bool ended_ = false;
};

} // namespace file_json
} // namespace sql
} // namespace oceanbase
