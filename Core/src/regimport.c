
#define _LIBYOYO
#define _REGIMPORT_BUILTIN

#include "hc/regimport.hc"

static void Set_Registry_Value(HKEY hkey, wchar_t *name, void *val, int L, int Tp)
{
	long err = 0;
	if ( ERROR_SUCCESS != (err=RegSetValueExW(hkey,name,0,Tp,(LPBYTE)val,L)) )
		__Raise(YO_ERROR_IO,
		__Format(__yoTa("failed to set value '%S': error %08x",0),name,err));
}

static void Set_Registry_String(HKEY hkey, wchar_t *name, wchar_t *val, int L)
{
	if ( L < 0 ) L = (wcslen(val)+1)*2;
	Set_Registry_Value(hkey,name,val,L,RR_TYPE_SZ);
}

static void Split_Key_Name(wchar_t *subkey, HKEY *master, wchar_t **parent, wchar_t **name)
{
	int i;

	static wchar_t *str_vars[3] =
	{L"HKEY_CURRENT_USER\\",L"HKEY_LOCAL_MACHINE\\",L"HKEY_CLASSES_ROOT\\"};
	static HKEY hkey_vars[3] =
	{HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_CLASSES_ROOT};

	for ( i = 0; i < 3; ++i )
		if (  Str_Unicode_Starts_With(subkey,str_vars[i]) )
		{
			*name = subkey+wcslen(str_vars[i]);
			*master = hkey_vars[i];
			*parent = str_vars[i];
			break;
		}
}

static HKEY Create_Registry_Key(wchar_t *subkey)
{
	int err;
	HKEY ret = 0;
	HKEY master = 0;
	wchar_t *name = 0;
	wchar_t *parent = 0;

	Split_Key_Name(subkey,&master,&parent,&name);        

	if ( !master )
		__Raise(YO_ERROR_DOESNT_EXIST,
		__Format(__yoTa("registry key '%S' doesn't exist",0),subkey));

	if ( ERROR_SUCCESS != (err = RegCreateKeyExW(master,name,0,0,0,KEY_ALL_ACCESS,0,&ret,0)) )
		__Raise(YO_ERROR_IO,
		__Format(__yoTa("failed to create registry key '%S': error %08x",0),subkey,err));

	return ret;
}

static void Delete_Registry_Key(wchar_t *subkey)
{
	int err;
	HKEY master = 0;
	wchar_t *name = 0;
	wchar_t *parent = 0;

	Split_Key_Name(subkey,&master,&parent,&name);        

	if ( !master )
		__Raise(YO_ERROR_DOESNT_EXIST,
		__Format(__yoTa("registry key '%S' doesn't exist",0),subkey));

	if ( ERROR_SUCCESS != (err = RegDeleteKeyW(master,name)) )
		__Raise(YO_ERROR_IO,
		__Format(__yoTa("failed to delete registry key '%S': error %08x",0),subkey,err));
}

static int Registry_Key_Exists(wchar_t *subkey)
{
	int err;
	HKEY K = 0;
	HKEY master = 0;
	wchar_t *name = 0;
	wchar_t *parent = 0;

	Split_Key_Name(subkey,&master,&parent,&name);        

	if ( !master )
		__Raise(YO_ERROR_DOESNT_EXIST,
		__Format(__yoTa("registry key '%S' doesn't exist",0),subkey));

	if ( ERROR_SUCCESS != (err = RegOpenKeyExW(master,name,0,KEY_QUERY_VALUE,&K)) )
		if ( err == ERROR_FILE_NOT_FOUND )
			return 0;
		else
			__Raise(YO_ERROR_IO,
			__Format(__yoTa("failed to create registry key '%S': error %08x",0),subkey,err));

	RegCloseKey(K);
	return 1;  
}

wchar_t Import_Registry_Appfolder	[MAX_PATH+1] = {0};
wchar_t Import_Registry_Winsys		[MAX_PATH+1] = {0};
wchar_t Import_Registry_System32	[MAX_PATH+1] = {0};

wchar_t *Registry_Expand_Value(wchar_t *value)
{
	if ( Str_Unicode_Starts_With_Nocase(value,L"{%APPFOLDER%}") )
	{
		wchar_t *S = Str_Unicode_Concat(Import_Registry_Appfolder,value+13);
		//MessageBoxA(0,__Format("%S = > %S",value,S),"APPFOLDER",0);
		return S;
	}
	if ( Str_Unicode_Starts_With_Nocase(value,L"{%WINSYS%}") )
	{
		wchar_t *S = Str_Unicode_Concat(Import_Registry_Winsys,value+10);
		//MessageBoxA(0,__Format("%S = > %S",value,S),"WINSYS",0);
		return S;
	}
	if ( Str_Unicode_Starts_With_Nocase(value,L"{%SYSTEM32%}") )
	{
		wchar_t *S = Str_Unicode_Concat(Import_Registry_System32,value+10);
		//MessageBoxA(0,__Format("%S = > %S",value,S),"SYSTEM32",0);
		return S;
	}
	else
		return value;
}

int Import_Registry(char *text,char error[256])
{
	int ok = 0;
	__Auto_Release __Try_Except
	{
		REGISTRY_RECORD *r;
		int charset;
		int thold = 0;
		int i;

		charset = Detect_Registry_Charset(&text);

		GetWindowsDirectoryW(Import_Registry_Winsys,MAX_PATH);
		wcscpy(Import_Registry_System32,Import_Registry_Winsys);
		wcscat(Import_Registry_System32,L"\\System32");

		for ( thold = 0; __Purge(&thold) 
			&& 0 != (r = Next_Registry_Record(&text,charset)); ) 
		{
			//puts(Format_Registry_Record(r,RR_CHARSET_UTF8));
			if ( !r->attr ) lb_create_key:
			{
				HKEY hkey = Create_Registry_Key(r->key);

				if ( r->dfltval )
				{
					wchar_t *S = Registry_Expand_Value(r->dfltval);
					Set_Registry_String(hkey,L"",S,-1);
				}

				if ( r->valarr )
					for ( i = 0; i < r->valarr->count; ++i )
					{
						REGISTRY_VALUE *val = r->valarr->at[i];
						wchar_t *S = val->value;
						int len = val->len;
						if ( val->value && val->type == RR_TYPE_SZ )
						{
							S = Registry_Expand_Value(val->value);
							if ( S != val->value )
								len = (wcslen(val->value)+1)*2;
						}
						Set_Registry_Value(hkey,val->name,S,len,val->type);
					}
			}
			else if ( r->attr == RR_KEY_ATTR_DELETE )
			{
				Delete_Registry_Key(r->key);
			}
			else if ( r->attr == RR_KEY_ATTR_CONDIT )
			{
				if ( !Registry_Key_Exists(r->key) )
					goto lb_create_key;
			}
		}

		ok = 1;
	}
	__Except
	{
		strncpy(error,__Format_Error(),254);
	}

	return ok;
}
