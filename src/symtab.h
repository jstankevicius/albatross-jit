#pragma once

#include "ast.h"
#include <memory>
#include <optional>
#include <unordered_map>

template <typename T> struct Scope {
  std::unordered_map<std::string, T> symbols;

  inline void add_symbol(std::string &sym_name, T info) {
    symbols.emplace(sym_name, info);
  }

  inline std::optional<T> find_symbol(std::string &sym_name) {
    if (symbols.count(sym_name) > 0) {
      return symbols[sym_name];
    }

    return {};
  }
};

// This struct should persist for the entire duration of the program. It should
// never be destructed unless the interpreter process is dead.
template <typename T> struct SymbolTable {
  std::vector<std::unique_ptr<Scope<T>>> scopes;

  int sym_idx = 0;

  inline void add_symbol(std::string &sym_name, T info) {
    cur_scope()->add_symbol(sym_name, info);
    sym_idx++;
  }

  inline std::optional<T> find_symbol(std::string &sym_name) {
    assert(!scopes.empty());

    // Start at the narrowest scope and move up, looking for the symbol
    for (auto rit = scopes.rbegin(); rit != scopes.rend(); ++rit) {
      auto &scope = *rit;
      auto sym = scope->find_symbol(sym_name);
      if (sym) {
        return sym;
      }
    }

    // Didn't find symbol, so return nothing. Throwing an error is up to the
    // caller.
    return {};
  }

  inline std::unique_ptr<Scope<T>> &cur_scope() { return scopes.back(); }

  inline void enter_scope() { scopes.push_back(std::make_unique<Scope<T>>()); }

  inline void exit_scope() {
    assert(scopes.size() > 1);
    scopes.pop_back();
  }
};