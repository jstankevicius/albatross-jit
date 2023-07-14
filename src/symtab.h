#pragma once

#include "ast.h"
#include <memory>
#include <optional>
#include <unordered_map>

typedef struct Scope {
  std::unordered_map<std::string, VarInfo> var_defs;
  std::unordered_map<std::string, FunInfo> fun_defs;

  inline void add_symbol(std::string &sym_name, VarInfo info) {
    var_defs.emplace(sym_name, info);
  }

  inline void add_function(std::string &fun_name, FunInfo info) {
    fun_defs.emplace(fun_name, info);
  }

  inline std::optional<VarInfo> find_symbol(std::string &sym_name) {
    if (var_defs.count(sym_name) > 0) {
      return var_defs[sym_name];
    }

    return {};
  }

  inline std::optional<FunInfo> find_function(std::string &fun_name) {
    if (fun_defs.count(fun_name) > 0) {
      return fun_defs[fun_name];
    }

    return {};
  }
} Scope;

// This struct should persist for the entire duration of the program. It should
// never be destructed unless the interpreter process is dead.
typedef struct SymbolTable {
  std::vector<std::unique_ptr<Scope>> scopes;

  // The next DeBruijn index to be assigned to a variable.
  int var_idx_db = 0;

  // Because function locations are not reused, they are counted differently
  // from variable locations.
  int fun_idx = 0;

  inline std::optional<VarInfo> find_symbol(std::string &sym_name) {
    assert(!scopes.empty());

    // Start at the narrowest scope and move up, looking for the symbol
    for (auto rit = scopes.rbegin(); rit != scopes.rend(); ++rit) {
      auto &scope = *rit;
      auto sym = scope->find_symbol(sym_name);
      if (sym)
        return sym;
    }

    // Didn't find symbol, so return nothing. Throwing an error is up to the
    // caller.
    return {};
  }

  inline std::optional<FunInfo> find_function(std::string &fun_name) {
    assert(!scopes.empty());

    // Start at the narrowest scope and move up, looking for the symbol
    for (auto rit = scopes.rbegin(); rit != scopes.rend(); ++rit) {
      auto &scope = *rit;
      auto fun = scope->find_function(fun_name);
      if (fun)
        return fun;
    }

    // Didn't find symbol, so return nothing. Throwing an error is up to the
    // caller.
    return {};
  }

  inline void enter_scope() { scopes.push_back(std::make_unique<Scope>()); }

  inline std::unique_ptr<Scope> &cur_scope() { return scopes.back(); }

  inline void exit_scope() {
    assert(scopes.size() > 1);
    var_idx_db -= cur_scope()->var_defs.size();
    scopes.pop_back();
  }

  inline void add_symbol(std::string &sym_name, Type sym_type) {
    cur_scope()->add_symbol(sym_name, VarInfo{sym_type, var_idx_db});
    printf("Added symbol %s w/ DBI %d in scope %ld\n", sym_name.c_str(),
           var_idx_db, scopes.size());
    ++var_idx_db;
  }

  inline void add_function(std::string &fun_name, Type ret_type,
                           std::vector<TypeNode> params) {
    cur_scope()->add_function(fun_name, FunInfo{ret_type, fun_idx, params});
    printf("Added function %s w/ index %d in scope %ld\n", fun_name.c_str(),
           fun_idx, scopes.size());
    ++fun_idx;
  }

} SymbolTable;