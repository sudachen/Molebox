
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"

teggo::XFILE Xo = xout;
teggo::XFILE Xoln = xoutln;

void SwitchXoToUtf8(bool useUtf8) 
  { 
    Xo = useUtf8?xout8:xout; 
    Xoln = useUtf8?xout8ln:xoutln; 
  }      
  
