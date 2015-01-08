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
        //assert(gate->getType() == AIG_GATE);

        if(gate->_faninList[0].getId() > gate->_faninList[1].getId())
        {
            input1 = gate->_faninList[0].getInvId();
            input2 = gate->_faninList[1].getInvId();
        }
        else
        {
            input1 = gate->_faninList[1].getInvId();
            input2 = gate->_faninList[0].getInvId();
        }
    }
    FanKey()
    {}
    unsigned int operator() () const
    {
        unsigned int key = (size_t(input2) << 16) + size_t(input1);
        return key;
    }
    bool operator == (const FanKey& k) const 
    { return ((input1 == k.input1)&&(input2 == k.input2));}
private:
     int input1;
     int input2;
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
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

