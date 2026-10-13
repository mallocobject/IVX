#include <cstdint>
#include <iostream>
#include <string>
#include <type_traits>
#include <unordered_map>

std::unordered_map<std::string, std::string>
pre_process(const std::string &code) {
    // TODO
    std::unordered_map<std::string, std::string> mp;
    const std::string symbol{"#type"};
    std::remove_const_t<decltype(std::string::npos)> begin = 0;
    auto pos = code.find(symbol, begin);
    while (pos != std::string::npos) {
        auto crlf = code.find_first_of("\r\n", pos + 1);
        std::string type = code.substr(pos + symbol.size() + 1,
                                       crlf - (pos + symbol.size() + 1));
        begin = code.find_first_not_of("\r\n", crlf);
        pos = code.find(symbol, begin);

        std::string src;
        if (pos != std::string::npos) {
            src = code.substr(begin, pos - begin);
        } else {
            src = code.substr(begin);
        }
        mp[type] = src;
    }

    return mp;
}

int main() {
    std::string code = R"(
    #type vertex
    app
    
    #type fragment
    asdad
    asdada
    
    
    
    #type pixel
    )";

    pre_process(code);
}