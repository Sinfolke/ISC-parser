#include "parser.h"
// no extra tokens here to simplify parsing
/*
    ###########################
            END RULES
    ###########################
*/
#define TOKEN(x) Parser::Token_result Parser::Tokenisator::x (const char* in)
#define Rule(x) Parser::Rule_result Parser::Parser::x (const char* in)
#define TO(t, x) std::any_cast<t>(x)
#define TOKEN_SUCCESS(in, pos, name) return { \
    true, { \
        getCurrentPos(in), in, pos, Parser::Tokens::name \
    } \
};
#define TOKEN_SUCCESSD(in, pos, name, data) return { \
    true, { \
        getCurrentPos(in), in, pos, Parser::Tokens::name, data \
    } \
};
#define RULE_SUCCESS(in, pos, name) return { \
    true, { \
        getCurrentPos(in), in, pos, Parser::Parser::name \
    } \
};
#define RULE_SUCCESSD(in, pos, name, data) return { \
    true, { \
        getCurrentPos(in), in, pos, Parser::Parser::name, data \
    } \
};
TOKEN(END) {
    if (*in == ';' or *in == '\n')
        TOKEN_SUCCESS(in, in + 1, END);
    return {};
}
TOKEN(STRICT_END) {
    if (*in == ';')
        TOKEN_SUCCESS(in, in + 1, STRICT_END);
    return {};
}
TOKEN(NEWLINE) {
    if (*in == '\n')
        TOKEN_SUCCESS(in, in + 1, NEWLINE);
    return {};
}
// <not implemented> use parralel_parsing;
// spacemode mixed;
/*
    ###########################
            FILE RULES
    ###########################
*/
#include "dataTypes.cpp"
Rule(id) {
    int c = 0;
    const char* pos = in;
    Token_result result;
    Group<3> current;
    auto _local_start = pos;
    while (*pos >= '0' and *pos <= '9') {
        ++pos;
    }
    current.push(_local_start, pos);
    skipSpaces(pos);
    if (
        not 
        (
        *pos >= 'a' and *pos <= 'f' || 
        *pos >= 'A' and *pos <= 'F' || 
        *pos == '_'
        )
    ) {
        return {};
    }
    current.push(pos++, 1);
    _local_start = pos;
    skipSpaces(pos);
    while (
        *pos >= 'a' and *pos <= 'f' || 
        *pos >= 'A' and *pos <= 'F' || 
        *pos >= '0' and *pos <= '9' || 
        *pos == '_'
    ) {
        ++pos;
    }
    current.push(_local_start, pos);
    // construct result as valid
    // extract full string from the first group
    RULE_SUCCESSD(in, pos, id, current.fill());
}
Rule(Import_path) {    
    auto pos = in;
    //! <pattern> csequence
    while (
        not 
        (
            *pos == '"' ||
            *pos == '<' ||
            *pos == '>' ||
            *pos == '|' ||
            *pos == ':' ||
            *pos == '?' ||
            *pos == '.' ||
        )
    ) ++pos;
    if (pos == in) return {}; // zero match
    RULE_SUCCESSD(in, pos, Priv_Import_path, string_part(in, pos));
};
Rule(Import_ext) {    
    auto pos = in;
    std::vector<Rule_result> data;
    while(true) {
        if (*pos == '.') {
            ++pos;
            auto match_res = id(pos);
            if (match_res.result) {
                data.push_back(match_res);
                pos += match_res.token.length();
                continue;
            }
            --pos; // dot is skipped
        }
        break;
    }
    if (pos == in) return {}; // zero match
    RULE_SUCCESSD(in, pos, Priv_Import_ext, data);
};
Rule(Import_file) {
    auto path_res = Import_path(in);
    if (!path_res.result) return {};
    auto pos = in + path_res.token.length();
    auto ext_res = Import_ext(pos);
    if (!ext_res.result) return {};
    pos += ext_res.token.length();
    // join
    std::string joined;
    for (auto obj : ext_res) {
        joined += TO(std::string, obj.data);
    }
    std::unordered_map<const char*, std::any> data = {
        { "path", path_res },
        { "ext", TO(std::vector<Rule_result>, ext_res.token.data).back() },
        { "fullext", ext_res },
        { "fullpath", std::string( TO(std::string_part, path_res.token.data )) + joined }
    };
    RULE_SUCCESSD(in, pos, Priv_Import_file, data);
}
Rule(Import_general_dir) {
    auto pos = in;
    if (*pos != '[')
        return {};
    
    auto path_res = Import_path(++pos);
    if (!path_res.result) return {};
    pos += path_res.token.length();

    if (*pos != ']')
        return {};
    
    auto _local_start = ++pos;
    std::vector<Rule> files;
    while(true) {
        auto file_res = Import_file(pos);
        if (!file_res.result)
            break;
        pos += file_res.token.length();
        files.push_back(file_res.token);
    }
    if (pos == _local_start)
        return {}; // zero match
    std::unordered_map<const char*, std::any> data {
        { "path", path_res.token },
        { "files", files }
    };
    RULE_SUCCESSD(in, pos, Priv_Import_general_dir, files);
}
Rule(Import_rule_specific) {
    auto pos = in;
    auto file_res = Import_file(pos);
    if (!file_res.result)
        return {};
    pos += file_res.token.length();
    if (*pos != '{')
        return {};
    auto _local_start = ++pos;
    std::vector<Token> tokens;
    std::vector<Token> tokens_current_name;
    while (true) {
        auto id_res = id(pos);
        if (!id_res.result)
            break;
        tokens.push_back(id_res.token);
        pos += id_res.token.length();
        if (*pos == '=') {
            ++pos;
            auto id2_res = id(pos);
            if (!id2_res.result)
                break;
            tokens_current_name.push_back(id2_res.token);
        }
    }
    if (_local_start == pos)
        return {}; // zero match
    std::unordered_map<const char*, std::any> data {
        { "path", file_res.token },
        { "tokens", tokens },
        { "tokens_current_name", tokens_current_name }
    }
    RULE_SUCCESSD(in, pos, Priv_Import_rule_specific, data);
}
Rule(Import) {
    if (strncmp(in, "import", sizeof("import") - 1))
        return {};
    auto pos = in + sizeof("import") - 1;
    auto first_rule_res = Import_file(pos);
    if (!first_rule_res.result) {
        first_rule_res = Import_general_dir(pos);
        if (!first_rule_res.result) {
            first_rule_res = Import_rule_specific(pos);
            if (!first_rule_res.result)
                return {};
        }
    }
    pos += first_rule_res.token.length();
    std::vector<Token> additional_paths;
    while (*pos == ',') {
        ++pos;
        auto result = Import_file(pos);
        if (!result.result) {
            result = Import_general_dir(pos);
            if (!result.result) {
                result = Import_rule_specific(pos);
                if (!result.result)
                    break;
            }
        }
        pos += result.token.length();
        additional_paths.push_back(result.token);
    }
    using obj_type = std::unordered_map<const char*, std::any>;
    std::vector<obj_type> data = {
        first_rule_res.token
    };
    // push remaining arguments inlining the array
    data.insert(data.end(), additional_paths.begin(), additional_paths.end());
    for (const obj : additional_paths) {
        data.push_back( TO(obj_type, obj.data ) );
    }
    RULE_SUCCESSD(in, pos, Import, data);
}
use: 
    $object<string> data = {  %2.name: %2.value };
    for (int i = 0; i < $1>%2.size(); i++):
        data[$1>%2[i].name] = $1>%2[i].value;


    'use' #USE (',' #USE)*

    #USE:
        ID (':' [^,]+ | STRING)?

        data:
            name: %1,
            value: %2,
;

#undef TOKEN
#undef RULE

