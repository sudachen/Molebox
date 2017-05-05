
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___7aa75ac8_0ea9_466e_b151_d6eb6831e1fe___
#define ___7aa75ac8_0ea9_466e_b151_d6eb6831e1fe___

#define _XOr(a,L,S) (+xordata::XoredStringX<char>(L,S))
#define _XOrL(a,_L,_S)  L##a

namespace xordata
{

	extern "C" void* __cdecl _UnXOrS(int source, int count);
	extern "C" void  __cdecl _UnXOrSfree(void*);

	template < class tUUU >
	struct XoredStringX
	{
		tUUU* t_;
		XoredStringX(unsigned L, unsigned S) : t_((tUUU*)_UnXOrS(S, L* sizeof(tUUU))) {}
		~XoredStringX() {_UnXOrSfree(t_);}
		inline tUUU* operator +() const { return t_; }
		//inline operator tUUU *() const { return t_; }
	};
}

#endif /*___7aa75ac8_0ea9_466e_b151_d6eb6831e1fe___*/
