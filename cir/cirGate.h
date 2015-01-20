/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include<algorithm>
#include "sat.h"
#include <climits>
#include <sstream>
#include <iomanip>


using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;
class CirGateV;
class FanKey;
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{  
    friend CirMgr;
    friend CirGateV; 
    friend FanKey;
public:
   CirGate(unsigned int id,unsigned int l=0,bool bTraced = false):
            _varId(id),_lineNo(l),_colNo(0),_symbol(""),
            _bTraced(bTraced),_fecGrp(0),_simValue(0),_ref(_globalRef){};
   
   virtual ~CirGate() {};

   // Basic access methods
   virtual string getTypeStr() const=0;
   virtual GateType getType()  const=0; 
   unsigned getLineNo() const { return _lineNo; }
   virtual unsigned int getIdIndex() const=0;

   //void setFecGrpNum(size_t num) {_fecGrpNum = num;}
   //size_t getFecGrpNum()const {return _fecGrpNum;}
   
   //property related
   void setTracedOrNot(bool traced) {_bTraced = traced;}
   bool isTraced()const {return _bTraced;}
     bool isUsed()const {
       return (getType()==PO_GATE) || !(_fanoutList.empty());}
   bool hasFloatingFanin()const;

   //symbol
   bool hasSymbol() const{return _symbol!="";}
   void setSymbol(string symbol) {_symbol = symbol;}
   string getSymbol() const {return _symbol;}
   
   // Print functions
   void printGate() const;
   void reportGate() const;
   void reportFanin(int level);
   void reportFanout(int level);
    
   //simulation related
   void setSimValue(const size_t simValue){_simValue = simValue;}
   void setInvSignal(bool inv){_invSignal = inv;}
   bool isInvSignal()const {return _invSignal;}

   //traversal methods
   bool isGlobalRef()const{return _ref == _globalRef;}
   static void setGlobalRef(){++(_globalRef);}
   void setToGlobalRef()const {_ref = _globalRef;}
    
   //fraig related
   void setVar(Var var){_satVar = var;}

   //other
   void sortFan();
   
protected:
    unsigned int _varId;
    unsigned int _lineNo;
    unsigned int _colNo;
    string _symbol;
    
    //for sweep
    bool _bTraced;

    //for simulate
    FecGrp _fecGrp;
    size_t _simValue;
    bool _invSignal;
    
    //for fraig
    Var _satVar;

    //for traversal
    mutable size_t _ref;
    static size_t _globalRef;

    vector<CirGateV> _faninList;
    vector<CirGateV> _fanoutList;

    /*****Helper Function*****/
    void fanoutTraversal(CirGate*,unsigned int, unsigned int,bool)const;
    void faninTraversal(CirGate*,unsigned int, unsigned int,bool)const;

    
};

class CirGateV
{
public:
    #define NEG 0x1
    CirGateV(const CirGate* gate,size_t phase):
        _gateV((size_t)gate + phase){}
    //CirGateV(const CirGateV& gateV):_gateV(gateV._gateV){}
    CirGate* gate() const {return (CirGate*)(_gateV & ~size_t(NEG));}
    bool isInv() const {return (_gateV & NEG);}
    int getInvId()const{
        if(isInv())
            return getId()*-1;
        else
            return getId();
        ;}
    size_t getGateV()const {return _gateV;}
    size_t getId()const{return this->gate()->_varId;}
    bool operator < (const CirGateV& rhs) const
    {return this->gate()->_varId < rhs.gate()->_varId;}
    void inverse() {_gateV ^= NEG;}
    string getInvStr()const{
        if(this->isInv())
            return "!";
        else
            return "";
    }
    size_t getSimValue()const{
        size_t ret = gate()->_simValue;
        if(this->isInv())
            ret = ~ret;
        return ret;
    }
private:
    size_t _gateV;
};

class AIGGate:public CirGate
{ 
public:
    AIGGate(unsigned int id,unsigned int i,unsigned int l=0):
        CirGate(id,l),_gateIdIndex(i){};
    ~AIGGate(){};
   virtual string getTypeStr()const {return "AIG";};
   virtual GateType getType()const{ return AIG_GATE;}
   virtual unsigned int getIdIndex()const{return _gateIdIndex;} 
private:
   unsigned int _gateIdIndex;
};

class PIGate:public CirGate
{ 
public:
    PIGate(unsigned int id,unsigned int l=0):CirGate(id,l){};
    ~PIGate(){};


   virtual string getTypeStr()const {return "PI";};
   virtual GateType getType()const{ return PI_GATE;}
   virtual unsigned int getIdIndex()const{return -1;}//error

};

class POGate:public CirGate
{ 
public:
    POGate(unsigned int id,unsigned int l=0):CirGate(id,l){};
    ~POGate(){};

   virtual string getTypeStr()const {return "PO";};
   virtual GateType getType()const{ return PO_GATE;}
   virtual unsigned int getIdIndex()const{return -1;}//error

};

class ConstGate:public CirGate
{
public:
    ConstGate():CirGate(0){_bTraced = true;};
    ~ConstGate(){};
    
   
   virtual string getTypeStr() const{return "CONST";};
   virtual GateType getType()const{ return CONST_GATE;}
   virtual unsigned int getIdIndex()const{return -1;}//error
};

class UndefGate:public CirGate
{ 
public:
    UndefGate(unsigned int id):CirGate(id){};
    ~UndefGate(){};

   virtual string getTypeStr() const{return "UNDEF";};
   virtual GateType getType()const{ return UNDEF_GATE;}
   virtual unsigned int getIdIndex()const{return -1;}//error

};
#endif // CIR_GATE_H

