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
                     _totalList[i]->getType()!=PI_GATE){

                    removeFromFanin (i,_totalList[i]->_faninList );
                    removeFromFanout(i,_totalList[i]->_fanoutList);
                    cout << "Sweeping: " << _totalList[i]->getTypeStr() 
                         << "(" <<_totalList[i]->_varId<< ") removed..." <<endl;

                    //delete _totalList[i];
                    //_totalList[i] = NULL;
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
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::removeFromFanin(unsigned int gid,vector<CirGateV>& fanin)
{
    
    if(fanin.empty())
        return ;
    for (size_t i = 0;i<fanin.size();++i) {
        if(fanin[i].gate()->_bTraced){
            for (size_t j = 0;j<fanin[i].gate()->_fanoutList.size();++j) {
                if(fanin[i].gate()->_fanoutList[j].gate()->_varId == gid){
                    fanin[i].gate()-> _fanoutList.erase(fanin[i].gate()->_fanoutList.begin()+j);
                    break;
                }
            }
        }
    }
    return ;
}
void
CirMgr::removeFromFanout(unsigned int gid,vector<CirGateV>& fanout)
{
    if(fanout.empty())
        return ;
    for (size_t i = 0;i<fanout.size();++i) {
        if(fanout[i].gate()->_bTraced){
            for (size_t j = 0;j<fanout[i].gate()->_faninList.size();++j) {
                if(fanout[i].gate()->_faninList[j].gate()->_varId == gid){
                    fanout[i].gate()-> _faninList.erase(fanout[i].gate()->_faninList.begin()+j);
                    break;
                }
            }
        }
    }
    return ;
}
