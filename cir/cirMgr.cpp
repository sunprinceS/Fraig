/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#define  _constGate (_totalList[0])
#include <algorithm>

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
CirGate* 
CirMgr::getGate(unsigned gid) const 
{ 
    if(gid > _M+_O) {return NULL;}
    if(gid <= _M){
        if(_totalList[gid]!=NULL)
            return _totalList[gid];
        return NULL;
    }
    else {
        if(_POs[gid-_M-1] == NULL)
        {cout << "ERROR in getGate" << endl;return NULL;}
        return _POs[gid-_M-1];
    }
}
bool
CirMgr::readCircuit(const string& fileName)
{ 
    ifstream cirFile;
    
    cirFile.open(fileName.c_str(),ios::in);
    if(cirFile.is_open())
    {
        if(!(parseHeader(cirFile))) {return false;}
        if(!(parseInput(cirFile))) {return false;}
        if(!(parseOutput(cirFile))) {return false;}
        if(!(parseAIGGate(cirFile))) {return false;}
        if(!(parseSymbol(cirFile))) {return false;}
        //parseCheck();//for debug
        
        if(!(genConnection())) {return false;}
        
        if(!(genDfsList())) {return false;}
        
        for (size_t i = 0;i<_totalList.size();++i) {
            if(_totalList[i] != NULL)
                _totalList[i]->sortFan();
        }
        return true;
    }
    else
    {
        cerr << "Cannot open design \""  << fileName<< " \"!!" << endl; 
        return false;
    }
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
    cout << "\nCircuit Statistics\n"
         << "==================\n"
         << "  PI    " << setw(8) << right << _I << endl
         << "  PO    " << setw(8) << right << _O << endl
         << "  AIG   " << setw(8) << right << _A << endl
         << "------------------\n"
         << "  Total " << setw(8) << _I+_O+_A << endl;
    
}

void
CirMgr::printNetlist() 
{
    cout << endl;
    for (size_t i = 0; i < _dfsList.size(); ++i) {
        cout << "[" << i << "] ";
        _dfsList[i]->printGate();
        cout << endl;
    }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(size_t i=0;i<_I;++i)
   {
       cout << " " << _PIIds[i]/2;
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(size_t i=0;i<_O;++i)
   {
       cout << " " << _POs[i]->_varId;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
    bool floatFirst = true;
    bool notUsedFirst = true;
    
    //floating fanin
    for (size_t i = 1; i < _totalList.size(); ++i){
        if(_totalList[i]!=NULL)
        {
            if(_totalList[i]->hasFloatingFanin()){
                if(floatFirst){
                    cout << "Gates with floating fanin(s):";
                    floatFirst = false;
                }
                cout << " " <<  i;
            }
        }
    }
    for (size_t i = 0; i < _POs.size(); ++i){
        if(_POs[i]->hasFloatingFanin()){
            if(floatFirst){
                cout << "Gates with floating fanin(s):";
                floatFirst = false;
            }
            cout << " " <<  _POs[i]->_varId;
        }
    }
    //defined but not used
    for (size_t i = 1; i < _totalList.size(); ++i){
        if(_totalList[i]!=NULL){
            if(_totalList[i]->isUsed() == false){
                if(notUsedFirst){
                    cout << "\nGates defined but not used  :";
                    notUsedFirst = false;
                }
                cout << " " <<  i;
            }
        }
    }
}

void
CirMgr::printFECPairs() const
{ 
}

void
CirMgr::writeAag(ostream& outfile) 
{

    if(_cirFormat == ASCII){outfile << "aag ";}
    else {outfile << "aig ";}

    outfile << _M << " " << _I << " " << _L << " " << _O << " " 
            << _tracedAndGateIds.size()<< endl;
    for(size_t i=0;i<_I;++i)
    {
        outfile << _PIIds[i] << endl;
    }
    
    //output
    for(size_t i=0;i<_O;++i)
    {
        outfile << _POIds[i] << endl;
    }
    //andGate
    for(size_t i=0;i<_tracedAndGateIds.size();++i)
    {
        unsigned int id = _tracedAndGateIds[i];
        outfile << _andGateIds[id]._LHS << " "
                << _andGateIds[id]._RHS1 << " " <<_andGateIds[id]._RHS2 << endl;
    }
    //symbol
    for(size_t i=0;i<_PIIds.size();++i)
    {
        if(_totalList[_PIIds[i]/2]->hasSymbol())
        outfile << "i" << i << " " << _totalList[_PIIds[i]/2]->getSymbol()<< endl;
            
    }
    for(size_t i=0;i<_POs.size();++i)
    {
        if(_POs[i]->hasSymbol())
        outfile << "o" << i << " " << _POs[i]->getSymbol()<< endl;
            
    }
    //comment
    outfile <<"c\n"<<"~Circuit created by TonyHsu~" << endl;

}

bool
CirMgr::parseHeader(istream& file)
{
    char header[3];
    file>> header[0] >> header[1] >> header[2];
    if(header[0] == 'a'&& header[2] == 'g')
    {
        if(header[1] == 'a'){
            _cirFormat = ASCII;
        }
        else if(header[1] == 'i'){
            _cirFormat = BINARY;
        }
        else{return false;}
    }
    file >> _M >> _I >> _L >> _O >> _A;

    _totalList.resize(_M+1,NULL);
    _totalList[0] = new ConstGate;
    return true;
}

bool 
CirMgr::parseInput(istream& file)
{
    _PIIds.resize(_I);

    for(size_t i=0;i<_I;++i)
    {
        unsigned int tmpId;
        unsigned int varId;
        file >> tmpId;
        varId = tmpId/2;
        if((varId) > _M || varId == 0){return false;}

        _PIIds[i] = tmpId;
        _totalList[varId] = new PIGate(varId,_gateDeclareLineNo);
        ++_gateDeclareLineNo;
    }
    return true;
}

bool
CirMgr::parseOutput(istream& file)
{
    _POIds.resize(_O);
    _POs.resize(_O);
    for(size_t i=0;i<_O;++i)
    {
        unsigned int tmpId;
        unsigned int varId = _M+i+1;
        file >> tmpId;

        _POIds[i] = tmpId;
        _POs[i] = new POGate(varId,_gateDeclareLineNo);
        ++_gateDeclareLineNo;
    }
    return true;
}

bool CirMgr::parseAIGGate(istream& file)
{
    _andGateIds.resize(_A);
    for(size_t i=0;i<_A;++i)
    {
        gateEle tmpId;
        file >> tmpId._LHS >> tmpId._RHS1 >> tmpId._RHS2;
        unsigned int varId = tmpId._LHS/2;
        if(varId ==0 || varId >_M){return false;}

        _totalList[varId] = new AIGGate(varId,i,_gateDeclareLineNo);
        _andGateIds[i] = tmpId;
        ++_gateDeclareLineNo;
    }
    return true;
}

bool 
CirMgr::parseSymbol(istream& file)
{
    char type;
    unsigned int pos; 
    string  info; 
    string sb;
    getline(file,info);//Need to add,but Ifon't know why@@
    while(getline(file,info))
    { 
        //cout << info << endl;
        if(info[0] == 'i' || info[0] == 'l' || info[0] == 'o')
            type = info[0];
        else if(info[0] == 'c')
            return true;
        else
            return false;
        
        size_t found = info.find_first_of(" ",1);
        string numStr = info.substr(1,found);
        pos= atoi(numStr.c_str());
        
        sb = info.substr(found+1);

        //set symbol
        if(type == 'i'){
            unsigned int varId = _PIIds[pos]/2;
            _totalList[varId]->setSymbol(sb);
        }
        else if(type == 'o'){
            _POs[pos]->setSymbol(sb);
        }
        else if(type == 'l'){}
        else {return false;}
    }
    return true;
}

//void
//CirMgr::parseCheck()const
//{
    ////header
    //if(_cirFormat == ASCII){cout << "aag ";}
    //else if(_cirFormat == BINARY){cout << "aig ";}
    //else;
    //cout << _M << " " << _I << " " << _L << " " << _O << " " << _A << endl;
    
    ////input
    //if(_I!=_PIIds.size()) {cout << "ERROR!!" << endl;}
    //for(size_t i=0;i<_I;++i)
    //{
        //cout << _PIIds[i] << endl;
    //}
    
    ////output
    //if(_O!=_POIds.size()) {cout << "ERROR!!" << endl;}
    //for(size_t i=0;i<_O;++i)
    //{
        //cout << _POIds[i] << endl;
    //}
    ////andGate
    //if(_A!=_andGateIds.size()) {cout << "ERROR" << endl;}
    //for(size_t i=0;i<_A;++i)
    //{
        //cout << _andGateIds[i]._LHS << " " << _andGateIds[i]._RHS1
             //<< " " << _andGateIds[i]._RHS2 << endl;
    //}

    //cout << "parse check end..." << endl;
//}

bool
CirMgr::genConnection()
{
    //begin from POs connection
    for(size_t i=0;i<_O;++i)
    {
        unsigned int inputId = _POIds[i]/2;
        bool inv = _POIds[i]%2;
        CirGate* fanin = findGate(inputId);
        _POs[i]->_faninList.push_back(CirGateV(fanin,inv));
        
        fanin->_fanoutList.push_back(CirGateV(_POs[i],inv));
    }

    //gate connection
    for(size_t i=0;i<_A;++i)
    {
       unsigned int gateId = _andGateIds[i]._LHS/2;
       unsigned int inputId1 = _andGateIds[i]._RHS1/2;
       bool inv1 = _andGateIds[i]._RHS1%2;
       unsigned int inputId2 = _andGateIds[i]._RHS2/2;
       bool inv2 = _andGateIds[i]._RHS2%2;
       
       CirGate* fanin1 = findGate(inputId1);
       CirGate* fanin2 = findGate(inputId2);
       _totalList[gateId]->_faninList.push_back(CirGateV(fanin1,inv1));
       _totalList[gateId]->_faninList.push_back(CirGateV(fanin2,inv2));

       fanin1->_fanoutList.push_back(CirGateV(_totalList[gateId],inv1));
       fanin2->_fanoutList.push_back(CirGateV(_totalList[gateId],inv2));
    }
    return true;
}

bool
CirMgr::genDfsList()
{
    traversalReset();
    if(_totalList.empty() && _POs.empty())
        return false;
    for(size_t i=0;i<_POs.size();++i)
    {
        dfsTraversal(_POs[i]);
    }
    _bDFSd = true;
    return true;
}
void
CirMgr::dfsTraversal(CirGate* curGate)
{
    GateType curGateType = curGate->getType();
    assert(curGate->isGlobalRef() == false &&
           curGate->getType() != UNDEF_GATE);
            
    
    for(size_t i=0;i<curGate->_faninList.size();++i)
    {
        if(curGate->_faninList[i].gate()->isGlobalRef() == false && 
           curGate->_faninList[i].gate()->getType() != UNDEF_GATE)
            dfsTraversal(curGate->_faninList[i].gate());
        else if(curGate->_faninList[i].gate()->getType()==UNDEF_GATE)
            curGate->_faninList[i].gate()->setTracedOrNot(true);
        else;//traced but not undef
    }

    _dfsList.push_back(curGate);
    curGate->setTracedOrNot(true);

    if(curGateType == AIG_GATE)
    {
        _tracedAndGateIds.push_back(curGate->getIdIndex());
    }
    curGate->setToGlobalRef();
}


//if not found,it will return undefGates,rather than NULL
CirGate* 
CirMgr::findGate(unsigned int gid)//used in connection
{
    assert(gid <= _M+_O);
    CirGate* sGate = getGate(gid);
    if(sGate != NULL)
        return sGate;
    else{
        _totalList[gid] = new UndefGate(gid);
        return _totalList[gid];
    }
}

void
CirMgr::traversalReset()
{
    _dfsList.clear();
    _tracedAndGateIds.clear();
    CirGate::setGlobalRef();
}

