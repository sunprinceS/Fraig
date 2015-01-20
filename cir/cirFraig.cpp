/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"


using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
class FanKey
{
public:
    FanKey(CirGate* gate)
    {
        assert(gate->getType() == AIG_GATE);

        if(gate->_faninList[0].getGateV() > gate->_faninList[1].getGateV())
        {
            input1 = gate->_faninList[0].getGateV();
            input2 = gate->_faninList[1].getGateV();
        }
        else
        {
            input1 = gate->_faninList[1].getGateV();
            input2 = gate->_faninList[0].getGateV();
        }
    }
    FanKey()
    {}
    unsigned int operator() () const
    {
        //unsigned int key = (size_t(input2) << 16) + size_t(input1);
        unsigned int key = input1 + (input2 << 5);
        return key;
    }
    bool operator == (const FanKey& k) const 
    { return ((input1 == k.input1)&&(input2 == k.input2));}
private:
     size_t input1;
     size_t input2;
};
/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
void
CirMgr::strash()
{
    IdList removeId;
    HashMap<FanKey,size_t>*gateHash=new HashMap<FanKey,size_t>(_dfsList.size());
    for(size_t i=0;i<_dfsList.size();++i)
    {
        if(_dfsList[i]->getType() == AIG_GATE)
        {
            size_t matchId = 0;
            FanKey fankey(_dfsList[i]);
            if(gateHash->check(fankey,matchId)){
                merge(CirGateV(_totalList[matchId],0),_dfsList[i]->_varId,"Strashing: ");
                removeId.push_back(_dfsList[i]->_varId);
            }
            else{
                gateHash->insert(make_pair(fankey,_dfsList[i]->_varId));
            }
        }
    }
    //gateHash->print();
    _bStrashed = true;
    for (size_t i = 0;i<removeId.size();++i) {
        delete _totalList[removeId[i]];
        _totalList[removeId[i]] = NULL;
    }
    genDfsList();
}

void
CirMgr::fraig()
{ 
    if(_solver == NULL){
        initSat();
    }
    for(size_t i=0;i<_fecGrps.size();++i)
    {
        for(size_t j=0;j<(*_fecGrps[i]).size();++j)
        {
            if((*_fecGrps[i])[j]!=-1)
            {
                for(size_t k=j+1;k<(*_fecGrps[i]).size();++k)
                {
                    bool inv = (_totalList[(*_fecGrps[i])[j]]->isInvSignal() != 
                                _totalList[(*_fecGrps[i])[k]]->isInvSignal());
                    if(solveSat((*_fecGrps[i])[j],(*_fecGrps[i])[k],inv))//not eq
                    {
                        continue;
                    }
                    else //eq
                    {
                        merge(CirGateV(_totalList[(*_fecGrps[i])[j]],inv),
                                (*_fecGrps[i])[k],"FRAIG");
                        (*_fecGrps[i])[k] = -1;
                    }
                }
            }
        }
    }
    genDfsList();
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::initSat()
{
    _solver = new SatSolver;
    _solver->initialize();

    //add all gate to the solver
    _totalList[0]->_satVar = _solver->newVar();
    for(size_t i=0;i<_dfsList.size();++i)
    {
        if(_dfsList[i]->getType() == AIG_GATE){
            _dfsList[i]->_satVar = _solver->newVar();
        }
    }

    for(size_t i=0;i<_dfsList.size();++i)
    {
        if(_dfsList[i]->getType() == AIG_GATE){
            _solver->addAigCNF(_dfsList[i]->_satVar,
                    _dfsList[i]->_faninList[0].gate()->_satVar,
                    _dfsList[i]->_faninList[0].isInv(),
                    _dfsList[i]->_faninList[1].gate()->_satVar,
                    _dfsList[i]->_faninList[1].isInv());
        }
    }
}
bool 
CirMgr::solveSat(size_t gid1,size_t gid2,bool inv)
{
    _solver->assumeRelease();
    
    if(gid1 == 0)//const
    {
       cout << "\nProving " << gid2 <<" = "
            <<(inv?"0":"1")<< "..."; 
       _solver->assumeProperty(_totalList[gid2]->_satVar,!inv);
    }
    else
    {
        cout << "\nProving (" << gid1 << ","
             << (inv?"!":"") << gid2 << ")...";

      Var f = _solver->newVar();
      _solver->addXorCNF(f,_totalList[gid1]->_satVar,false, 
                           _totalList[gid2]->_satVar,inv);
      _solver->assumeProperty(f, true);
    }
    bool result = _solver->assumpSolve();
    return result;

}
