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

#include "gsnatives.h"

static bool IsSteamWorksLoaded(void)
{
	return (g_SteamWorks.pSWGameServer->GetSteamClient() != NULL);
}

static ISteamGameServer *GetGSPointer(void)
{
	return g_SteamWorks.pSWGameServer->GetGameServer();
}

static cell_t sm_IsVACEnabled(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();
	
	if (pServer == NULL)
	{
		return 0;
	}
	
	return pServer->BSecure() ? 1 : 0;
}

static cell_t sm_GetPublicIP(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();
	
	if (pServer == NULL)
	{
		return 0;
	}
	
	uint32_t ipaddr = pServer->GetPublicIP();
	
	cell_t *addr;
	pContext->LocalToPhysAddr(params[1], &addr);
	for (char iter = 3; iter > -1; --iter)
	{
		addr[(~iter) & 0x03] = (static_cast<unsigned char>(ipaddr >> (iter * 8)) & 0xFF); /* I hate you; SteamTools. */
	}
	
	return 1;
}

static cell_t sm_GetPublicIPCell(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();

	if (pServer == NULL)
	{
		return 0;
	}

	return pServer->GetPublicIP();
}

static cell_t sm_IsLoaded(IPluginContext *pContext, const cell_t *params)
{
	return IsSteamWorksLoaded() ? 1 : 0;
}

static cell_t sm_SetGameDescription(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();

	if (pServer == NULL)
	{
		return 0;
	}
	
	char *pDesc;
	pContext->LocalToString(params[1], &pDesc);
	
	pServer->SetGameDescription(pDesc);
	return 1;
}

static cell_t sm_IsConnected(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();

	if (pServer == NULL)
	{
		return 0;
	}

	return pServer->BLoggedOn() ? 1 : 0;
}

static cell_t sm_SetRule(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();

	if (pServer == NULL)
	{
		return 0;
	}

	char *pKey, *pValue;
	pContext->LocalToString(params[1], &pKey);
	pContext->LocalToString(params[2], &pValue);

	pServer->SetKeyValue(pKey, pValue);
	return 1;
}

static cell_t sm_ClearRules(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();

	if (pServer == NULL)
	{
		return 0;
	}

	pServer->ClearAllKeyValues();
	return 1;
}

static cell_t sm_ForceHeartbeat(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();

	if (pServer == NULL)
	{
		return 0;
	}

	pServer->ForceHeartbeat();
	return 1;
}

static cell_t sm_WasRestartRequested(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();

	if (pServer == NULL)
	{
		return 0;
	}

	return pServer->WasRestartRequested();
}

static cell_t sm_UserHasSubscription(IPluginContext *pContext, const cell_t *params)
{
	ISteamGameServer *pServer = GetGSPointer();

	if (pServer == NULL)
	{
		return 0;
	}
    
	int index = params[1];
	if (index < 1 || index > playerhelpers->GetMaxClients())
	{
		return pContext->ThrowNativeError("Client index %d is invalid.", index);
	}

	edict_t *pEdict = gamehelpers->EdictOfIndex(index);
	if (pEdict == NULL)
	{
		return pContext->ThrowNativeError("Edict for client %d is invalid.", index);
	}

	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(pEdict);
	if (pPlayer == NULL || !pPlayer->IsAuthorized())
	{
		return pContext->ThrowNativeError("CSteamID for client %d is not ready.", index);
	}

	const CSteamID *steamID = engine->GetClientSteamID(pEdict);
	if (steamID == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve CSteamID for client %d.", index);
	}
    
	return pServer->UserHasLicenseForApp(*steamID, params[2]);
}

static sp_nativeinfo_t gsnatives[] = {
	{"Steamworks_IsVACEnabled",				sm_IsVACEnabled},
	{"Steamworks_GetPublicIP",				sm_GetPublicIP},
	{"Steamworks_GetPublicIPCell",				sm_GetPublicIPCell},
	{"Steamworks_IsLoaded",				sm_IsLoaded},
	{"Steamworks_SetGameDescription",	sm_SetGameDescription},
	{"Steamworks_IsConnected",				sm_IsConnected},
	{"Steamworks_SetRule",						sm_SetRule},
	{"Steamworks_ClearRules",						sm_ClearRules},
	{"Steamworks_ForceHeartbeat",				sm_ForceHeartbeat},
	{"Steamworks_WasRestartRequested",				sm_WasRestartRequested},
	{"Steamworks_UserHasSubscription",				sm_UserHasSubscription},
	{NULL,											NULL}
};

SteamWorksGSNatives::SteamWorksGSNatives()
{
	sharesys->AddNatives(myself, gsnatives);
}

SteamWorksGSNatives::~SteamWorksGSNatives()
{
	/* We tragically can't remove ourselves... hopefully no one uses this class, you know, like a class. */
}
