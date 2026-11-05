#ifndef CYBERHAND_LR0_AUTOMATON_HPP
#define CYBERHAND_LR0_AUTOMATON_HPP

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <string_view>

namespace cyberhand::syntax::lr0 {

inline constexpr std::string_view END_MARKER = "$";

struct Production { int id{0}; std::string lhs; std::vector<std::string> rhs; bool operator==(const Production&) const = default; };
struct Item { int prod_id{0}; size_t dot{0}; auto operator<=>(const Item&) const = default; };
struct State { int id{0}; std::set<Item> items; };
enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };
struct Action { ActionType type{ActionType::ERROR}; int target{0}; bool operator==(const Action&) const = default; };
struct Conflict { int state_id{0}; std::string symbol, description; };
struct ParseStep { size_t step{0}; std::string stack_str, symbols_str, input_str, action_str; };
struct ParseResult { bool success{false}; std::vector<ParseStep> trace; std::string error_message; };

struct Grammar {
    std::string start_symbol;
    std::vector<std::string> non_terminals, terminals;
    std::vector<Production> productions;
    static Grammar createKinematicGrammar();
};

class LR0Automaton {
public:
    explicit LR0Automaton(Grammar grammar);
    [[nodiscard]] const std::vector<State>& getStates() const noexcept { return states_; }
    [[nodiscard]] const std::map<std::pair<int, std::string>, Action>& getActionTable() const noexcept { return action_table_; }
    [[nodiscard]] const std::map<std::pair<int, std::string>, int>& getGotoTable() const noexcept { return goto_table_; }
    [[nodiscard]] const std::vector<Conflict>& getConflicts() const noexcept { return conflicts_; }
    [[nodiscard]] bool hasConflicts() const noexcept { return !conflicts_.empty(); }
    [[nodiscard]] const Grammar& getGrammar() const noexcept { return grammar_; }
    [[nodiscard]] ParseResult simulate(const std::vector<std::string>& tokens) const;
    void printStates(std::ostream& os = std::cout) const;
    void printTable(std::ostream& os = std::cout) const;
    void printTrace(const ParseResult& res, std::ostream& os = std::cout) const;

private:
    Grammar grammar_;
    std::vector<State> states_;
    std::map<std::pair<int, std::string>, Action> action_table_;
    std::map<std::pair<int, std::string>, int> goto_table_, transitions_;
    std::vector<Conflict> conflicts_;

    void augmentGrammar();
    [[nodiscard]] std::set<Item> closure(const std::set<Item>& items) const;
    [[nodiscard]] std::set<Item> goTo(const std::set<Item>& items, const std::string& symbol) const;
    void buildCanonicalCollection();
    void buildTables();
    [[nodiscard]] bool isNonTerminal(const std::string& s) const;
    [[nodiscard]] bool isTerminal(const std::string& s) const;
    [[nodiscard]] std::string formatItem(const Item& item) const;
};

} // namespace cyberhand::syntax::lr0

#endif // CYBERHAND_LR0_AUTOMATON_HPP
