/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"
#include <iomanip>
#include <sstream>

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
size_t CirGate::_globalRef = 0;
/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
    cout << "==================================================" << endl;

    ostringstream data;
    data << "= " << getTypeStr() << "(" << _varId << ")";
    if(_symbol != "")
        data << "\"" << _symbol << "\"";
    data << ", line " << _lineNo;
    cout << setw(49) << left << data.str() << right << "=" << endl;
    
    ostringstream fecs;
    fecs << "= FECs: ";
    if(_fecGrp != NULL)
    {
        for(size_t i=0;i<_fecGrp->size();++i)
            fecs << (*_fecGrp)[i] << " ";
    }
    cout << setw(49) << left << fecs.str() << right << "=" << endl;

    cout << "= Value: ";
    for(size_t i=1;i<=32;++i)
    {
        cout << ((_simValue & (0x1 << (32-i)))?'1':'0');
        if(i%4 == 0 && i!=32) {cout <<'_';}
    }
    cout << " =" << endl;

    cout << "==================================================" << endl;
    
}

void
CirGate::reportFanin(int level)
{
   assert (level >= 0);

   CirGate::setGlobalRef();
   this->faninTraversal(this,0,level,0);
}

void
CirGate::reportFanout(int level)
{
   assert (level >= 0);

   CirGate::setGlobalRef();
   this->fanoutTraversal(this,0,level,0);
}

void
CirGate::faninTraversal(CirGate* curGate,unsigned int curLevel,
        unsigned int maxLevel,bool inv)const
{
    assert(curLevel <= maxLevel);
    //indent
    for(size_t i=0;i<curLevel;++i)
    {
        cout << "  ";
    }
    
    // print curGate
    if(inv) {cout << "!";}
    cout << curGate->getTypeStr() << " " << curGate->_varId;
    
    if(curLevel == maxLevel || curGate->_faninList.empty())//maxLevel or Buttom
    {
        cout << endl;
        return ;
    } 

    //Traversal or not
    if(curGate->isGlobalRef())
        cout << " (*)" << endl;
    else
    {
        cout << endl;
        for(size_t i=0;i< curGate->_faninList.size();++i)
            faninTraversal(curGate->_faninList[i].gate(),curLevel+1,
                           maxLevel,curGate->_faninList[i].isInv());
    }
    curGate->setToGlobalRef();
    return;
}

void
CirGate::fanoutTraversal(CirGate* curGate,unsigned int curLevel,
        unsigned int maxLevel,bool inv)const
 {
    assert(curLevel <= maxLevel);
    //indent
    for(size_t i=0;i<curLevel;++i)
    {
        cout << "  ";
    }
    // print curGate
    if(inv) {cout << "!";}
    cout << curGate->getTypeStr() << " " << curGate->_varId;
    
    if(curLevel == maxLevel || curGate->_fanoutList.empty())//maxLevel or Top
    {
        cout << endl;
        return ;
    } 

    //Traversal or not
    if(curGate->isGlobalRef())
        cout << " (*)" << endl;
    else
    {
        cout << endl;
        for(size_t i=0;i< curGate->_fanoutList.size();++i)
            fanoutTraversal(curGate->_fanoutList[i].gate(),curLevel+1,
                           maxLevel,curGate->_fanoutList[i].isInv());
    }
    curGate->setToGlobalRef();
    return;
}

bool CirGate::hasFloatingFanin() const
{
    for (size_t i = 0; i < _faninList.size(); ++i) {
        if (_faninList[i].gate()->getType() == UNDEF_GATE)
            return true;
    }
    return false;
}
void CirGate::printGate() const
{
   if (_varId == 0) {
       cout << "CONST0";
       return;
   }
   cout << setw(4) << left << getTypeStr() << _varId;
   for (size_t i = 0; i < _faninList.size(); ++i) {
       cout << " ";
       if (_faninList[i].gate()->getType() == UNDEF_GATE)
           cout << "*";
       if (_faninList[i].isInv())
           cout << "!";
       cout << _faninList[i].gate()->_varId;
   }
   if (_symbol != "")
       cout << " (" << _symbol << ")";
}

void CirGate::sortFan()
{
    if (!(_fanoutList.empty()))
        sort(_fanoutList.begin(), _fanoutList.end());
}


