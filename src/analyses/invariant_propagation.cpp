/*******************************************************************\

Module: Invariant Propagation

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <util/simplify_expr.h>
#include <util/base_type.h>
#include <util/symbol_table.h>
#include <util/std_expr.h>

#include "invariant_propagation.h"

/*******************************************************************\

Function: invariant_propagationt::make_all_true

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::make_all_true()
{
  for(auto &state : state_map)
    state.second.invariant_set.make_true();
}

/*******************************************************************\

Function: invariant_propagationt::make_all_false

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::make_all_false()
{
  for(auto &state : state_map)
    state.second.invariant_set.make_false();
}

/*******************************************************************\

Function: invariant_propagationt::add_objects

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::add_objects(
  const goto_programt &goto_program)
{
  // get the globals
  object_listt globals;
  get_globals(globals);

  // get the locals
  goto_programt::decl_identifierst locals;
  goto_program.get_decl_identifiers(locals);

  // cache the list for the locals to speed things up
  typedef std::unordered_map<irep_idt, object_listt, irep_id_hash>
    object_cachet;
  object_cachet object_cache;

  forall_goto_program_instructions(i_it, goto_program)
  {
    #if 0
    invariant_sett &is=(*this)[i_it].invariant_set;

    is.add_objects(globals);
    #endif

    for(const auto &local : locals)
    {
      // cache hit?
      object_cachet::const_iterator e_it=object_cache.find(local);

      if(e_it==object_cache.end())
      {
        const symbolt &symbol=ns.lookup(local);

        object_listt &objects=object_cache[local];
        get_objects(symbol, objects);
        #if 0
        is.add_objects(objects);
        #endif
      }
      #if 0
      else
        is.add_objects(e_it->second);
      #endif
    }
  }
}

/*******************************************************************\

Function: invariant_propagationt::get_objects

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::get_objects(
  const symbolt &symbol,
  object_listt &dest)
{
  std::list<exprt> object_list;

  get_objects_rec(symbol.symbol_expr(), object_list);

  for(const auto &expr : object_list)
    dest.push_back(object_store.add(expr));
}

/*******************************************************************\

Function: invariant_propagationt::get_objects_rec

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::get_objects_rec(
  const exprt &src,
  std::list<exprt> &dest)
{
  const typet &t=ns.follow(src.type());

  if(t.id()==ID_struct ||
     t.id()==ID_union)
  {
    const struct_typet &struct_type=to_struct_type(t);

    const struct_typet::componentst &c=struct_type.components();

    exprt member_expr(ID_member);
    member_expr.copy_to_operands(src);

    for(const auto &component : c)
    {
      member_expr.set(ID_component_name, component.get_name());
      member_expr.type()=component.type();
      // recursive call
      get_objects_rec(member_expr, dest);
    }
  }
  else if(t.id()==ID_array)
  {
    // get_objects_rec(identifier, suffix+"[]", t.subtype(), dest);
    // we don't track these
  }
  else if(check_type(t))
  {
    dest.push_back(src);
  }
}

/*******************************************************************\

Function: invariant_propagationt::add_vars

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::add_objects(
  const goto_functionst &goto_functions)
{
  // get the globals
  object_listt globals;
  get_globals(globals);

  forall_goto_functions(f_it, goto_functions)
  {
    // get the locals
    std::set<irep_idt> locals;
    get_local_identifiers(f_it->second, locals);

    const goto_programt &goto_program=f_it->second.body;

    // cache the list for the locals to speed things up
    typedef std::unordered_map<irep_idt, object_listt, irep_id_hash>
      object_cachet;
    object_cachet object_cache;

    forall_goto_program_instructions(i_it, goto_program)
    {
      #if 0
      invariant_sett &is=(*this)[i_it].invariant_set;

      is.add_objects(globals);
      #endif

      for(const auto &local : locals)
      {
        // cache hit?
        object_cachet::const_iterator e_it=object_cache.find(local);

        if(e_it==object_cache.end())
        {
          const symbolt &symbol=ns.lookup(local);

          object_listt &objects=object_cache[local];
          get_objects(symbol, objects);
          #if 0
          is.add_objects(objects);
          #endif
        }
        #if 0
        else
          is.add_objects(e_it->second);
        #endif
      }
    }
  }
}

/*******************************************************************\

Function: invariant_propagationt::get_globals

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::get_globals(
  object_listt &dest)
{
  // static ones
  forall_symbols(it, ns.get_symbol_table().symbols)
    if(it->second.is_lvalue &&
       it->second.is_static_lifetime)
      get_objects(it->second, dest);
}

/*******************************************************************\

Function: invariant_propagationt::check_type

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool invariant_propagationt::check_type(const typet &type) const
{
  if(type.id()==ID_pointer)
    return true;
  else if(type.id()==ID_struct ||
          type.id()==ID_union)
    return false;
  else if(type.id()==ID_array)
    return false;
  else if(type.id()==ID_symbol)
    return check_type(ns.follow(type));
  else if(type.id()==ID_unsignedbv ||
          type.id()==ID_signedbv)
    return true;
  else if(type.id()==ID_bool)
    return true;

  return false;
}

/*******************************************************************\

Function: invariant_propagationt::initialize

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::initialize(const goto_programt &goto_program)
{
  baset::initialize(goto_program);

  forall_goto_program_instructions(it, goto_program)
  {
    invariant_sett &s=state_map[it].invariant_set;

    if(it==goto_program.instructions.begin())
      s.make_true();
    else
      s.make_false();

    s.set_value_sets(value_sets);
    s.set_object_store(object_store);
    s.set_namespace(ns);
  }

  add_objects(goto_program);
}

/*******************************************************************\

Function: invariant_propagationt::initialize

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::initialize(const goto_functionst &goto_functions)
{
  baset::initialize(goto_functions);

  forall_goto_functions(f_it, goto_functions)
    initialize(f_it->second.body);
}

/*******************************************************************\

Function: invariant_propagationt::simplify

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::simplify(goto_functionst &goto_functions)
{
  Forall_goto_functions(it, goto_functions)
    simplify(it->second.body);
}

/*******************************************************************\

Function: invariant_propagationt::simplify

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void invariant_propagationt::simplify(goto_programt &goto_program)
{
  Forall_goto_program_instructions(i_it, goto_program)
  {
    if(!i_it->is_assert())
      continue;

    // find invariant set
    state_mapt::const_iterator s_it=state_map.find(i_it);
    if(s_it==state_map.end())
      continue;

    const invariant_sett &invariant_set=s_it->second.invariant_set;

    exprt simplified_guard(i_it->guard);

    invariant_set.simplify(simplified_guard);
    ::simplify(simplified_guard, ns);

    if(invariant_set.implies(simplified_guard).is_true())
      i_it->guard=true_exprt();
  }
}
