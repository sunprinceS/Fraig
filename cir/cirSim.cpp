/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <cmath>
#include <string>

using namespace std;

// TODO: Keep "CirMgr::randomSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
/**************************************/
/*   Static varaibles and functions   */
/**************)***********************/
class SimValue
{
public:
    SimValue(size_t sim)
    {
        //if((sim &0x1) == 0)
            _sim = sim;
        //else
            //_sim = ~sim;
    }
    SimValue()
    {}
    unsigned int operator() () const{
        return _sim;}
    SimValue inverse(){return ~_sim;}
    bool operator == (const SimValue& k) const {
        return (_sim == k._sim) ;}
private:
     size_t _sim;
};
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{ 
    _simValues.resize(_I,0);

    /******MAGIC FUNCTION********/
    size_t maxFail = (size_t)(log10(_A + _I))*2 + 3;
    /****************************/
    cout << "MAX_FAILS = " << maxFail << endl;
    
    size_t fails = 0;
    size_t cycles = 0;
    
    if(this->_bSimd == false){
        _bSimd = true;
        initSim();}

    while(fails < maxFail){
        for(size_t i=0;i<_simValues.size();++i){
            _simValues[i] = rnGen(INT_MAX);}

        if(simulation())
            fails = 0;
        else
            ++fails;
        
        if(_simLog != NULL)
            recordSim(32); 
        
        ++cycles;
    }
    cout << cycles*32 << " patterns simulated." << endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
    _simValues.resize(_I,0);
    size_t numSim=0;

    string curLine;
    vector<string> sequence; 
    if(this->_bSimd == false){
        _bSimd = true;
        initSim();}

    while(getline(patternFile,curLine)){

        splitStr(curLine,&sequence);        
        for(size_t i=0;i<sequence.size();++i)
        {
            //check length
            if(sequence[i].length()!=_I){
                cerr << "\nError: Pattern(" << sequence[i] 
                    <<  ") length(" << sequence[i].length()
                    << ") does not match the number of inputs("
                    << _I << ") in a circuit!!" << endl;
                break;
            }

            //check format
            size_t errPoint = sequence[i].find_first_not_of("01");
            if(errPoint != string::npos){
                cerr << "\nError: Pattern(" << sequence[i] << ")"
                    << "contains a non-0/1 character(\'" 
                    << sequence[i][errPoint] << "\')." << endl;
                break;
            }

            //setup simvalue
            for(size_t j=0;j<_I;++j){
                if(sequence[i][j] == '1'){
                    _simValues[j] += (1<<(numSim%32));
                }
            }
            ++numSim;

            if(numSim%32 == 0){
                simulation();
                if(_simLog != NULL)
                    recordSim(32);

                for(size_t j=0;j<_I;++j){
                    _simValues[j] = 0;
                }
            }
        }
        sequence.clear();
    }
    if(numSim%32 != 0){
        simulation();

        if(_simLog != NULL)
            recordSim(numSim%32);

        for(size_t i=0;i<_I;++i){
            _simValues[i] = 0;
        }
    }
    cout << numSim << " patterns simulated." << endl;
}


/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void 
CirMgr::splitStr(string& line,vector<string>* seq)
{
    size_t pos = line.find_first_not_of(" ");
    size_t seqEnd;
    while(pos != string::npos)
    {
        seqEnd = line.find_first_of(" ",pos);
        seq->push_back(line.substr(pos,seqEnd-pos));
        pos = line.find_first_not_of(" ",seqEnd);
    }
    return ;
}
void 
CirMgr::initSim(){

      //used in first simulation
      _fecGrps.reserve(_tracedAndGateIds.size() + 1);
      FecGrp firstGrp; 
      firstGrp = new IdList;
      firstGrp->reserve(_tracedAndGateIds.size()+1);


      firstGrp->push_back(0); //push const 0 gate
      for(size_t i = 0;i < _dfsList.size();++i)
      { 
         if(_dfsList[i]->getType() == AIG_GATE)
         {
            {
               firstGrp->push_back(_dfsList[i]->_varId);
               _dfsList[i]->_fecGrp = firstGrp;
            }
         }
      }
    _fecGrps.push_back(firstGrp);
}

bool 
CirMgr::simulation(){

    for(size_t i=0;i<_I;++ i){
        _totalList[_PIIds[i]/2]->setSimValue(_simValues[i]);
    }
    
    //simulation every dfs gate
    for(size_t i=0;i<_dfsList.size();++i)
    {
        if(_dfsList[i]->getType()!= PI_GATE && 
           _dfsList[i]->getType()!=CONST_GATE){

           _dfsList[i]->setSimValue(gateSim(_dfsList[i]));
        }
    }
    
    vector<FecGrp> oldFecGrps;
    _fecGrps.swap(oldFecGrps);

    //we've assumed that the _fecGrps have been initialized
    //partition begin
    for(size_t i=0;i<oldFecGrps.size();++i)
    {
        HashMap<SimValue,FecGrp> newFecGrps(getHashSize(oldFecGrps[i]->size()));
        for(size_t j=0;j<oldFecGrps[i]->size();++j)
        {
            FecGrp group = NULL;
            size_t sim = _totalList[(*(oldFecGrps[i]))[j]]->_simValue;
            SimValue simVal(sim);

            if(newFecGrps.check(simVal,group))
            {
                group->push_back((*(oldFecGrps[i]))[j]);
                _totalList[(*oldFecGrps[i])[j]]->setInvSignal(false);
            }
            else if(newFecGrps.check(simVal.inverse(),group))
            {
                group->push_back((*(oldFecGrps[i]))[j]);
                _totalList[(*oldFecGrps[i])[j]]->setInvSignal(true);
            }
            else{
                group = new IdList;
                group->push_back((*(oldFecGrps[i]))[j]);
                _totalList[(*oldFecGrps[i])[j]]->setInvSignal(false);
                newFecGrps.insert(make_pair(simVal,group));
            }
            _totalList[(*(oldFecGrps[i]))[j]]->_fecGrp = group;

        }

        HashMap<SimValue,FecGrp>::iterator it=newFecGrps.begin();
        for (; it != newFecGrps.end(); ++it)
        {
            if(( (*it).second)->size() > 1)
                _fecGrps.push_back((*it).second);
            else{
                _totalList[((*it).second->front())]->_fecGrp = NULL;
                delete (*it).second;
            }
        }
    }
    for(size_t i=0;i<oldFecGrps.size();++i)
    {
        delete oldFecGrps[i];
        oldFecGrps[i] = NULL;
    }
    cout << "Total #FEC Group = " << _fecGrps.size() << '\r';
    return !(oldFecGrps.size() == _fecGrps.size());
}

size_t 
CirMgr::gateSim(CirGate* curGate){
    
    size_t ret;
    if(curGate ->getType() == AIG_GATE)
    {
        size_t input1 = curGate->_faninList[0].getSimValue();
        size_t input2 = curGate->_faninList[1].getSimValue();
        ret = (input1&input2); 
    }
    else if(curGate -> getType() == PO_GATE)
    {
        ret = curGate->_faninList[0].getSimValue();
    }
    else{cerr << "Unexpected gateType in gateSim" << endl;}
    return ret;
}

void
CirMgr::recordSim(size_t simNum)
{
    for(size_t i=0;i<simNum;++i)
    {
        for(size_t j=0;j<_I;++j){
            if((_totalList[_PIIds[j]/2]->_simValue&(0x1<<i)))
                *_simLog << '1';
            else
                *_simLog << '0';
        }
        *_simLog << " ";

        for(size_t j=0;j<_O;++j){
            if((_POs[j] ->_simValue&(0x1<<i)))
                *_simLog << '1';
            else
                *_simLog << '0';
        }
        *_simLog << endl;
    }
}
