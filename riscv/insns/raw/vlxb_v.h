if(p->enable_smart_mcpu && !p->is_vl_available)
{
  p->get_state()->raw = true;
  return true;
}
if(p->enable_smart_mcpu && !p->is_vl_available)
  return true;
SCALAR_VECTOR_CHECK_RAW();
