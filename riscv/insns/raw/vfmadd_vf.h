if(p->enable_smart_mcpu && !p->is_vl_available)
{
  p->get_state()->raw = true;
  return true;
}
VECTOR_SCALAR_VECTOR_FLOAT_CHECK_RAW();
