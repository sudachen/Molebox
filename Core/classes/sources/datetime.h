
/*

  Copyright (c) 2005-2010, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___3D83CE2B_7D9C_4942_8136_C7B7D0ACA231___
#define ___3D83CE2B_7D9C_4942_8136_C7B7D0ACA231___

#include "_specific.h"
#include "_crc32.h"
#include "string.h"
#include <time.h>

namespace teggo
{
  struct DateTime
    {
      enum DT_NOW { NOW };
      enum DT_UNKNOWN { UNKNOWN };
      enum DT_INFINITY { INFINITY };
      enum DT_RAW { RAW };
            
      enum { CC_BARIER = 0 };
      
      i32_t  millis;
      
      struct 
        {
          u32_t day:   5;
          u32_t month: 4;
          u32_t year:  22;
          u32_t cc:    1;
        } dmyc;
        
      unsigned Day()   const { return dmyc.day; }
      unsigned Month() const { return dmyc.month; }
      unsigned Year()  const { return dmyc.cc?CcYear():-CcYear();}
      signed   CcYear()const { return dmyc.year; }
      bool     AD()    const { return !!dmyc.cc; }
      bool     BC()    const { return !dmyc.cc; }   
      
      pwide_t Smon(bool shortform=true) const
        {                 
          static pwide_t m[12][2] = 
            { 
              {L"Jan",L"January"},
              {L"Feb",L"February"},
              {L"Mar",L"March"},
              {L"Apr",L"April"},
              {L"May",L"May"},
              {L"Jun",L"June"},
              {L"Jul",L"July"},
              {L"Aug",L"August"},
              {L"Sep",L"September"},
              {L"Oct",L"October"},
              {L"Nov",L"November"},
              {L"Dec",L"December"},
            };
                  
          if ( millis == -1 ) // is not valid date
            return shortform?L"Inv":L"Invalid";
            
          unsigned month = Month();
          REQUIRE(month>0&&month<=12);
          return m[month-1][shortform?0:1];
        }        
        
      pwide_t Lmon() const
        {
          return Smon(false);
        }
      
      DateTime &Day(unsigned d)    
        {                      
          REQUIRE(d>0&&d<=31);
          dmyc.day = d; return *this; 
        }
      
      DateTime &Month(unsigned m)  
        {
          REQUIRE(m>0&&m<=12); 
          dmyc.month = m; return *this; 
        }
      
      DateTime &Year(signed y)     
        { 
          y < 0 ? dmyc.year = -y, dmyc.cc = 0 : dmyc.year = y, dmyc.cc = 1; 
          return *this; 
        }
        
      DateTime &Time(unsigned h, unsigned m, unsigned s, unsigned u) 
        {
          millis = h*60*60*1000 + m*60*1000 + s*1000 + u;
          return *this; 
        }
      
      enum { USECONDS_COUNT = 1 };
      
      unsigned Hour() const   { return (millis/(60*60*1000*USECONDS_COUNT))%24; }
      unsigned Minute() const { return (millis/(60*1000*USECONDS_COUNT))%60; }
      unsigned Second() const { return (millis/(1000*USECONDS_COUNT))%60; }
      unsigned Millis() const { return (millis/USECONDS_COUNT)%1000; }
      
      DateTime(DT_RAW) {}
      DateTime(DT_UNKNOWN) { millis = -1; memset(&dmyc,0xff,sizeof(dmyc)); dmyc.cc = 0; }
      DateTime(DT_INFINITY) { millis = -1; memset(&dmyc,0xff,sizeof(dmyc)); }
        
      bool Unknown() { return millis == -1 && !dmyc.cc; }
      bool Infinity() { return millis == -1 && dmyc.cc; }
        
      DateTime() 
        {
          memset(&dmyc,0,sizeof(dmyc));
          millis = 0;
        }
        
      DateTime(DT_NOW)
        {
          InitWholeFromPOSIXtime(time(0));
        }
        
      DateTime ShiftYear(int y)
        {                    
          if ( millis == -1 )
            return *this;
          DateTime d = *this;
          d.Year(d.Year()+y);
          return d;
        }
        
      void InitWholeFromPOSIXtime(__time64_t t)
        {
          if ( tm *xtm = _localtime64(&t) )
            {
              tm tm = *xtm;
              Year(tm.tm_year+1900);
              Month(tm.tm_mon+1);
              Day(tm.tm_mday);
              Time(tm.tm_hour,tm.tm_min,tm.tm_sec,0);
            }
          else
            *this = UNKNOWN;
        }
        
      static DateTime FromPOSIXtime(__time64_t t)
        {
          DateTime dt(RAW);
          dt.InitWholeFromPOSIXtime(t);
          return dt;
        }
        
      template < class tC > 
      static DateTime FromCstring(const tC *cStr)
        {
          DateTime dt(RAW);
          dt.InitWholeFromCstr(cStr);
          return dt;
        }
        
      void InitWholeFromCstr(const wchar_t *cStr) 
        { 
          InitWholeFromCstr(+StringA(cStr)); 
        }
        
      void InitWholeFromCstr(const char *cStr)
        {
          unsigned day,mon,h,m,s,u;
          signed year;
          char b,c;
          
          // M$VC swscanf isn't able to use constant strings
          sscanf(cStr,"%d-%d-%d %c%c %d:%d:%d %d",
            &day,&mon,&year,&b,&c,&h,&m,&s,&u);
          
          if ( b == 'b' || b == 'B' ) year = -year;
          Day(day).Month(mon).Year(year).Time(h,m,s,u);
        }
        
      StringW ToString() const
        {
          return _S*L"%02d-%02d-%04d %s %02d:%02d:%02d %03d"
            %Day()%Month()%CcYear()
            %(BC()?L"BC":L"AD")
            %Hour()%Minute()%Second()%Millis();
        }
        
      __time64_t PosixTime()
        { 
          tm tm;
          tm.tm_year = Year()-1900;
          tm.tm_mon = Month()-1;
          tm.tm_mday = Day();
          tm.tm_hour = Hour();
          tm.tm_min = Minute();
          tm.tm_sec = Second();
          return _mktime64(&tm);
        }
    };
}

#endif //___3D83CE2B_7D9C_4942_8136_C7B7D0ACA231___
