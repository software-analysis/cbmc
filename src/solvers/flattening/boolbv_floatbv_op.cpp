/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <algorithm>
#include <iostream>

#include <util/std_types.h>

#include "boolbv.h"

#include "../floatbv/float_utils.h"

/*******************************************************************\

Function: boolbvt::convert_floatbv_typecast

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bvt boolbvt::convert_floatbv_typecast(const floatbv_typecast_exprt &expr)
{
  const exprt &op0=expr.op(); // number to convert
  const exprt &op1=expr.rounding_mode(); // rounding mode

  bvt bv0=convert_bv(op0);
  bvt bv1=convert_bv(op1);

  const typet &src_type=ns.follow(expr.op0().type());
  const typet &dest_type=ns.follow(expr.type());

  if(src_type==dest_type) // redundant type cast?
    return bv0;

  float_utilst float_utils(prop);

  float_utils.set_rounding_mode(convert_bv(op1));

  if(src_type.id()==ID_floatbv &&
     dest_type.id()==ID_floatbv)
  {
    float_utils.spec=ieee_float_spect(to_floatbv_type(src_type));
    return
      float_utils.conversion(
        bv0,
        ieee_float_spect(to_floatbv_type(dest_type)));
  }
  else if(src_type.id()==ID_signedbv &&
          dest_type.id()==ID_floatbv)
  {
    float_utils.spec=ieee_float_spect(to_floatbv_type(dest_type));
    return float_utils.from_signed_integer(bv0);
  }
  else if(src_type.id()==ID_unsignedbv &&
          dest_type.id()==ID_floatbv)
  {
    float_utils.spec=ieee_float_spect(to_floatbv_type(dest_type));
    return float_utils.from_unsigned_integer(bv0);
  }
  else if(src_type.id()==ID_floatbv &&
          dest_type.id()==ID_signedbv)
  {
    std::size_t dest_width=to_signedbv_type(dest_type).get_width();
    float_utils.spec=ieee_float_spect(to_floatbv_type(src_type));
    return float_utils.to_signed_integer(bv0, dest_width);
  }
  else if(src_type.id()==ID_floatbv &&
          dest_type.id()==ID_unsignedbv)
  {
    std::size_t dest_width=to_unsignedbv_type(dest_type).get_width();
    float_utils.spec=ieee_float_spect(to_floatbv_type(src_type));
    return float_utils.to_unsigned_integer(bv0, dest_width);
  }
  else
    return conversion_failed(expr);
}

/*******************************************************************\

Function: boolbvt::convert_floatbv_op

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bvt boolbvt::convert_floatbv_op(const exprt &expr)
{
  const exprt::operandst &operands=expr.operands();

  if(operands.size()!=3)
    throw "operator "+expr.id_string()+" takes three operands";

  const exprt &op0=expr.op0(); // first operand
  const exprt &op1=expr.op1(); // second operand
  const exprt &op2=expr.op2(); // rounding mode

  bvt bv0=convert_bv(op0);
  bvt bv1=convert_bv(op1);
  bvt bv2=convert_bv(op2);

  const typet &type=ns.follow(expr.type());

  if(op0.type()!=type || op1.type()!=type)
  {
    std::cerr << expr.pretty() << std::endl;
    throw "float op with mixed types";
  }

  float_utilst float_utils(prop);

  float_utils.set_rounding_mode(bv2);

  if(type.id()==ID_floatbv)
  {
    float_utils.spec=ieee_float_spect(to_floatbv_type(expr.type()));

    if(expr.id()==ID_floatbv_plus)
      return float_utils.add_sub(bv0, bv1, false);
    else if(expr.id()==ID_floatbv_minus)
      return float_utils.add_sub(bv0, bv1, true);
    else if(expr.id()==ID_floatbv_mult)
      return float_utils.mul(bv0, bv1);
    else if(expr.id()==ID_floatbv_div)
      return float_utils.div(bv0, bv1);
    else if(expr.id()==ID_floatbv_rem)
      return float_utils.rem(bv0, bv1);
    else
      throw "unexpected operator "+expr.id_string();
  }
  else if(type.id()==ID_vector || type.id()==ID_complex)
  {
    const typet &subtype=ns.follow(type.subtype());

    if(subtype.id()==ID_floatbv)
    {
      float_utils.spec=ieee_float_spect(to_floatbv_type(subtype));

      std::size_t width=boolbv_width(type);
      std::size_t sub_width=boolbv_width(subtype);

      if(sub_width==0 || width%sub_width!=0)
        throw "convert_floatbv_op: unexpected vector operand width";

      std::size_t size=width/sub_width;
      bvt bv;
      bv.resize(width);

      for(std::size_t i=0; i<size; i++)
      {
        bvt tmp_bv0, tmp_bv1, tmp_bv;

        tmp_bv0.assign(bv0.begin()+i*sub_width, bv0.begin()+(i+1)*sub_width);
        tmp_bv1.assign(bv1.begin()+i*sub_width, bv1.begin()+(i+1)*sub_width);

        if(expr.id()==ID_floatbv_plus)
          tmp_bv=float_utils.add_sub(tmp_bv0, tmp_bv1, false);
        else if(expr.id()==ID_floatbv_minus)
          tmp_bv=float_utils.add_sub(tmp_bv0, tmp_bv1, true);
        else if(expr.id()==ID_floatbv_mult)
          tmp_bv=float_utils.mul(tmp_bv0, tmp_bv1);
        else if(expr.id()==ID_floatbv_div)
          tmp_bv=float_utils.div(tmp_bv0, tmp_bv1);
        else
          assert(false);

        assert(tmp_bv.size()==sub_width);
        assert(i*sub_width+sub_width-1<bv.size());
        std::copy(tmp_bv.begin(), tmp_bv.end(), bv.begin()+i*sub_width);
      }

      return bv;
    }
    else
      return conversion_failed(expr);
  }
  else
    return conversion_failed(expr);
}
