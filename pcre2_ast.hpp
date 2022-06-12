#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using namespace std;

struct s;
using JsonValue = variant<
  map<string, shared_ptr<s> >,
  double,
  string,
  vector<shared_ptr<s> >,
  bool,
  monostate /*json null*/>;
struct s {
    JsonValue v;
    ~s();
};
inline s::~s() = default;

string to_json(shared_ptr<JsonValue> v);

shared_ptr<JsonValue> from_json(const string& str);
