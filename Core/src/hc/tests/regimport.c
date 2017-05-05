
#define _LIBYOYO
#define _REGIMPORT_BUILTIN

#include "../yoyo.hc"
#include "../regimport.hc"
#include "../prog.hc"
#include "../file.hc"

void usage()
  {
    puts("use: regimport file.reg");
  }

int main(int argc, char **argv)
  {
    Prog_Init(argc,argv,"?|h",PROG_EXIT_ON_ERROR);
    
    //__Try_Exit(argv[0])
      {
        YO_BUFFER *bf;
        REGISTRY_RECORD *r;
        char *text;
        int charset;
        int thold = 0;
        
        if ( Prog_Arguments_Count() < 1 )
          {
            usage();
            exit(-1);
          }
        
        __Auto_Ptr(bf) bf = Oj_Read_All(File_Open(Prog_Argument(0),"rt"));
        
        Buffer_Fill_Append(bf,0,2);
        text = bf->at;
        charset = Detect_Registry_Charset(&text);
        
        for ( thold = 0; __Purge(&thold) && 0 != (r = Next_Registry_Record(&text,charset)); ) 
          {
            puts(Format_Registry_Record(r,RR_CHARSET_UTF8));
          }
        
      }
  }

