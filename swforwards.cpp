/*
    This file is part of SourcePawn SteamWorks.

    SourcePawn SteamWorks is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SourcePawn SteamWorks is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SourcePawn SteamWorks.  If not, see <http://www.gnu.org/licenses/>.
	
	Author: Kyle Sanderson (KyleS).
*/

#include "swforwards.h"

SteamWorksForwards::SteamWorksForwards() :
		m_CallbackGSClientApprove(this, &SteamWorksForwards::OnGSClientApprove),
		m_CallbackValidateTicket(this, &SteamWorksForwards::OnValidateTicket),
		m_CallbackSteamConnected(this, &SteamWorksForwards::OnSteamServersConnected),
		m_CallbackSteamConnectFailure(this, &SteamWorksForwards::OnSteamServersConnectFailure),
		m_CallbackSteamDisconnected(this, &SteamWorksForwards::OnSteamServersDisconnected)
{
	this->pFOSSC = forwards->CreateForward("Steamworks_OnServersConnected", ET_Ignore, 0, NULL);
	this->pFOSSCF = forwards->CreateForward("Steamworks_OnServersConnectFailure", ET_Ignore, 1, NULL, Param_Cell);
	this->pFOSSD = forwards->CreateForward("Steamworks_OnServersDisconnected", ET_Ignore, 1, NULL, Param_Cell);
    this->pFOVC = forwards->CreateForward("Steamworks_OnFamilySharingDetected", ET_Ignore, 2, NULL, Param_String, Param_String);
}

SteamWorksForwards::~SteamWorksForwards()
{
	forwards->ReleaseForward(this->pFOVC);
	forwards->ReleaseForward(this->pFOSSC);
	forwards->ReleaseForward(this->pFOSSCF);
	forwards->ReleaseForward(this->pFOSSD);
}

// workaround for compatibility
inline const char *RenderSteamID(CSteamID steamID)
{
    char* pchBuf;
    static char rgchBuf[4][30];
    static int nBuf = 0;
    pchBuf = rgchBuf[nBuf++];
    nBuf %= 4;
    
    AccountID_t steamid32 = steamID.GetAccountID();

    sprintf(pchBuf, "STEAM_1:%u:%u", (steamid32 % 2) ? 1 : 0, (uint32)steamid32 / 2);
    return pchBuf;
}

void SteamWorksForwards::NotifyPawnValidateClient(CSteamID child, CSteamID parent)
{
	if (this->pFOVC->GetFunctionCount() == 0)
	{
		return;
	}

    if (child.GetAccountID() != parent.GetAccountID())
    {
        this->pFOVC->PushString(RenderSteamID(child));
        this->pFOVC->PushString(RenderSteamID(parent));
        this->pFOVC->Execute(NULL);
    }
}

void SteamWorksForwards::OnGSClientApprove(GSClientApprove_t *pApprove)
{
	this->NotifyPawnValidateClient(pApprove->m_SteamID, pApprove->m_OwnerSteamID);
}

void SteamWorksForwards::OnValidateTicket(ValidateAuthTicketResponse_t *pTicket)
{
	this->NotifyPawnValidateClient(pTicket->m_SteamID, pTicket->m_OwnerSteamID);
}

void SteamWorksForwards::OnSteamServersConnected(SteamServersConnected_t *pResponse)
{
	if (this->pFOSSC->GetFunctionCount() == 0)
	{
		return;
	}
	
	this->pFOSSC->Execute(NULL);
}

void SteamWorksForwards::OnSteamServersConnectFailure(SteamServerConnectFailure_t *pResponse)
{
	if (this->pFOSSCF->GetFunctionCount() == 0)
	{
		return;
	}
	
	this->pFOSSCF->PushCell(pResponse->m_eResult);
	this->pFOSSCF->Execute(NULL);
}

void SteamWorksForwards::OnSteamServersDisconnected(SteamServersDisconnected_t *pResponse)
{
	if (this->pFOSSD->GetFunctionCount() == 0)
	{
		return;
	}

	this->pFOSSD->PushCell(pResponse->m_eResult);
	this->pFOSSD->Execute(NULL);
}
