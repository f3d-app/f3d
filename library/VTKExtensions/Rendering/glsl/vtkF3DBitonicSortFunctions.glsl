ivec2 disperse(uint h)
{
  uint t = gl_GlobalInvocationID.x;
  uint q = (t / h) * 2 * h;

  uint x = q + t % h;
  uint y = q + t % h + h;

  return ivec2(x, y);
}

ivec2 flip(uint h)
{
  uint t = gl_GlobalInvocationID.x;
  uint q = (t / h) * 2 * h;

  uint x = q + t % h;
  uint y = q + 2 * h - (t % h) - 1;

  return ivec2(x, y);
}

void swap_key(inout KeyType key1, inout KeyType key2)
{
  KeyType tmp = key1;
  key1 = key2;
  key2 = tmp;
}

void swap_value(inout ValueType value1, inout ValueType value2)
{
  ValueType tmp = value1;
  value1 = value2;
  value2 = tmp;
}

void compare_and_swap(ivec2 idx)
{
  if (key[idx.x] > key[idx.y] && idx.y < count)
  {
    swap_key(key[idx.x], key[idx.y]);
    swap_value(value[idx.x], value[idx.y]);
  }
}
