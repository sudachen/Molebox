
#include "_common.h"
#include "logger.h"
#include "vfs.h"

extern "C" void* Pict_From_BMP_Specific_NoX(void* bytes, int count, int format);
extern "C" void Execute_Splash_Thread(void*);

void SplashScreen()
{
	void* pict = 0;
	if (FioProxyPtr fp = VFS->GetProxy(L"_$splash$_.bmp", 0, true))
	{
		u32_t wasread;
		BufferT<byte_t> buffer(fp->Size());
		fp->Read(0, +buffer, buffer.Count(), &wasread);
		pict = Pict_From_BMP_Specific_NoX(+buffer, buffer.Count(), 0);
	}
	if (pict)
		Execute_Splash_Thread(pict);
}
