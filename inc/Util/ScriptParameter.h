#pragma once
#include <map>
#include <string>

// Stub for getScriptParameter used in ground truth.
// Substitutes named parameters in a template string.
// e.g. template "You picked up {Parameter0} x{Parameter1}"
//      params {"Parameter0": "Sword", "Parameter1": "1"}
// Returns the substituted string, or the original template if no substitution.
std::string getScriptParameter(const char* templateStr,
                               const std::map<std::string, std::string>& params);
