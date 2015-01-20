/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
void
CirMgr::sweep()
{  
      if(_bDFSd){
        IdList removeId;
        for(size_t i=0;i<_totalList.size();++i){
            if(_totalList[i] != NULL){
                if(!(_totalList[i]->_bTraced) && 
                     (_totalList[i]->getType()==AIG_GATE || _totalList[i]->getType() == UNDEF_GATE)){

                    removeFromFanin (i,_totalList[i]->_faninList );
                    removeFromFanout(i,_totalList[i]->_fanoutList);
                    cout << "Sweeping: " << _totalList[i]->getTypeStr() 
                         << "(" <<_totalList[i]->_varId<< ") removed..." <<endl;

                    removeId.push_back(i);
                }
            }
        }
        for(size_t i=0;i<removeId.size();++i){
            delete _totalList[removeId[i]];
            _totalList[removeId[i]] = NULL;
        }
        _A = _tracedAndGateIds.size();//have been tested!!
    }
}

void
CirMgr::optimize()
{ 
    if(_bSimd == false && _bDFSd == true)
    {
        _bDFSd = false;

        IdList removeId;
        for(size_t i=0;i<_dfsList.size();++i){
            if(_dfsList[i]->getType() == AIG_GATE){
                GateOutput output = checkOutput(_dfsList[i]->_varId);
                if(output != NORMAL_OUTPUT){
                    if(output == ZERO_OUTPUT)
                        zeroOutputSimplify(_dfsList[i]->_varId);
                    else
                        singleOutputSimplify(_dfsList[i]->_varId);

                    removeId.push_back(_dfsList[i]->_varId);
                    _dfsList[i]->_bTraced = false;
                }
            }
        }

        for (size_t i = 0;i<removeId.size();++i) {
            delete _totalList[removeId[i]];
            _totalList[removeId[i]] = NULL;
        }
        if(!genDfsList()) 
            cerr <<"ERROR in genDfsList in Optimization"<< endl;
    }
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

//Only AIG_GATE can use this function
void
CirMgr::removeFromFanin(const unsigned int& gid,vector<CirGateV>& fanin)
{
    
    if(fanin.empty())
        return ;
    for (size_t i = 0;i<fanin.size();++i) {
        for (size_t j = 0;j<fanin[i].gate()->_fanoutList.size();++j) {
            if(fanin[i].gate()->_fanoutList[j].gate()->_varId == gid){
                fanin[i].gate()-> _fanoutList.erase(fanin[i].gate()->_fanoutList.begin()+j);
            }
        }
    }
    return ;
}

void
CirMgr::removeFromFanout(const unsigned int& gid,vector<CirGateV>& fanout)
{
    //assert(_totalList[gid]->getType() == AIG_GATE);

    if(fanout.empty())
        return ;
    for (size_t i = 0;i<fanout.size();++i) {
        for (size_t j = 0;j<fanout[i].gate()->_faninList.size();++j) {
            if(fanout[i].gate()->_faninList[j].gate()->_varId == gid){
                fanout[i].gate()-> _faninList.erase(fanout[i].gate()->_faninList.begin()+j);
            }
        }
    }
    return ;
}

void
CirMgr::zeroOutputSimplify(const unsigned int& gid)
{
    assert(_totalList[gid]->getType() == AIG_GATE);

    CirGateV replaceGateV(_totalList[0],0);
     
    merge(replaceGateV,gid,"Simplifying: "); 

}
void
CirMgr::singleOutputSimplify(const unsigned int& gid)
{  
    assert(_totalList[gid]->getType() == AIG_GATE);

    CirGateV replaceGateV(_totalList[0],0);
    if(_totalList[gid]->_faninList[0].getId() != 0){
        replaceGateV = _totalList[gid]->_faninList[0];
    }
    else{
        replaceGateV = _totalList[gid]->_faninList[1];
    }

    merge(replaceGateV,gid,"Simplifying: "); 
}

void
CirMgr::merge(const CirGateV& replaceGateV,const unsigned int& gid,string why)
{ 
    cout << why << replaceGateV.getId()
         << " merging "<<replaceGateV.getInvStr()<< gid << "..."<< endl;
    reconnectFanout(replaceGateV,gid,_totalList[gid]->_fanoutList);
    removeFromFanin(gid,_totalList[gid]->_faninList);
    --_A;
}



GateOutput
CirMgr::checkOutput(const unsigned int& gid)const
{
    assert(_totalList[gid]->getType() == AIG_GATE);

    short who = -1;
    if(_totalList[gid]->_faninList[0].getId() == 0)
        who = 0;
    else if(_totalList[gid]->_faninList[1].getId() == 0)
        who = 1;
    else; //no CONST fanin

    //CONST fanin
    if(who == 0 || who == 1)
    {
        if(_totalList[gid]->_faninList[who].isInv())
            return SINGLE_OUTPUT;
        else
            return ZERO_OUTPUT;
    }

    //NON_CONST fanin,but output is either always 0 or 1
    if(_totalList[gid]->_faninList[0].getId() == 
       _totalList[gid]->_faninList[1].getId()){
        if(_totalList[gid]->_faninList[0].isInv() == 
           _totalList[gid]->_faninList[1].isInv()){
            return SINGLE_OUTPUT;
        }
        else
            return ZERO_OUTPUT;
    }
    return NORMAL_OUTPUT;
}

void
CirMgr::reconnectFanout(const CirGateV& replaceGateV,const unsigned int& gid,
        vector<CirGateV>& fanout)
{
    assert(_totalList[gid]->getType() == AIG_GATE);

    if(fanout.empty())
        return ;
    for(size_t i=0;i<fanout.size();++i){
        for(size_t j=0;j<fanout[i].gate()->_faninList.size();++j){
            if(fanout[i].gate()->_faninList[j].getId() == gid){
                bool invOrNot = fanout[i].gate()->_faninList[j].isInv();
                fanout[i].gate()->_faninList[j] = replaceGateV;

                //connect fanin to the fanout
                if(invOrNot)
                    fanout[i].gate()->_faninList[j].inverse();
                //connect fanout to the fanin
                _totalList[replaceGateV.getId()]->_fanoutList.push_back(fanout[i]);
            } 
        }
    }
}
