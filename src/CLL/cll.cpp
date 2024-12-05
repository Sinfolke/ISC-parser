#include <parser.h>
#include <parser_defs.h>
Rule(cll) {
    auto pos = in;
    if (*pos != '\n') {
        printf("cll exit 1\n");
        return {};
    }
    pos++;
    // skip spaces
    if (*pos != '$')
        return {};
    pos++;
    // skip spaces
    printf("cll_var\n");
    auto res = cll_var(pos);
    if (!res.result) {
        printf("cll_if\n");
        res = cll_if(pos);
        if (!res.result) {
            printf("expr\n");
            res = expr(pos);
            if (!res.result) {
                printf("copiable_method_call\n");
                res = copiable_method_call(pos);
                if (!res.result)
                    return {};
            }
        }
    }
    pos += res.token.length();
    // skip spaces
    // end
    RULE_SUCCESSD(in, pos, cll, res.token);
}