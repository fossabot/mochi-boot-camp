#include <iostream>
#include <string>

#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <kcpolydb.h>

namespace tl = thallium;
namespace kc = kyotocabinet;

void set(const tl::request& req, std::string& key, std::string& value) {
  std::cout << "[set] " << key << " => " << value << std::endl;
  req.respond((int)(key.size() + value.size()));
}

void get(const tl::request& req, std::string& key) {
  std::cout << "[get] " << key << std::endl;
  req.respond(key);
}

void remove(const tl::request& req, std::string& key) {
  std::cout << "[remove] " << key << std::endl;
  req.respond(key);
}

int main(int argc, char** argv) {
  // TODO: delete
kc::PolyDB db;
if (!db.open("tokyo-mochi-storage.kch", kc::PolyDB::OWRITER | kc::PolyDB::OCREATE)) {
std::cerr << "open error: " << db.error().name() << std::endl;
  }

  db.set("tokyo", "mochi");
std::string value;
db.get("tokyo", &value);
std::cout << "get 'tokyo' => " << value << std::endl;

if (!db.close()) {
  std::cerr << "close error: " << db.error().name() << std::endl;
}
// END

  tl::engine myEngine("na+sm", THALLIUM_SERVER_MODE);
  std::cout << "Server running at address " << myEngine.self() << std::endl;

  myEngine.define("set", set);
  myEngine.define("get", get);
  myEngine.define("remove", remove);

  return 0;
}
