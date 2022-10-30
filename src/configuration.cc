#include "configuration.h"

#include <inipp.h>

#include <fstream>

#include "logging.h"

namespace dll_loader {

namespace fs = std::filesystem;

bool LoaderConfiguration::LoadFromFile(const fs::path& configuration_path) {
  inipp::Ini<char> ini;
  // Parse the configuration file
  {
    std::ifstream is(configuration_path);
    if (!is.is_open()) {
      LOG("Failed to open '{}'", configuration_path.string());
      return false;
    }

    ini.parse(is);
  }

  // Extract the target DLL's path
  if (!inipp::get_value(ini.sections[""], "DllPath", dll_path_str)) {
    LOG("Failed to retrieve 'DllPath' value from configuration");
    return false;
  }

  // Extract the DLL mapping configuration
  if (!inipp::get_value(ini.sections[""], "ManualMap", use_manual_mapping)) {
    LOG("Failed to retrieve 'ManualMap' value from configuration");
    return false;
  }

  // Extract the name of the 'OnLoad' function if present
  {
    std::string on_load{};
    if (inipp::get_value(ini.sections[""], "OnLoad", on_load)) {
      on_load_function = on_load;
    }
  }
  // Extract the name of the 'OnUnload' function if present
  {
    std::string on_unload{};
    if (inipp::get_value(ini.sections[""], "OnUnload", on_unload)) {
      on_unload_function = on_unload;
    }
  }

  return true;
}

}  // namespace dll_loader