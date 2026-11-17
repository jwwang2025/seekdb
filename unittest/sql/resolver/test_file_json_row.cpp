#include "sql/resolver/dml/ob_file_sql_json_row.h"
#include <cassert>
#include <cstdio>
using namespace oceanbase::sql::file_json;
int main() {
  FlatObject a(20);
  const char valid[] = R"({"\u540d":"\u4e2d\ud83d\ude0a","s":"001","n":1.30e1,"b":true,"z":null})";
  assert(a.parse(valid,sizeof(valid)-1) && a.members_.size()==5);
  assert(a.members_[0].key=="名" && a.members_[0].text=="中😊");
  assert(a.members_[1].text=="001" && a.members_[1].kind==Kind::String);
  assert(a.members_[2].text=="1.30e1" && a.members_[2].kind==Kind::Number);
  assert(a.members_[3].kind==Kind::Boolean && a.members_[4].kind==Kind::Null);
  const char *bad[] = {R"({"a":nan})",R"({"a":2,})",R"({"a":001})",R"({"x":"\uD800"})",
                       R"({"x":{}})",R"({"x":[1]})",R"([3])",R"(null)",R"({"x":1}oops)"};
  for (auto s : bad) { FlatObject x(20); assert(!x.parse(s, strlen(s))); }
  FlatObject limited(1); const char many[] = R"({"a":1,"b":2})";
  assert(!limited.parse(many,sizeof(many)-1));
  FlatObject nul(2); const char trailing[] = "{}\0x"; assert(!nul.parse(trailing,sizeof(trailing)-1));
  puts("JSONL: Unicode escapes, scalar kinds, number text, nested/malformed input and limits passed");
}
