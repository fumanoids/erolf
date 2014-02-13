
void test_all(void);
#include "mathOptions.h"

#define INIT_TEST \
	int test_ct = 0;\
	int test_success = 0;
#define END_TEST(e)\
	printf("%s: %i/%i succeeded test\n",e,test_success,test_ct);

#define TEST(x) \
{ \
++test_ct; \
if(x) { \
	++test_success; \
}\
else {\
	printf("%s failed in %s:%i\n",#x,__FILE__,__LINE__); \
}}




