#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

using namespace std;

struct s;
using JsonValue = variant<
  map<string, unique_ptr<s> >,
  double,
  string,
  vector<unique_ptr<s> >,
  bool,
  monostate /*json null*/>;
struct s {
    JsonValue v;
    ~s();
};
inline s::~s() = default;

string to_json(unique_ptr<JsonValue> v);

unique_ptr<JsonValue> from_json(const string& str);
