layout(local_size_x=1) in;

uniform uint elements;

layout(binding=1, std430) buffer offsetBuffer
{
  uint offset[];
};

void main(void)
{
  uint id=gl_GlobalInvocationID.x;

  uint m=elements/gl_NumWorkGroups.x;
  uint r=elements-m*gl_NumWorkGroups.x;
  uint row,stop;
  if(id < r) {
    row=m*id+id;
    stop=row+m+1;
  } else {
    row=m*id+r;
    stop=row+m;
  }

  uint Sum=offset[row];
  for(uint i=row+1u; i < stop; ++i) {
    Sum += offset[i];
    offset[i]=Sum;
  }
}
