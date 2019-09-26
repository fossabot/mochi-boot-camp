#include <iostream>
#include <string>

#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>

namespace tl = thallium;

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <address>" << std::endl;
    exit(0);
  }
  tl::engine myEngine("na+sm", THALLIUM_CLIENT_MODE);

  tl::remote_procedure set = myEngine.define("set");
  tl::remote_procedure get = myEngine.define("get");
  tl::remote_procedure remove = myEngine.define("remove");

  tl::endpoint server = myEngine.lookup(argv[1]);

  std::string key = "tokyo";
  std::string value = "mochi";

  int set_ret = set.on(server)(key, value);
  std::cout << "[set] Server answered " << set_ret << std::endl;

  std::string get_ret = get.on(server)(key);
  std::cout << "[get] Server answered " << get_ret << std::endl;

  std::string remove_ret = remove.on(server)(key);
  std::cout << "[remove] Server answered " << remove_ret << std::endl;

  return 0;
}
