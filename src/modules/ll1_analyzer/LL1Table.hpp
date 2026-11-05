#ifndef CYBERHAND_LL1_TABLE_HPP
#define CYBERHAND_LL1_TABLE_HPP

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <string_view>

namespace cyberhand::syntax::ll1 {

inline constexpr std::string_view EPSILON = "ε";
inline constexpr std::string_view END_MARKER = "$";

struct Production {
    int id{0};
    std::string lhs;
    std::vector<std::string> rhs;
};

struct Grammar {
    std::string start_symbol;
    std::vector<std::string> non_terminals;
    std::vector<std::string> terminals;
    std::vector<Production> productions;

    static Grammar createKinematicGrammar();
};

struct ParseResult {
    bool success{false};
    std::vector<std::string> trace;
    std::string error_message;
};

class LL1Table {
public:
    explicit LL1Table(Grammar grammar);

    void computeFirstSets();
    void computeFollowSets();
    void buildTable();

    [[nodiscard]] std::set<std::string> firstOfSequence(const std::vector<std::string>& seq) const;
    [[nodiscard]] bool isLL1() const noexcept { return conflict_count_ == 0; }
    [[nodiscard]] size_t conflictCount() const noexcept { return conflict_count_; }

    [[nodiscard]] const std::map<std::string, std::set<std::string>>& getFirstSets() const noexcept { return first_sets_; }
    [[nodiscard]] const std::map<std::string, std::set<std::string>>& getFollowSets() const noexcept { return follow_sets_; }
    [[nodiscard]] const std::map<std::pair<std::string, std::string>, std::vector<Production>>& getTable() const noexcept { return table_; }
    [[nodiscard]] const Grammar& getGrammar() const noexcept { return grammar_; }

    [[nodiscard]] ParseResult simulate(const std::vector<std::string>& tokens) const;

    void printFirstFollow(std::ostream& os = std::cout) const;
    void printTable(std::ostream& os = std::cout) const;

private:
    Grammar grammar_;
    std::map<std::string, std::set<std::string>> first_sets_;
    std::map<std::string, std::set<std::string>> follow_sets_;
    std::map<std::pair<std::string, std::string>, std::vector<Production>> table_;
    size_t conflict_count_{0};

    [[nodiscard]] bool isTerminal(const std::string& s) const;
    [[nodiscard]] bool isNonTerminal(const std::string& s) const;
    [[nodiscard]] std::string tokenToTerminal(const std::string& token) const;
};

} // namespace cyberhand::syntax::ll1

#endif // CYBERHAND_LL1_TABLE_HPP
