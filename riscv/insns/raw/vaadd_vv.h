if(p->enable_smart_mcpu && !p->is_vl_available)
{
  p->get_state()->raw = true;
  return true;
}
// vaadd.vv vd, vs2, vs1
VECTOR_VECTOR_CHECK_RAW();
