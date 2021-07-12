if(p->enable_smart_mcpu && !p->is_vl_available)
{
  p->get_state()->raw = true;
  return true;
}
VECTOR_VECTOR_UNSIGNED_CHECK_RAW();
