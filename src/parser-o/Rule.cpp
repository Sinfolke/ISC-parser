Rule(Rule) {
    auto pos = in;
    auto id_res = id(pos);
    if (!id_res.result)
        return {};
    pos += id_res.token.length();
    Rule_result Rule_rule_res;
    while ((Rule_rule_res = Rule_rule(pos)).result) {
        // do something
    }

    Rule_result Rule_nested_rule_res;
    while ((Rule_nested_rule_res = Rule_rule(pos)).result) {
        // do something
    }
    auto strict_end_result = STRICT_END();
}
#include "Rule/csequence.cpp"
#include "Rule/data_block.cpp"
#include "Rule/nested_rule.cpp"
#include "Rule/hex.cpp"
#include "Rule/bin.cpp"