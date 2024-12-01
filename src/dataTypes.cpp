// STRING Rule
#include <parser.h>
#include <parser_defs.h>
Rule(string) {
    const char* pos = in;
    char quote;
    std::string data;
    ISC_STD::skip_spaces(pos);
    if (not (*pos == '"' || *pos == '\'') )
        return {};
    quote = *pos++;
    // Capture the string content
    while (*pos != quote) {
        ISC_STD::skip_spaces(pos);
        if (*pos == '\\') {
            data += *pos++; // Skip escape character
        }
        data += *pos++;
    }
    pos++; // Skip the closing quote
    RULE_SUCCESSD(in, pos, string, data);
}

// NUMBER Rule
Rule(number) {
    const char* pos = in;
    std::string sign(1, *pos);
    std::string main, dec;
    bool hasPoint = false;

    ISC_STD::skip_spaces(pos);
    // Check for sign
    if (*pos == '+' || *pos == '-') {
        pos++;
    }
    ISC_STD::skip_spaces(pos);
    // Read main number
    while (isdigit(*pos)) {
        main += *pos++;
    }
    ISC_STD::skip_spaces(pos);
    // Check for decimal point
    if (*pos == '.' || *pos == ',') {
        hasPoint = true;
        pos++;
        ISC_STD::skip_spaces(pos);
        while (isdigit(*pos)) {
            dec += *pos++;
        }
        ISC_STD::skip_spaces(pos);
    }

    std::string full = sign + main + (hasPoint ? (".") + dec : "");
    double main_n = std::stod(main);
    double dec_n = hasPoint ? std::stod(dec) : 0.0;
    std::unordered_map<const char*, std::any> data {
        { "sign", sign },
        { "main", main },
        { "main_n", main_n },
        { "dec", dec },
        { "dec_n", dec_n },
        { "full", sign + main + std::string(".") + dec }
    };
    RULE_SUCCESSD(in, pos, boolean, data);
}

// BOOLEAN Rule
Rule(boolean) {
    const char* pos = in;
    std::string d;
    int val;
    std::unordered_map<const char*, std::any> data;
    ISC_STD::skip_spaces(pos);
    if (!strncmp(in, "true", sizeof("true"))) {
        d = "true";
        val = 1;
    } else if (not strncmp(in, "false", sizeof("false"))) {
        d = "false";
        val = 0;
    } else {
        return {};
    }
    data = {
        { "val", val},
        { "lit", d }
    };
    RULE_SUCCESSD(in, pos, boolean, data);
}

// ARRAY Rule
Rule(array) {
    const char* pos = in;
    std::vector<std::any> data;
    ISC_STD::skip_spaces(pos);
    if (*pos != '[')
        return {};
    pos++;
    ISC_STD::skip_spaces(pos);
    auto any_data_f = any_data(pos);
    if (!any_data_f.result) 
        goto arrayClose; // should be closed immediately
    
    data.push_back(any_data_f.token);
    pos += any_data_f.token.length();
    ISC_STD::skip_spaces(pos);
    while(*pos == ',') {
        ISC_STD::skip_spaces(pos);
        auto any_data_s = any_data(pos + 1);
        if (!any_data_s.result)
            break;
        pos += any_data_s.token.length();
    }
    arrayClose:
    if (*pos != ']')
        return {};
    RULE_SUCCESSD(in, pos, array, data);
}

// OBJECT Rule
Rule(object) {
    const char* pos = in;
    std::unordered_map<std::string, std::any> data;
    std::vector<std::string> keys;
    std::vector<std::any> values;

    ISC_STD::skip_spaces(pos);
    if (*pos != '{')
        return {};
    ++pos;
    ISC_STD::skip_spaces(pos);
    auto id_res = id(pos);
    if (id_res.result) {
        pos += id_res.token.length();
        ISC_STD::skip_spaces(pos);
        if (*pos != ':')
            return {};
        pos++;
        ISC_STD::skip_spaces(pos);
        auto any_data_res = any_data(pos);
        if (!any_data_res.result)
            return {};
        data[TO (std::string, id_res.token.data )] = any_data_res.token;
        ISC_STD::skip_spaces(pos);
        while (*pos == ',') {
            ISC_STD::skip_spaces(pos);
            ++pos;
            id_res = id(pos);
            if (not id_res.result)
                break;
            pos += id_res.token.length();
            ISC_STD::skip_spaces(pos);
            if (*pos!= ':')
                break;
            ISC_STD::skip_spaces(pos);
            any_data_res = any_data(++pos);
            if (!any_data_res.result)
                break;
            data[TO(std::string, id_res.token.data)] = any_data_res.token;
            pos += any_data_res.token.length();
        }
    }
    if (*pos != '}')
        return {};
    RULE_SUCCESSD(in, pos, object, data);
}

// ANY_DATA Rule
Rule(any_data) {
    const char* pos = in;
    std::any data;

    // Try each type in order
    ::Parser::Rule_result result = boolean(in);
    ::Parser::Rule token;
    if (result.result)
        token = result.token;
    result = number(in);
    if (result.result)
        token = result.token;
    result = string(in);
    if (result.result)
        token = result.token;
    result = array(in);
    if (result.result)
        token = result.token;
    result = object(in);
    if (result.result)
        token = result.token;
    if (token.empty())
        return {};
    RULE_SUCCESSD(in, in + token.length(), any_data, token);
}