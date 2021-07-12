if(p->enable_smart_mcpu && !p->is_vl_available)
{
  p->get_state()->raw = true;
  return true;
}
SCALAR_DEST_VECTOR_UNSIGNED_CHECK_RAW();
