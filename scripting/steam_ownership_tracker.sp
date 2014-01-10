#pragma semicolon 1

#include <sourcemod>
#include <steamworks>

#define LOG_FILE "logs/filtered_accounts.txt"

#define L4D2_CHRISTMAS_DLC_APPID    271840

new Handle:hQueuedSteamIds;

new String:szLogFile[PLATFORM_MAX_PATH];

/**
    v1: Semi-broken Steamtools, no FS detection, hooks unhooking for no reason
    v2: Half-functioning Steamtools and reliable Steamworks; something causes crashes during map changes
    v3: Fully functional and reliable Steamworks-L4D2 fork
**/

public Plugin:myinfo =
{
    name = "Steam Ownership Tracker",
    description = "Detect 'Family Sharing' and 'Free-to-Buy' players",
    author = "Visor",
    version = "3.1",
    url = "https://github.com/Attano/SteamWorks"
};

public OnPluginStart()
{
    BuildPath(Path_SM, szLogFile, sizeof(szLogFile), LOG_FILE);

    hQueuedSteamIds = CreateArray(64);
}

public Steamworks_OnServersConnectFailure()
{
    LogError("Steamworks: Failed to establish connection to Steam @SteamWorks::SteamServersConnectFailure()");
}

// This should be firing after our forwards
public OnClientAuthorized(client, const String:auth[])
{
    if (!Steamworks_IsConnected())
    {
        LogError("Steamworks: No Steam Connection!");
        return;
    }

    // Kick for Family Sharing
    new index = FindStringInArray(hQueuedSteamIds, auth);
    if (index != -1)
    {
        // Failsafe
        decl String:steamID[32];
        GetArrayString(hQueuedSteamIds, index, steamID, sizeof(steamID));
        if (StrEqual(steamID, auth))
        {
            KickClient(client, "Please buy the game");
        }
    }

    // Kick for being a free-to-buy player(sounds racist tho)
    if (Steamworks_UserHasSubscription(client, L4D2_CHRISTMAS_DLC_APPID) == k_EUserHasLicenseResultHasLicense)
    {
        LogToFileEx(szLogFile, "Detected free-to-buy subscription on %L", client);
        KickClient(client, "Please go away");
    }
}

public Steamworks_OnFamilySharingDetected(const String:steamID[], const String:ownerSteamID[])
{
    PushArrayString(hQueuedSteamIds, steamID);
    LogToFileEx(szLogFile, "Detected Family Sharing on <%s> [owner <%s>]", steamID, ownerSteamID);
}