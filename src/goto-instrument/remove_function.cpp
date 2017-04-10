/*******************************************************************\

Module: Remove function definition

Author: Michael Tautschnig

Date: April 2017

\*******************************************************************/

#include <util/message.h>

#include <goto-programs/goto_functions.h>

#include "remove_function.h"

/*******************************************************************\

Function: remove_function

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void remove_function(
  symbol_tablet &symbol_table,
  goto_functionst &goto_functions,
  const irep_idt &identifier,
  message_handlert &message_handler)
{
  messaget message(message_handler);

  goto_functionst::function_mapt::iterator entry=
    goto_functions.function_map.find(identifier);

  if(entry==goto_functions.function_map.end())
  {
    message.error() << "No function " << identifier
                    << " in goto program" << messaget::eom;
    return;
  }
  else if(entry->second.is_inlined())
  {
    message.warning() << "Function " << identifier << " is inlined, "
                      << "instantiations will not be removed"
                      << messaget::eom;
  }

  if(entry->second.body_available())
  {
    message.status() << "Removing body of " << identifier
                     << messaget::eom;
    entry->second.clear();
    symbol_table.lookup(identifier).value.make_nil();
  }
}

/*******************************************************************\

Function: remove_functions

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void remove_functions(
  symbol_tablet &symbol_table,
  goto_functionst &goto_functions,
  const std::list<std::string> &names,
  message_handlert &message_handler)
{
  for(const auto &f : names)
    remove_function(symbol_table, goto_functions, f, message_handler);
}
