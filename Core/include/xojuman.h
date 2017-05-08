
enum
{
    XOJUMAN_MAX_PACKAGE_COUNT = 100,
    XOJUMAN_MAX_PACKAGE_NAME_LEN = 16,
    XOJUMAN_MAX_PATH_LEN = 256,
};


typedef struct _XOJUMAN_INHERIT_PACKAGE
{
    void*  handle;
    INT64T offs;
    int    opts;
    unsigned char pwd[16];
    char   name[XOJUMAN_MAX_PACKAGE_NAME_LEN + 1];
} XOJUMAN_INHERIT_PACKAGE;

typedef struct _XOJUMAN_STARTUP_INFO
{
    int flags;

    void* core_handle;
    int core_size;
    byte_t orign[12];
    void* reserved_space;

    int subsystem;
    int master_id;
    int pkg_count;

    wchar_t container[XOJUMAN_MAX_PATH_LEN + 1];
    wchar_t appname[XOJUMAN_MAX_PATH_LEN + 1];
    wchar_t cmdline[XOJUMAN_MAX_PATH_LEN + 1];
    wchar_t regmask[XOJUMAN_MAX_PATH_LEN + 1];

    XOJUMAN_INHERIT_PACKAGE pkg[XOJUMAN_MAX_PACKAGE_COUNT];
} XOJUMAN_STARTUP_INFO;

