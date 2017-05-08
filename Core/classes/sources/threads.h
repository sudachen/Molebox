
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___80ab0127_8cdb_4670_9e1d_b6e2016324e4___
#define ___80ab0127_8cdb_4670_9e1d_b6e2016324e4___

#include "_specific.h"
#include "refcounted.h"
#include "ptr_rcc.h"

#define __lockon__(x) \
    if ( teggo::CRITICAL_SECTION_LOCKER __sync_locker_l = teggo::CRITICAL_SECTION_LOCKER(static_cast<CRITICAL_SECTION*>(x)) ) \
        goto TEGGO_LABEL(__LINE__); else TEGGO_LABEL(__LINE__):

#define __lock__ \
    if ( teggo::CRITICAL_SECTION_LOCKER __sync_locker_l = teggo::CRITICAL_SECTION_LOCKER(+teggo::GLOBAL_CRITICAL_SECTION()) ) \
        goto TEGGO_LABEL(__LINE__); else TEGGO_LABEL(__LINE__):

/*
uintptr_t _beginthreadex(
   void *security,
   unsigned stack_size,
   unsigned ( *start_address )( void * ),
   void *arglist,
   unsigned initflag,
   unsigned *thrdaddr
);

void _endthreadex(
   unsigned retval
);
*/

namespace teggo
{

    _TEGGO_EXPORTABLE void SelectSyncObject(CRITICAL_SECTION** _cs);

    struct GLOBAL_CRITICAL_SECTION
    {
        _TEGGO_EXPORTABLE CRITICAL_SECTION* _Select() const;
        operator CRITICAL_SECTION* () const { return _Select(); }
        //CRITICAL_SECTION *operator&() const { return _Select(); }
        CRITICAL_SECTION* operator+() const { return _Select(); }
    };

    struct Tlocker
    {
        mutable CRITICAL_SECTION* cs_;
        Tlocker() : cs_ (0) {};
        ~Tlocker()
        {
            if ( cs_ )
            {
                DeleteCriticalSection(cs_);
                HeapFree(GetProcessHeap(),0,cs_);
            }
        }
        CRITICAL_SECTION* _Select() const
        {
            if (!cs_) SelectSyncObject(&cs_);
            return cs_;
        }
        operator CRITICAL_SECTION* () const
        {
            return _Select();
        }
        CRITICAL_SECTION* operator&() const
        {
            return _Select();
        }
        void Lock()
        {
            EnterCriticalSection(_Select());
        }
        void Unlock()
        {
            LeaveCriticalSection(_Select());
        }
    };

    struct CRITICAL_SECTION_LOCKER
    {
        CRITICAL_SECTION* locker_;
        CRITICAL_SECTION_LOCKER(CRITICAL_SECTION* l) : locker_(l)
        { EnterCriticalSection(locker_); }
        ~CRITICAL_SECTION_LOCKER()
        { LeaveCriticalSection(locker_); }
        operator bool() const { return false; }
    };

    template <class T>
    struct TsafeHolder
    {
        T* instance_;
        T* Get()
        {
            if ( !instance_ )
                __lock__
            {
                if ( !(T* volatile&)instance_ )
                    instance_ = new T();
            }
            return instance_;
        }
        T* operator ->() { return Get(); }
    };

    struct Event : Refcounted
    {
        enum
        {
            WAIT_IS_ABORTED = 0,
            IS_RAISED       = 1,
            IS_INVALID      = 2,
        };

        HANDLE event_;

        Event() : event_(0)
        {
            event_ = CreateEvent(0,0,0,0);
        }
        ~Event()
        {
        }

        void Raise()
        {
            SetEvent(event_);
        }

        int Wait(int ms=-1)
        {
            unsigned u = WaitForSingleObject(event_,(DWORD)ms);
            if ( u == WAIT_OBJECT_0 ) return IS_RAISED;
            if ( u == WAIT_TIMEOUT ) return WAIT_IS_ABORTED;
            //if ( u == WAIT_ABANDONED )
            return IS_INVALID;
        }
    };

    struct Thread : Refcounted
    {
        Tlocker l_;
        int volatile status_;
        Event e1_; // notification
        HANDLE thread_;

        enum
        {
            PREPARING,
            STARTING,
            WORKING,
            STOPPING,
            STOPPED
        };

        Thread() : status_(PREPARING), thread_(0)
        {
        }

        ~Thread()
        {
            //KillThread();
        }

        bool IsWorking() const
        {
            return status_ == WORKING;
        }
        bool IsStopping() const
        {
            return status_ == STOPPING;
        }

        virtual void Execute() = 0;
        static unsigned __stdcall threadfunc_(void* f)
        {
            rcc_ptr<Thread> th = RccRefe((Thread*)f);
            th->status_ = WORKING;
            th->e1_.Raise(); // waiting by Run
            th->Execute();
            th->status_ = STOPPED;
            th->e1_.Raise(); // waiting by Stop
            //_endthreadex( 0 );
            return 0;
        }

        void Run()
        {
            __lockon__(l_)
            {
                unsigned long thid;
                status_ = STARTING;
                //thread_ = _beginthreadex(0,0,&threadfunc_,this,&thid);
                thread_ = CreateThread(0,0,(LPTHREAD_START_ROUTINE)&threadfunc_,this,0,&thid);
                do e1_.Wait(-1); // infinity
                while ( status_ == STARTING );
            }
        }

        bool Stop(int ms=-1)
        {
            __lockon__(l_)
            if ( IsWorking() )
            {
                status_ = STOPPING;
                e1_.Wait(ms);
                return status_ > STOPPING;
            }
            else
                return true;
        }

        void KillThread_()
        {
            if ( IsWorking() )
            {
                HANDLE t = thread_; thread_ = 0;
                TerminateThread((HANDLE)t,-1);
            }
        }

        void Kill(int ms=100)
        {
            __lockon__(l_)
            if ( thread_ )
                if ( !Stop(ms) )
                    KillThread_();
        }
    };

    typedef rcc_ptr<Thread> ThreadPtr;

    template <class T, class R>
    struct CoroutineT: Thread
    {
        typedef typename R (*subroutine_t)(typename T);
        typename T arg_;
        subroutine_t routine_;
        virtual void Execute()
        {
            routine_(arg_);
        }
        CoroutineT( typename subroutine_t routine, typename T arg ) : arg_(arg), routine_(routine)
        {
        }
    };

    template <class T, class R>
    ThreadPtr StartCoroutine( typename R (*subroutine)(typename T), typename T arg )
    {
        ThreadPtr thp = RccPtr((Thread*)new CoroutineT<T,R>(subroutine,arg));
        thp->Run();
        return thp;
    }

    #if defined _TEGGOINLINE || defined _TEGGO_THREADS_HERE

    #if defined _TEGGOINLINE
# define _TEGGO_THREADS_FAKE_INLINE CXX_FAKE_INLINE
    #else
# define _TEGGO_THREADS_FAKE_INLINE _TEGGO_EXPORTABLE
    #endif

    _TEGGO_THREADS_FAKE_INLINE
    void SelectSyncObject(CRITICAL_SECTION** _cs)
    {
        CRITICAL_SECTION* cs = (CRITICAL_SECTION*)HeapAlloc(GetProcessHeap(),0,sizeof(CRITICAL_SECTION));
        InitializeCriticalSection(cs);
        if ( 0 != InterlockedCompareExchange((long*)_cs,(long)cs,0) )
        {
            DeleteCriticalSection(cs);
            HeapFree(GetProcessHeap(),0,cs);
        }
    }

    _TEGGO_THREADS_FAKE_INLINE
    CRITICAL_SECTION* GLOBAL_CRITICAL_SECTION::_Select() const
    {
        static CRITICAL_SECTION* cs = 0;
        if ( !cs ) SelectSyncObject(&cs);
        return cs;
    }

    #endif

}

#endif /*___80ab0127_8cdb_4670_9e1d_b6e2016324e4___*/
