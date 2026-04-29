// UIMSG — 1:1 還原 mofclient.c 0x455D30
#include "UI/UIMSG.h"

#include <cstring>

UIMSG::UIMSG(CControlBase* sender)
{
    m_aParam[0] = 0;
    m_aParam[1] = 0;
    m_nEventID  = -1;
    m_pSender   = sender;
    m_aParam[0] = 0;
    m_aParam[1] = 0;
    m_aParam[2] = 0;
    m_aParam[3] = 0;
}
