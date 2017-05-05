
/*

  Copyright (c) 2005-2009, Alexey Sudachen, DesaNova Ltda.

*/

#ifndef ___75051c36_9510_4859_a9c9_437897b53e13___
#define ___75051c36_9510_4859_a9c9_437897b53e13___

#include "XorS.h"

template <class T>
void Delete(T*); // notimplemented

template <class T, class U>
struct HandledObjectRecord
{
	T* o;
	U state;
	u32_t r0, r1;
};

template <class T, class U>
struct HandledObjectsArray
{
	typedef HandledObjectRecord<T, U> R_t;
	typedef FlatmapT< HANDLE , HandledObjectRecord<T, U> > fmap_t;
	enum { RECORDS_POOL_SIZE = 64 * 1024 };
	enum { RECORDS_POOL_PAGE = 8 * 1024 };
	enum { INC_RECORDS_COUNT = RECORDS_POOL_PAGE / sizeof(R_t) };
	enum { MAX_RECORDS_COUNT = RECORDS_POOL_SIZE / sizeof(R_t) };
	R_t* records_;
	fmap_t fmap_;
	unsigned count_;
	unsigned capacity_;
	Tlocker l_;

	HandledObjectsArray()
	{
		records_ = (R_t*)VirtualAlloc(0, RECORDS_POOL_SIZE, MEM_RESERVE, PAGE_READWRITE);
		count_ = 0;
		capacity_ = 0;
	}

	struct R
	{
		T* p;
		bool f;
		U* s;
		T* operator->() { return p; }
		operator bool() { return f; }
		U& State() { return *s; }
		R(T* p = 0, bool f = false, U* s = 0) : p(p), f(f), s(s) {}
		T* operator+() { return p; }
	};

	HANDLE PutHandle(T* o, HANDLE handle)
	{
		__lockon__(l_)
		{
			REQUIRE(o != 0);
			R_t t;
			memset(&t, 0, sizeof(t));
			t.o = o;
			fmap_.Put(handle, t);
			return handle;
		}
	}

	HANDLE Put(T* o)
	{
		__lockon__(l_)
		{
			REQUIRE(o != 0);
			R_t* r = 0;
			for (int i = 0; i < count_; ++i)
			{
				if (!records_[i].o)
					r = records_ + i;
			}
			if (!r && capacity_ > count_)
			{
				r = records_ + count_;
				++count_;
			}
			if (!r && capacity_ < MAX_RECORDS_COUNT)
			{
				r = records_ + count_;
				VirtualAlloc(records_ + capacity_, RECORDS_POOL_PAGE, MEM_COMMIT, PAGE_READWRITE);
				capacity_ += INC_RECORDS_COUNT;
				++count_;
			}
			if (!r)
			{
				MessageBoxA(0,
				            _XOr("Not enough records for new virtual handle", 0, 0),
				            _XOr("fatal error", 0, 0), 0);
				TerminateProcess((HANDLE) - 1, -1);
			}
			r->o = o;
			memset(&r->state, 0, sizeof(U));
			return (HANDLE)r;
		}
	}

	R_t* _Get(HANDLE h)
	{
		if (h != 0)
		{
			R_t* r = (R_t*)((u32_t)h&~15);
			if (r >= records_ && r < records_ + count_)
				return r;
		}
		return 0;
	}

	R Get(HANDLE h)
	{
		__lockon__(l_)
		{
			if (R_t* r = _Get(h))
			{
				return R(r->o, 1, &r->state);
			}
			if (R_t* r = fmap_.Get(h))
			{
				return R(r->o, 1, &r->state);
			}
			return R(0, 0, 0);
		}
	}

	void Close(HANDLE h)
	{
		__lockon__(l_)
		{
			if (R_t* r = _Get(h))
			{
				if (r->o) Delete(r->o);
				r->o = 0;
			}
			if (R_t* r = fmap_.Get(h))
			{
				if (r->o) Delete(r->o);
				fmap_.Erase(h);
			}
		}
	}

	void SetState(HANDLE h, U state)
	{
		__lockon__(l_)
		{
			if (R_t* r = _Get(h))
			{
				r->state = state;
			}
			if (R_t* r = fmap_.Get(h))
			{
				r->state = state;
			}
		}
	}

	void GetState(HANDLE h, U& state)
	{
		__lockon__(l_)
		{
			if (R_t* r = _Get(h))
			{
				state = r->state;
			}
			if (R_t* r = fmap_.Get(h))
			{
				state = r->state;
			}
		}
	}

	bool NextRecord(int* i, HANDLE* h, R* o)
	{
		__lockon__(l_)
		{
			while (*i >= 0 && *i < count_)
				if (R_t* r = records_ + (*i)++)
					if (r->o)
						return *h = (HANDLE)r, *o = R(r->o, 1, &r->state), true;
			return false;
		}
	}

};

#endif /*___75051c36_9510_4859_a9c9_437897b53e13___*/
