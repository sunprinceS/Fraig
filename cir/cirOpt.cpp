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
        for(size_t i=0;i<_totalList.size();++i){
            if(_totalList[i] != NULL){
                if(!(_totalList[i]->_bTraced) && 
                     _totalList[i]->getType()!=PI_GATE){

                    removeFromFanin (i,_totalList[i]->_faninList );
                    removeFromFanout(i,_totalList[i]->_fanoutList);
                    cout << "Sweeping: " << _totalList[i]->getTypeStr() 
                         << "(" <<_totalList[i]->_varId<< ") removed..." <<endl;
                    //if(_totalList[i]->getType() == AIG_GATE)
                    //--_A;
                    delete _totalList[i];
                    _totalList[i] = NULL;
                }
            }
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
bool
CirMgr::removeFromFanin(unsigned int gid,vector<CirGateV>& fanin)
{
    
    if(fanin.empty())
        return true;
    for (size_t i = 0;i<fanin.size();++i) {
        for (size_t j = 0;j<fanin[i].gate()->_fanoutList.size();++j) {
            if(fanin[i].gate()->_fanoutList[j].gate()->_varId == gid){
                fanin[i].gate()-> _fanoutList.erase(fanin[i].gate()->_fanoutList.begin()+j);
                break;
            }
            //else{
                //cerr << "remove Fanin has unexpected result at " << gid << endl;
                //return false;
            //}
        }
    }
    return true;
}
bool
CirMgr::removeFromFanout(unsigned int gid,vector<CirGateV>& fanout)
{
    if(fanout.empty())
        return true;
    for (size_t i = 0;i<fanout.size();++i) {
        for (size_t j = 0;j<fanout[i].gate()->_faninList.size();++j) {
            if(fanout[i].gate()->_faninList[j].gate()->_varId == gid){
                fanout[i].gate()-> _faninList.erase(fanout[i].gate()->_faninList.begin()+j);
                break;
            }
        }
    }
    //cerr << "remove Fanout has unexpected result at " << gid << endl;
    //return false;
    return true;
}
