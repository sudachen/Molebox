

#ifndef C_once_E7FB0186_312A_4844_9FA2_11C1CF90475A
#define C_once_E7FB0186_312A_4844_9FA2_11C1CF90475A

#include "string.hc"
#include "buffer.hc"

enum RR_CHARSET
  {
    RR_CHARSET_ONE_BYTE_LOCALE = 0,
    RR_CHARSET_UNICODE = 1,
    RR_CHARSET_UTF8 = 2,
  };

enum RR_KEY_ATTRIBUTE
  {
    RR_KEY_ATTR_DELETE = 1,
    RR_KEY_ATTR_CONDIT = 2,
  };

enum RR_TYPE
  {
    RR_TYPE_SZ = 1,
    RR_TYPE_BINARY = 3,
    RR_TYPE_DWORD = 4,
  };

typedef struct _REGISTRY_VALUE
  {
    int len;
    int type;
    wchar_t *name;
    wchar_t *value;
  } REGISTRY_VALUE;

#ifdef _REGIMPORT_BUILTIN
static void REGISTRY_VALUE_Destruct(REGISTRY_VALUE *irv)
  {
    free(irv->name);
    free(irv->value);
    __Destruct(irv);
  }

static void *REGISTRY_VALUE_Init(
  __Acquire wchar_t *name, __Acquire void *val, int len, int type)
  {
    REGISTRY_VALUE *irv = __Object_Dtor(sizeof(REGISTRY_VALUE),REGISTRY_VALUE_Destruct);
    irv->value = val;
    irv->name = name;
    irv->type = type;
    irv->len  = len;
    return irv;
  }
#endif

typedef struct _REGISTRY_RECORD
  {
    int attr;
    wchar_t *key;
    wchar_t *dfltval;
    YO_ARRAY *valarr;
  } REGISTRY_RECORD;

#ifdef _REGIMPORT_BUILTIN
static void REGISTRY_RECORD_Destruct(REGISTRY_RECORD *irr)
  {
    free(irr->key);
    free(irr->dfltval);
    __Unrefe(irr->valarr);
    __Destruct(irr);
  }
#endif

void *REGISTRY_RECORD_Init(wchar_t *key, int attr)
#ifdef _REGIMPORT_BUILTIN
  {
    REGISTRY_RECORD *irr = __Object_Dtor(sizeof(REGISTRY_RECORD),REGISTRY_RECORD_Destruct);
    irr->key = Str_Unicode_Copy_Npl(key,-1);
    irr->attr = attr;
    irr->valarr = __Refe(Array_Refs());
    return irr;
  }
#endif
  ;
  
#ifdef _REGIMPORT_BUILTIN

static wchar_t RR_Get_Char(char **text,int charset)
  {
    if ( charset == RR_CHARSET_UTF8 )
      {
        if ( !**text ) 
          return 0;
        else
          {
            return Utf8_Get_Wide(text);
          }
      }
    else if ( charset == RR_CHARSET_UNICODE )
      {
        byte_t **S = (byte_t**)text;
        if ( !**S && !(*S)[1] ) 
          return 0;
        else 
          {
            wchar_t R =  (wchar_t)**S|((wchar_t)(*S)[1]<<8);
            *S += 2;
            return R;  
          }
      }
    else /* assume RR_CHARSET_ONE_BYTE_LOCALE */
      {
        if ( !**text ) 
          return 0;
        else
          {
            char c = *(*text)++;
            //return Str_Locale_To_Unicode_One_Char(c);
            return c;
          }
      }
  }

static wchar_t RR_Peek_Char(char **text, int charset)
  {
    char *q = *text;
    return RR_Get_Char(&q,charset);
  }

static int RR_Skip_Char(char **text, int ch, int charset)
  {
    char *q = *text;
    if ( RR_Get_Char(&q,charset) == ch )
      {
        *text = q;
        return ch;
      }
    return 0;
  }

static void RR_Skip_Line(char **text, int charset)
  {
    char *q = *text;
    while ( RR_Get_Char(&q,charset) != '\n' )
      {
        *text = q;
      }
  }

static void RR_Skip_Spaces(char **text, int charset)
  {
    char *q = *text;
    for(;;)
      {
        wchar_t c = RR_Get_Char(&q,charset);
        if ( iswspace(c) && c != '\n' )
          *text = q;
        else if ( c == '\\' )
          {
            RR_Skip_Char(&q,'\r',charset);
            if ( RR_Get_Char(&q,charset) != '\n' )
              __Raise(YO_ERROR_ILLFORMED,0);
            *text = q;
          }
        else
          break;
      }
  }

static int RR_Skip_Nocase(char **text, wchar_t *pat, int charset)
  {
    char *q = *text;
    
    for (;*pat;++pat)
      {
        wchar_t c = RR_Get_Char(&q,charset);
        if ( towupper(*pat) !=  towupper(c) )
          return 0;
      }
    
    *text = q;
    return 1;
  }

static REGISTRY_RECORD *RR_Fetch_Key_Info(char **text, int charset)
  {
    REGISTRY_RECORD *irr = 0;
    wchar_t kname[256]; 
    int attr = 0;
    int kn = 0;
    wchar_t c = RR_Get_Char(text,charset);
    
    if ( c != '[' ) __Raise(YO_ERROR_ILLFORMED,0);
    
    c = RR_Peek_Char(text,charset);
    if ( c == '-' ) 
      {
        attr = RR_KEY_ATTR_DELETE;
        RR_Get_Char(text,charset);
      }
    else if ( c == '?' )
      {
        attr = RR_KEY_ATTR_CONDIT;
        RR_Get_Char(text,charset);
      }
      
    while ( (c = RR_Get_Char(text,charset)) )
      {
        if ( !c || c == ']' || c == '\n' )
          {
            if ( c != ']' ) return 0;
            break;
          }
        else
          {
            if ( kn == sizeof(kname)/sizeof(wchar_t)-1 ) 
              { 
                RR_Skip_Line(text,charset);
                break;
              }
            else kname[kn++] = c;
          }
      }
    kname[kn] = 0;
    return REGISTRY_RECORD_Init(kname,attr);
  }

static wchar_t *RR_Fetch_String(char **text, int *len, int charset)
  {
    wchar_t *ret = 0;

    if ( RR_Get_Char(text,charset) != '"' )
      __Raise(YO_ERROR_ILLFORMED,0);
    
    __Auto_Ptr(ret)
      {
        wchar_t c;
        int i=0;
        
        while ( (c = RR_Get_Char(text,charset)) )
          {
            ret = __Resize(ret,(i+1)*sizeof(wchar_t),0);
            switch (c)
              {
                case '\\':
                  if ( (c = RR_Get_Char(text,charset)) )
                    {
                      switch (c)
                        {
                          case '\\': case '"':
                            ret[i++] = c;
                            break;
                          case 'n': 
                            ret[i++] = '\n';
                            break;
                          case '0':
                            ret[i++] = 0;
                            break;
                          default:
                            __Raise(YO_ERROR_ILLFORMED,0);
                        }
                    }
                  break;
            
                case '"':
                  ret[i++] = 0;
                  goto end_of_line;
                  
                default:
                  if ( c == '\n' ) 
                    __Raise(YO_ERROR_ILLFORMED,0);
                  
                  ret[i++] = c;
              }
          }
      
      end_of_line:
          
        if ( len ) *len = (i+1)*sizeof(wchar_t);
      }
      
    return ret;
  }

static void RR_Fetch_DfltValue(char **text, REGISTRY_RECORD *irr, int charset)
  {
    if ( RR_Get_Char(text,charset) != '@' )
      __Raise(YO_ERROR_ILLFORMED,0);
    RR_Skip_Spaces(text,charset);
    if ( RR_Get_Char(text,charset) != '=' )
      __Raise(YO_ERROR_ILLFORMED,0);
    RR_Skip_Spaces(text,charset);
    free(irr->dfltval);
    irr->dfltval = __Retain(RR_Fetch_String(text,0,charset));
  }

static uint_t RR_Fetch_Hex_Dword_(char **text, int charset)
  {
    uint_t value = 0;
    wchar_t c0;
    wchar_t c1;
    char *q = *text;
    while ( (c0 = RR_Get_Char(&q,charset)) )
      {
        if ( iswxdigit(c0) )
          {
            if ( !(c1 = RR_Get_Char(&q,charset)) || !iswxdigit(c1) )
              __Raise(YO_ERROR_ILLFORMED,0);
            __Gogo
              {
                char cc[] = { c0, c1, 0 };
                value = (value << 8) | Str_Unhex_Byte(cc,0,0);
                *text = q;
              }
          }
        else break;
      }
    return value;
  }

static void *RR_Fetch_Hex_Dword(char **text, int *data_len, int charset)
  {
    uint_t value = RR_Fetch_Hex_Dword_(text,charset);
    *data_len = 4;
    return __Memcopy(&value,4);
  }

static void *RR_Fetch_Hex_Sequence(char **text, int *data_len, int *data_type, int charset)
  {
    byte_t *ret = 0;
    *data_len = 0;
    
    __Auto_Ptr(ret)
      {
        if ( RR_Skip_Char(text,'(',charset) )
          {
            *data_type = RR_Fetch_Hex_Dword_(text,charset);
            if ( RR_Get_Char(text,charset) != ')')
              __Raise(YO_ERROR_ILLFORMED,0);
          }
        else
          *data_type = RR_TYPE_BINARY;
        
        if ( !RR_Skip_Char(text,':',charset) )
          __Raise(YO_ERROR_ILLFORMED,0);
        
        RR_Skip_Spaces(text,charset);
        
        do
          {
            uint_t value = RR_Fetch_Hex_Dword_(text,charset);
            ret = __Resize(ret,*data_len+1,0);
            ret[(*data_len)++] = (byte_t)value;
            
            RR_Skip_Spaces(text,charset);
            if ( !RR_Skip_Char(text,',',charset) )
              break;
            RR_Skip_Spaces(text,charset);
          }
        while( RR_Peek_Char(text,charset) != '\n' );
      }
    
    return ret;
        
  }

static void RR_Fetch_Value(char **text, REGISTRY_RECORD *irr, int charset)
  {
    __Auto_Release
      {
        wchar_t *name;
        wchar_t c;
        void   *value;
        int    data_len;
        int    data_type;
    
        name = RR_Fetch_String(text,0,charset);
    
        RR_Skip_Spaces(text,charset);
        if ( RR_Get_Char(text,charset) != '=' )
          __Raise(YO_ERROR_ILLFORMED,0);
        RR_Skip_Spaces(text,charset);
    
        c = RR_Peek_Char(text,charset);
    
        if ( c == '"' )
          {
            data_type = RR_TYPE_SZ;
            data_len = 0;
            value = RR_Fetch_String(text,&data_len,charset);
          }
    
        else if ( RR_Skip_Nocase(text,L"dword:",charset) )
          {
            data_type = RR_TYPE_DWORD;
            data_len = 4;
            RR_Skip_Spaces(text,charset);
            value = RR_Fetch_Hex_Dword(text,&data_len,charset);
          }
    
        else if ( RR_Skip_Nocase(text,L"hex",charset) )
          {
            data_type = RR_TYPE_DWORD;
            data_len = 0;
            value = RR_Fetch_Hex_Sequence(text,&data_len,&data_type,charset);
          }
    
        if ( value )
          {
            if ( name && name[0] )
              Oj_Push(irr->valarr,
                __Refe(
                  REGISTRY_VALUE_Init(__Retain(name), __Retain(value), 
                                      data_len, data_type)));
            else if ( data_type == RR_TYPE_SZ )
              {
                free(irr->dfltval);
                irr->dfltval = __Retain(value);
              }
            else
              __Raise(YO_ERROR_ILLFORMED,0);
          }
      }
  }

#endif

REGISTRY_RECORD *Next_Registry_Record(char **text, int charset)
#ifdef _REGIMPORT_BUILTIN
  {
    wchar_t c;
    REGISTRY_RECORD *irr = 0;
    __Auto_Ptr(irr)
      {
        RR_Skip_Spaces(text,charset);
        while ( !!(c = RR_Peek_Char(text,charset)) ) 
          {
            switch (c)
              {
                case '\n':
                  RR_Get_Char(text,charset);
                  break;
                case '[':
                  if ( irr ) goto l_end_key;
                  irr = RR_Fetch_Key_Info(text,charset);
                  break;
                case '@':
                  if ( !irr ) RR_Skip_Line(text,charset);
                  else RR_Fetch_DfltValue(text,irr,charset);
                  break;
                case '"':
                  if ( !irr ) RR_Skip_Line(text,charset);
                  else RR_Fetch_Value(text,irr,charset);
                  break;
                case ';': 
                default:
                  RR_Skip_Line(text,charset);
                  break;
              }
            RR_Skip_Spaces(text,charset);
          }
        l_end_key:;
      }
    return irr;
  }
#endif
  ;

int Detect_Registry_Charset(char **text)
#ifdef _REGIMPORT_BUILTIN
  {
    int bom = Str_Find_BOM(*text);
    
    if ( bom == YO_BOM_UTF16_LE )
      return RR_CHARSET_UNICODE;
  
  #ifdef __windoze
    if ( !bom )
      return RR_CHARSET_ONE_BYTE_LOCALE;
  #endif  
     
    return RR_CHARSET_UTF8;
  }
#endif
  ;

void Format_Registry_Record_Quote_String(YO_BUFFER *ob, int k, int len)
#ifdef _REGIMPORT_BUILTIN
  {
    enum { wSz = sizeof(wchar_t) };
    int j;
    for ( j = 0; j < len/wSz; ++j )
      {
        wchar_t *p = (wchar_t*)(ob->at+k);
        if ( !p[j] || p[j] == '\\' || p[j] == '\n' )
          {
            if ( !p[j] ) p[j] = '0';
            else if ( p[j] == '\n' ) p[j] = 'n';
            Buffer_Insert(ob,k+j*wSz,L"\\",1*wSz);
            k += wSz;
          }
      }
  }
#endif
;
  
char *Format_Registry_Record(REGISTRY_RECORD *irr, int charset)
#ifdef _REGIMPORT_BUILTIN
  {
    enum { wSz = sizeof(wchar_t) };
    char *ret = 0;
    
    __Auto_Ptr(ret)
      {
        int i;
        YO_BUFFER *ob = Buffer_Init(0);
        
        Buffer_Append(ob,L"[",wSz);
        Buffer_Append(ob,irr->key,wcslen(irr->key)*wSz);
        Buffer_Append(ob,L"]\n",2*wSz);
        
        if ( irr->dfltval )
          {
            int k,len;
            len = wcslen(irr->dfltval)*wSz;
            Buffer_Append(ob,L"@=\"",3*wSz);
            k = ob->count;
            Buffer_Append(ob,irr->dfltval,len);
            Buffer_Append(ob,L"\"\n",2*wSz);
            Format_Registry_Record_Quote_String(ob,k,len);
          }
        
        for ( i = 0; i < Array_Count(irr->valarr); ++i )
          {
            REGISTRY_VALUE *val = Array_At(irr->valarr,i);
            Buffer_Append(ob,L"\"",1*wSz);
            Buffer_Append(ob,val->name,wcslen(val->name)*wSz);
            
            if ( val->type == RR_TYPE_SZ )
              {
                int j,k;
                Buffer_Append(ob,L"\"=\"",3*wSz);
                k = ob->count;
                Buffer_Append(ob,val->value,val->len);
                Buffer_Append(ob,L"\"",1*wSz);
                Format_Registry_Record_Quote_String(ob,k,val->len);
              }
            else if ( val->type == RR_TYPE_DWORD )
              {
                int i;
                wchar_t hex[8] = {0};
                uint_t value = *(uint_t*)val->value;
                Buffer_Append(ob,L"\"=dword:",8*wSz);
                for ( i = 3; i >=0 ; --i )
                  {
                    char c[3];
                    Str_Hex_Byte((value>>(i*8)),0,&c);
                    hex[(3-i)*2] = c[0];
                    hex[(3-i)*2+1] = c[1];
                  }
                Buffer_Append(ob,hex,8*wSz);
              }
            else if ( val->type == RR_TYPE_BINARY )
              {
                int i;
                Buffer_Append(ob,L"\"=hex:",6*wSz);
                for ( i = 0; i < val->len ; ++i )
                  {
                    byte_t value = ((byte_t*)val->value)[i];
                    char c[3] = {0};
                    wchar_t hex[2];
                    Str_Hex_Byte(value,0,&c);
                    if ( i != 0 ) Buffer_Append(ob,L",",wSz);
                    hex[0] = c[0];
                    hex[1] = c[1];
                    Buffer_Append(ob,hex,2*wSz);
                  }
              }
            
            Buffer_Append(ob,L"\n",1*wSz);
          }
        
        Buffer_Append(ob,L"\0",1*wSz);

        if ( charset == RR_CHARSET_UTF8 )
          ret = Str_Unicode_To_Utf8((wchar_t*)ob->at);
        else if ( charset == RR_CHARSET_UNICODE )
          ret = __Memcopy(ob->at,ob->count);
        //else if ( charset == RR_CHRSET_ONE_BYTE_LOCALE )
        //  ret = Str_Unicode_To_Locale((wchar_t*)ob->at);
        else
          __Raise(YO_ERROR_INVALID_PARAM,0);
      }
      
    return ret;  
  }
#endif
  ;
  
#endif /*C_once_E7FB0186_312A_4844_9FA2_11C1CF90475A*/


