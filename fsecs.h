#ifndef MM_FSECS_H
#define MM_FSECS_H

typedef void (*fsecs_test_funct)(void *);

void init_fsecs(void);
double fsecs(fsecs_test_funct f, void *argp);

#endif /* MM_FSECS_H */
