
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___15724387_86d6_46a7_8d40_ca2bf44700d6___
#define ___15724387_86d6_46a7_8d40_ca2bf44700d6___

#include "_specific.h"
#include "string.h"
#include "streams.h"
#include "refcounted.h"
#include "xdom.h"

namespace teggo {

  _TEGGO_EXPORTABLE TEGGOt_E  CXX_STDCALL DEFparse(DataStream *ds, XMLdocumentBuilder *db);
  _TEGGO_EXPORTABLE TEGGOt_E  CXX_STDCALL DEFparse(StringParam source, XMLdocumentBuilder *db);
  _TEGGO_EXPORTABLE Xdocument CXX_STDCALL XopenDefDocument(StringParam docname);
  _TEGGO_EXPORTABLE Xdocument CXX_STDCALL XopenDefDocument(DataStream &);
  _TEGGO_EXPORTABLE TEGGOt_E  CXX_STDCALL XwriteDefDocument(Xnode *root_node, StringParam fname, bool use_utf8 = false);
  _TEGGO_EXPORTABLE TEGGOt_E  CXX_STDCALL XwriteDefDocument(Xnode *root_node, DataStream *ds, bool use_utf8 = false);

} // namespace teggo

#if defined _TEGGOINLINE
#include "defparser.inl"
#endif

#endif //___15724387_86d6_46a7_8d40_ca2bf44700d6___
