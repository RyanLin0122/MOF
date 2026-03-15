#include "Util/ScriptParameter.h"

// Stub: returns template unchanged (parameter substitution not yet implemented).
std::string getScriptParameter(const char* templateStr,
                               const std::map<std::string, std::string>& /*params*/) {
    return templateStr ? templateStr : "";
}
