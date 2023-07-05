#include <memory>
#include <unordered_map>
#include <optional>
#include "ast.h"

typedef struct Scope {
    std::unordered_map<std::string, Type> scope_symbols;

    inline void add_symbol(std::string& sym_name, Type sym_type) {
        scope_symbols.emplace(sym_name, sym_type);
    }

} Scope;

typedef struct SymbolTable {
    std::vector<std::unique_ptr<Scope>> scopes;

    inline std::optional<Type> find_symbol(std::string& sym_name) {
        assert(!scopes.empty());

        // Start at the narrowest scope and move up, looking for the symbol
        for (auto rit = scopes.rbegin(); rit != scopes.rend(); ++rit) {
            auto& scope = *rit;
            if (scope->scope_symbols.count(sym_name) > 0) {
                return scope->scope_symbols[sym_name];
            }
        }

        // Didn't find symbol, so return nothing. Throwing an error is up to the
        // caller.
        return {};
    }
} SymbolTable;