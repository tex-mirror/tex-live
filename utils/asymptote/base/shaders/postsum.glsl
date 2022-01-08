layout(local_size_x=1) in;

uniform uint elements;

layout(binding=1, std430) buffer offsetBuffer
{
  uint offset[];
};

uint ceilquotient(uint a, uint b)
{
  return (a+b-1u)/b;
}

void main(void)
{
  uint id=gl_GlobalInvocationID.x;

  uint m=ceilquotient(elements,gl_NumWorkGroups.x);
  uint row=m*id;
  uint col=min(m,elements-row);
  uint stop=row+col-1u;

  uint curr=offset[row];
  for(uint i=row; i < stop; ++i) {
    uint next=offset[i+1u];
    curr += next;
    offset[i+1u]=curr;
  }
}
