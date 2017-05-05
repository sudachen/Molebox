
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

void _cdecl freeargv(wchar_t **vector)
  {
    wchar_t **scan;
    if (vector != NULL)
      {
        for (scan = vector; *scan != NULL; scan++) free (*scan);
        free (vector);
      }
  }

#define ISBLANK(a) ((a) == ' ' || (a) == '\t')
wchar_t ** __cdecl buildargv(wchar_t *input, int *_argc)
  {

    enum {EOS=0,INITIAL_MAXARGC=128};

    wchar_t *arg;
    wchar_t *copybuf;
    int dquote = 0;
    int lquote = 0;
    int argc = 0;
    int maxargc = 0;
    wchar_t **argv = NULL;
    wchar_t **nargv;

    if (input != NULL)
      {
        copybuf = (wchar_t *) _alloca ((wcslen (input) + 1)*sizeof(wchar_t));
        /* Is a do{}while to always execute the loop once.  Always return an
           argv, even for null strings.  See NOTES above, test case below. */
        do
          {
            /* Pick off argv[argc] */
            while ( ISBLANK(*input) )
              {
                input++;
              }
            if ((maxargc == 0) || (argc >= (maxargc - 1)))
              {
                /* argv needs initialization, or expansion */
                if (argv == NULL)
                  {
                    maxargc = INITIAL_MAXARGC;
                    nargv = (wchar_t **) malloc (maxargc * sizeof (wchar_t *));
                  }
                else
                  {
                    maxargc *= 2;
                    nargv = (wchar_t **) realloc (argv, maxargc * sizeof (wchar_t *));
                  }
                if (nargv == NULL)
                  {
                    if (argv != NULL)
                      {
                        freeargv (argv);
                        argv = NULL;
                      }
                    break;
                  }
                argv = nargv;
                argv[argc] = NULL;
              }
            /* Begin scanning arg */
            arg = copybuf;
            while (*input != EOS)
              {
                if (ISBLANK (*input) && !dquote && !lquote)
                  {
                    break;
                  }
                else
                  {
                    if (0) ;
                    else if (dquote)
                      {
                        if (*input == '"') 
                          {
                            if ( input[1] == '"' )
                              *arg++ = *input++;
                            dquote = 0;
                          }
                        else *arg++ = *input;
                      }
                    else if (lquote)
                      {
                        if (*input == '>') lquote = 0;
                        *arg++ = *input;
                      }
                    else // !dquote && !lquote
                      {
                        if (0) ;
                        else if (*input == '"') dquote = 1;
                        else 
                          {
                            if (*input == '<') lquote = 1;
                            *arg++ = *input;
                          }
                      }
                    input++;
                  }
              }
            *arg = EOS;
            argv[argc] = _wcsdup (copybuf);
            if (argv[argc] == NULL)
              {
                freeargv (argv);
                argv = NULL;
                break;
              }
            argc++;
            argv[argc] = NULL;

            while (ISBLANK (*input)) ++input;
          }
        while (*input != EOS);
      }
    *_argc = argc;
    return (argv);
  }
