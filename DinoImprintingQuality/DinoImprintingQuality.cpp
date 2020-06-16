#include <API/ARK/Ark.h>
#include <fstream>
#include "json.hpp"
#pragma comment(lib, "ArkApi.lib")

DECLARE_HOOK(APrimalDinoCharacter_UpdateImprintingQuality_Imp, void, APrimalDinoCharacter*, float);
nlohmann::json json;
float DinoImprintingQuality = 0;

FString GetText(const std::string& str)
{
	return FString(ArkApi::Tools::Utf8Decode(json.value(str, str)));
}

void Hook_APrimalDinoCharacter_UpdateImprintingQuality_Imp(APrimalDinoCharacter* dino, float Amount)
{
	if (dino->bIsBaby()())
	{
		float oldDinoImprintingQuality = dino->GetCharacterStatusComponent()->DinoImprintingQualityField();
		Amount = oldDinoImprintingQuality + DinoImprintingQuality / 100;
		if (Amount > 1)
			Amount = 1;
		AShooterPlayerController* player = ArkApi::GetApiUtils().FindPlayerFromSteamId(ArkApi::GetApiUtils().GetSteamIDForPlayerID(dino->ImprinterPlayerDataIDField()));
		if (player)
			ArkApi::GetApiUtils().SendChatMessage(player, GetText("Sender"), *GetText("Message"), DinoImprintingQuality);
		//dino->AddedImprintingQuality(Amount);
	}
	APrimalDinoCharacter_UpdateImprintingQuality_Imp_original(dino, Amount);
}

void ReloadConfigRcon(RCONClientConnection* rcon_connection, RCONPacket* rcon_packet, UWorld* /*unused*/)
{
	FString reply;
	reply = L"ReloadConfigRcon";
	rcon_connection->SendMessageW(rcon_packet->Id, 0, &reply);
}
void Load()
{
	Log::Get().Init("DinoImprintingQuality1.2");
	try
	{
		std::ifstream configfile(ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/DinoImprintingQuality/config.json");
		if (configfile.is_open())
		{
			configfile >> json;
			configfile.close();
		}
		else
		{
			Log::GetLog()->error("Could not open file config.json");
			configfile.close();
			return;
		}

	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error(error.what());
		return;
	}
	DinoImprintingQuality = json.value("SetImprintingQuality",0.0f);
	if (DinoImprintingQuality > 0)
		ArkApi::GetHooks().SetHook("APrimalDinoCharacter.UpdateImprintingQuality",
		&Hook_APrimalDinoCharacter_UpdateImprintingQuality_Imp,
		&APrimalDinoCharacter_UpdateImprintingQuality_Imp_original);

	ArkApi::GetCommands().AddRconCommand("DinoImprintingQuality.reload", &ReloadConfigRcon);
}

void Unload()
{
	ArkApi::GetHooks().DisableHook("APrimalDinoCharacter.UpdateImprintingQuality",
		&Hook_APrimalDinoCharacter_UpdateImprintingQuality_Imp);
	ArkApi::GetCommands().RemoveRconCommand("DinoImprintingQuality.reload");
}
BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Load();
		break;
	case DLL_PROCESS_DETACH:
		Unload();
		break;
	}
	return TRUE;
}