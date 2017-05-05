/*

  LZSS python module
  there is simple & dirty LZ77 implementation

  (c)2008, Alexey Sudachen, alexey@sudachen.name

 */


#include <Python.h>

extern int lz77ss_compress(char *in_b, int in_b_len, unsigned char *out_b, int out_b_len);
extern int lz77ss_decompress(unsigned char *in_b, int in_b_len, char *out_b, int out_b_len);

static PyObject *lzss_compress(PyObject *_0,PyObject *args)
  {
    char *in_b,*prefix = 0;
    int   in_b_len,prefix_len = 0;
    if ( PyArg_ParseTuple(args,"s#|s#",&in_b,&in_b_len,&prefix,&prefix_len) )
      {
        int  out_b_len = in_b_len, out_i = 0;
        unsigned char *out_b = 0;
        PyObject *ret = 0;
        out_b = malloc(out_b_len+prefix_len);
        if ( prefix_len )
          memcpy(out_b,prefix,prefix_len);
        out_i = lz77ss_compress(in_b,in_b_len,out_b+prefix_len,out_b_len);
        if ( out_i > 0 )
          ret = PyString_FromStringAndSize(out_b,out_i+prefix_len);
        else
          PyErr_SetString(PyExc_Exception,"LZSS failed");
        free(out_b);
        return ret;
      }
    return 0;
  }


static PyObject *lzss_decompress(PyObject *_0,PyObject *args)
  {
    char *in_b;
    int   in_b_len;
    if ( PyArg_ParseTuple(args,"s#",&in_b,&in_b_len) )
      {
        int  out_b_len = 0, out_i = 0;
        unsigned char *out_b = 0;
        PyObject *ret = 0;

        out_b_len = (unsigned int)in_b[0]|((unsigned int)in_b[1]<<8)|
          ((unsigned int)in_b[2]<<16)|((unsigned int)in_b[3]<<24);

        out_b = malloc(out_b_len);
        out_i = lz77ss_decompress(in_b+4,in_b_len-4,out_b,out_b_len);
        if ( out_i > 0 )
          ret = PyString_FromStringAndSize(out_b,out_i);
        else
          PyErr_SetString(PyExc_Exception,"LZSS failed");
        free(out_b);
      }
    return 0;
  }

static PyMethodDef lzss_funcs[] =
  {
    {"compress",lzss_compress,METH_VARARGS,0},
    {"decompress",lzss_decompress,METH_VARARGS,0},
    {0,0,0,0},
  };

void init_lzss()
  {
    Py_InitModule("_lzss", lzss_funcs);
  }
