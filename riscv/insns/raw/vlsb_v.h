if(p->enable_smart_mcpu && !p->is_vl_available)
{
  p->get_state()->raw = true;
  return true;
}
bool b1 = C_RS1;
bool b2 = C_RS2;
return (b1 || b2);
