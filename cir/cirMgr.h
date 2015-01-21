/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "cirGate.h"
using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;
/**********************/
/*   struct and enum  */
/**********************/
struct gateEle
{ 
    unsigned int _LHS;
    unsigned int _RHS1;
    unsigned int _RHS2;
};

enum formatType { 
    ASCII,
    BINARY,

    DUMMYFORMAT
};

class CirMgr
{ 
    friend CirGate;
public:
   CirMgr():_gateDeclareLineNo(2),_solver(NULL),
    _bDFSd(false),_bStrashed(false),_bSimd(false){};
   ~CirMgr()
   {
       for (size_t i = 0;i<_totalList.size();++i) {
           if(_totalList[i] != NULL)
               delete _totalList[i];
           _totalList[i] = NULL;
       }

       for(size_t i=0;i<_POs.size();++i)
       {
           delete _POs[i];
           _POs[i] = NULL;
       }
       resetFecGrps();
       if(_solver != NULL){
           delete _solver;
           _solver = NULL;}
   }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const ;

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFECPairs() const;
   void fraig();
    
   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist(); 
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) ;

private:

    //header info
    formatType _cirFormat;
    unsigned int _M;
    unsigned int _I;
    unsigned int _L;
    unsigned int _O;
    unsigned int _A;

    //important list
    GateList _totalList;//total gates of PI and AND
    GateList _dfsList;//active gates


    //input
    IdList _PIIds;//literal id

    //output
    vector<POGate*> _POs;
    IdList _POIds;//literal id

    //And Gate
    vector<gateEle> _andGateIds;//literal id

    //write
    IdList _tracedAndGateIds;
    unsigned int _gateDeclareLineNo;
    
    //simulation
    ofstream           *_simLog;
    vector<unsigned int> _simValues;
    vector<FecGrp> _fecGrps; 
    
    //fraig
    SatSolver* _solver;

    //flags
    bool _bDFSd;
    bool _bStrashed;
    bool _bSimd;

    /******Helper function******/

    //Parse
    bool parseHeader(istream&);
    bool parseInput (istream&);
    bool parseOutput(istream&);
    bool parseAIGGate(istream&);
    bool parseSymbol(istream&);
    void parseCheck()const;
    
    //generating list
    bool genConnection();
    void dfsTraversal(CirGate* curGate);
    bool genDfsList();

    //optimization related
    void removeFromFanin (const unsigned int& gid,vector<CirGateV>& fanin);
    void removeFromFanout(const unsigned int& gid,vector<CirGateV>& fanout);
   
    void singleOutputSimplify(const unsigned int& gid);
    void zeroOutputSimplify(const unsigned int& gid);
    void merge(const CirGateV&,const unsigned int&,string why = "");
    GateOutput checkOutput(const unsigned int& gid)const;
    void reconnectFanout(const CirGateV&,const unsigned int&,vector<CirGateV>&);

    //simulation related
    void initSim();
    bool simulation();
    size_t gateSim(CirGate* curGate);
    void splitStr(string& line,vector<string>* seq); 
    void recordSim(size_t numSim);

    //fraig related
    void initSat();
    bool solveSat(size_t gid1,size_t gid2,bool inv);
    void resetFecGrps();

    //tool 
    void traversalReset();
    CirGate* findGate(unsigned int gid);
};
#endif // CIR_MGR_H
