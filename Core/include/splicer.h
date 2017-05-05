
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

extern void InitHeapAndSplicer(STB_INFO *info, void *base);
extern void *AllocExecutableBlock(int);
extern void FreeExecutableBlock(void*);
extern void *Splice5(void *f);
extern void *WrapWithJxThunk(void*);