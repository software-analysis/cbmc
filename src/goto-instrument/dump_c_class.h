/*******************************************************************\

Module: Dump Goto-Program as C/C++ Source

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_GOTO_INSTRUMENT_DUMP_C_CLASS_H
#define CPROVER_GOTO_INSTRUMENT_DUMP_C_CLASS_H

#include <set>
#include <string>

#include <util/language.h>

#include <langapi/mode.h>

class dump_ct
{
public:
  dump_ct(
    const goto_functionst &_goto_functions,
    const bool use_system_headers,
    const bool include_harness,
    const namespacet &_ns,
    language_factoryt factory):
    goto_functions(_goto_functions),
    copied_symbol_table(_ns.get_symbol_table()),
    ns(copied_symbol_table),
    language(factory()),
    harness(include_harness)
  {
    if(use_system_headers)
      init_system_library_map();
  }

  virtual ~dump_ct()
  {
    delete language;
  }

  void operator()(std::ostream &out);

protected:
  const goto_functionst &goto_functions;
  symbol_tablet copied_symbol_table;
  const namespacet ns;
  languaget *language;
  const bool harness;

  typedef std::unordered_set<irep_idt, irep_id_hash> convertedt;
  convertedt converted_compound, converted_global, converted_enum;

  std::set<std::string> system_headers;

  typedef std::unordered_map<irep_idt, std::string, irep_id_hash>
    system_library_mapt;
  system_library_mapt system_library_map;

  typedef std::unordered_map<irep_idt, irep_idt, irep_id_hash>
    declared_enum_constants_mapt;
  declared_enum_constants_mapt declared_enum_constants;

  void init_system_library_map();

  std::string type_to_string(const typet &type);
  std::string expr_to_string(const exprt &expr);

  bool ignore(const symbolt &symbol);

  static std::string indent(const unsigned n)
  {
    return std::string(2*n, ' ');
  }

  std::string make_decl(
      const irep_idt &identifier,
      const typet &type)
  {
    symbol_exprt sym(identifier, type);
    code_declt d(sym);

    std::string d_str=expr_to_string(d);
    assert(!d_str.empty());
    assert(*d_str.rbegin()==';');

    return d_str.substr(0, d_str.size()-1);
  }

  void convert_compound_declaration(
      const symbolt &symbol,
      std::ostream &os_body);
  void convert_compound(
      const typet &type,
      const typet &unresolved,
      bool recursive,
      std::ostream &os);
  void convert_compound(
      const struct_union_typet &type,
      const typet &unresolved,
      bool recursive,
      std::ostream &os);
  void convert_compound_enum(
    const typet &type,
    std::ostream &os);

  typedef std::unordered_map<irep_idt, code_declt, irep_id_hash>
          local_static_declst;

  void convert_global_variable(
      const symbolt &symbol,
      std::ostream &os,
      local_static_declst &local_static_decls);

  void convert_function_declaration(
      const symbolt &symbol,
      const bool skip_main,
      std::ostream &os_decl,
      std::ostream &os_body,
      local_static_declst &local_static_decls);

  void insert_local_static_decls(
    code_blockt &b,
    const std::list<irep_idt> &local_static,
    local_static_declst &local_static_decls,
    std::list<irep_idt> &type_decls);

  void insert_local_type_decls(
    code_blockt &b,
    const std::list<irep_idt> &type_decls);

  void cleanup_expr(exprt &expr);
  void cleanup_type(typet &type);
  void cleanup_decl(
    code_declt &decl,
    std::list<irep_idt> &local_static,
    std::list<irep_idt> &local_type_decls);
  void cleanup_harness(code_blockt &b);
};

#endif // CPROVER_GOTO_INSTRUMENT_DUMP_C_CLASS_H
