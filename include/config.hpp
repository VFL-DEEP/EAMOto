#pragma once
#include <string>
#include "auth.hpp"

namespace eamoto {
namespace config {

// Checks if the EAM config file already has a valid unexpired token
bool hasValidToken();

// Writes new token details to the expected GLib keyfile format without destroying other config lines
bool writeTokenDetails(const eamoto::auth::TokenDetails& tokens);

} // namespace config
} // namespace eamoto
