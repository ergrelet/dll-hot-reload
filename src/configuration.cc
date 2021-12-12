#include "configuration.h"

#include <inipp.h>

#include <fstream>

#include "logging.h"

namespace dll_loader {

namespace fs = std::filesystem;

// LoaderConfiguration

bool LoaderConfiguration::LoadFromFile(const fs::path& configuration_path) {
  std::ifstream is(configuration_path);
  if (!is.is_open()) {
    LOG("Failed to open '{}'", configuration_path.string());
    return false;
  }
  inipp::Ini<char> ini;
  ini.parse(is);

  if (!inipp::get_value(ini.sections[""], "DllPath", dll_path_str)) {
    LOG("Failed to retrieve 'DllPath' value from configuration");
    return false;
  }

  return true;
}

}  // namespace dll_loader