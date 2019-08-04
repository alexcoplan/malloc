#include <Foundation/Foundation.h>

int main(void)
{

  NSMutableArray *arr = [[NSMutableArray alloc] init];
  [arr addObject:@"hello"];
  [arr addObject:@"world"];
  NSLog(@"%@", arr);
  return 0;
}
