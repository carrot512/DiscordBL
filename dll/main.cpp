#include "stdafx.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <stdio.h>

#include "discord-rpc.h"
#include "discord-register.h"
#include "torque.h"

#define FORMAT "DiscordBL::%s(%s);"
#define APP_ID "481965315715235851"
#define NU_ARG "ptlaaxobimwroe"
#define STEAM_ID "250340"
#define BL_VER 21

#define WIN32_LEAN_AND_MEAN

static void RunCallbacks(DWORD *obj, int argc, const char** argv)
{
	Discord_RunCallbacks();
}

static void OnReady(const DiscordUser* user)
{
	Printf("DiscordBL | Connected to user %s#%s - %s",
		user->username,
		user->discriminator,
		user->userId);
}

static void OnDisconnected(int code, const char* message)
{
	Printf("DiscordBL | Disconnected from Rich Presence (%d: %s)", code, message);
}

static void OnError(int code, const char* message)
{
	Printf("DiscordBL | Error occured! (%d: %s)", code, message);
}

static void OnJoin(const char* secret)
{
	Printf("DiscordBL | Join event occured (%s)", secret);

	char buffer[512];
	sprintf_s(buffer, FORMAT, "JoinServer", secret);
	Eval(buffer, false, nullptr);
}

static void OnSpectate(const char* secret)
{
	Printf("DiscordBL | Spectate event occured (%s)", secret);
}

static void OnJoinRequest(const DiscordUser* request)
{
	Printf("DiscordBL | Join request event occured from %s#%s - %s",
		request->username,
		request->discriminator,
		request->userId);
	
	std::string arguments = request->username;
	arguments = arguments + "#" + request->discriminator + ", " + request->userId;

	char buffer[512];
	sprintf_s(buffer, FORMAT, "DecideJoinRequest", arguments.c_str());
	Eval(buffer, false, nullptr);
}

static void DiscordInitialize(DWORD *obj, int argc, const char** argv)
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = OnReady;
	handlers.disconnected = OnDisconnected;
	handlers.errored = OnError;
	handlers.joinGame = OnJoin;
	handlers.spectateGame = OnSpectate;
	handlers.joinRequest = OnJoinRequest;
	Discord_Initialize(APP_ID, &handlers, 1, NULL); // STEAM_ID
	Discord_RunCallbacks();

	Printf("DiscordBL | Discord initialized");
}

static void Register(DWORD *obj, int argc, const char** argv)
{
	Discord_Register(APP_ID, NU_ARG);
	// Discord_RegisterSteamGame(APP_ID, STEAM_ID);
	Eval("$Pref::DiscordBL::GameRegistered = true;", true, NULL);
	Printf("DiscordBL | Registered game");
}

static void RequestReply(DWORD *obj, int argc, const char** argv)
{
	Discord_Respond(argv[0], atoi(argv[1]));
}

static void Shutdown(DWORD *obj, int argc, const char** argv)
{
	Printf("DiscordBL | Shutting down Discord RPC");
	Discord_Shutdown();
}

static void UpdatePresence(DWORD* obj, int argc, const char** argv)
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = argv[1];
	discordPresence.largeImageText = argv[2];
	discordPresence.largeImageKey = "logo";

	if (!_stricmp(argv[3], "true") || !_stricmp(argv[3], "1") || (0 != atoi(argv[3])))
	{
		std::string title = argv[7];
		title[0] = toupper(title[0]);
		
		discordPresence.details = argv[4];
		discordPresence.partySize = atoi(argv[5]);
		discordPresence.partyMax = atoi(argv[6]);
		discordPresence.smallImageKey = argv[7];
		discordPresence.smallImageText = title.c_str();
		// discordPresence.joinSecret = argv[8]; JOINING IS NOT FINISHED!!!! IT DOES NOT WORK!!!
		discordPresence.partyId = argv[9];
	}

	Discord_UpdatePresence(&discordPresence);
	Printf("DiscordBL | Rich Presence updated");
}

// Begin entry point
bool Init()
{
	if (!InitTorque(BL_VER))
	{
		return false;
	}
	
	ConsoleFunction("DiscordBL", "Initialize", DiscordInitialize,
		"DiscordBL::Initialize() - Initializes the Discord RPC connection. Only call this once.", 0, 0);
	ConsoleFunction("DiscordBL", "Register", Register,
		"DiscordBL::Register() - Registers the game to Discord.", 0, 0);
	ConsoleFunction("DiscordBL", "UpdatePresence", UpdatePresence,
		"DiscordBL::UpdatePresence(string details, string playerName, bool additionalDetails, string serverName, int players, int maxPlayers, string status, string partyKey, string partyId) - Updates the Rich Presence.", 4, 10);
	ConsoleFunction("DiscordBL", "RunCallbacks", RunCallbacks,
		"DiscordBL::RunCallbacks() - Runs Discord's callbacks.", 0, 0);
	ConsoleFunction("DiscordBL", "RequestReply", RequestReply,
		"DiscordBL::RequestReply(int userId, int response) - Responds to a join request. Response codes: 0 = DISCORD_REPLY_NO, 1 = DISCORD_REPLY_YES, 2 = DISCORD_REPLY_IGNORE", 0, 3);
	ConsoleFunction("DiscordBL", "Shutdown", Shutdown,
		"DiscordBL::Shutdown() - Shuts down the Discord RPC.", 0, 0);
	
	Printf("DiscordBL | DLL loaded");
	return true;
}

bool Detach()
{
	Discord_Shutdown();
	return true;
}

// Entry point
int __stdcall DllMain(HINSTANCE hInstance, unsigned long reason, void *reserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
			return Init();

		case DLL_PROCESS_DETACH:
			return Detach();

		default:
			return true;
	}
}