#include <parser.h>
#include <parser_defs.h>

Rule(expr_parenthesed) {
    auto pos = in;
    ISC_STD::skip_spaces(pos);
    if (*pos != '(')
        return {};
    pos++;
    ISC_STD::skip_spaces(pos);
    auto res = expr_parenthesed_variable_assignment(pos);
    if (!res.result) {
        res = copiable_method_call(pos);
        if (!res.result) {
            res = cll_ternary(pos);
            if (!res.result)
                return {};
        }
    }
    pos += res.token.length();
    ISC_STD::skip_spaces(pos);
    if (*pos != ')')
        return {};

    RULE_SUCCESSD(in, pos, expr_parenthesed, res);
}
Rule(expr_parenthesed_variable_assignment) {
    auto pos = in;
    ISC_STD::skip_spaces(pos);
    auto id_res = id(pos);
    if (!id_res.result)
        return {};
    pos += id_res.token.length();
    ISC_STD::skip_spaces(pos);
    auto assignment_operator_res = assignment_op(pos);
    if (!assignment_operator_res.result)
        return {};
    
    pos += assignment_operator_res.token.length();
    ISC_STD::skip_spaces(pos);
    auto assignment_val_res = expr(pos);
    if (!assignment_val_res.result)
        return {};
    
    pos += assignment_val_res.token.length();

    std::unordered_map<const char*, std::any> data {
        { "name", id_res.token },
        { "operator", assignment_operator_res.token },
        { "val", assignment_val_res.token }
    };

    RULE_SUCCESSD(in, pos, expr_parenthesed_variable_assignment, data);
}