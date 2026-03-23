#include "Util/ScriptParameter.h"

std::string getScriptParameter(const char* templateStr,
                               const std::map<std::string, std::string>& params) {
    std::string result = templateStr ? templateStr : "";
    for (const auto& [key, value] : params) {
        const std::string token = "{" + key + "}";
        std::string::size_type pos = 0;
        while ((pos = result.find(token, pos)) != std::string::npos) {
            result.replace(pos, token.size(), value);
            pos += value.size();
        }
    }
    return result;
}
