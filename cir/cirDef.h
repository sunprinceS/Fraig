/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include "myHashMap.h"

using namespace std;

// TODO: define your own typedef or enum

class CirGate;
class CirMgr;
class SatSolver;

/***TypeDef and Enum***/
typedef vector<CirGate*>           GateList;
typedef vector<unsigned>           IdList;
typedef IdList*              FecGrp;
enum GateType
{ 
   UNDEF_GATE = 0,
   PI_GATE    = 1,
   PO_GATE    = 2,
   AIG_GATE   = 3,
   CONST_GATE = 4,

   TOT_GATE
};

enum GateOutput
{
    ZERO_OUTPUT   =  0,
    SINGLE_OUTPUT =  1,
    NORMAL_OUTPUT =  2,

    TOT_OUTPUT
};
#endif // CIR_DEF_H
