#include "Abstracts/Core/GameConfigurator.h"
#include "Abstracts/Core/GameInstance.h"
#include "FirstTask/FirstTaskGame.h"
#include "KatamaryTask/KatamaryGame.h"
#include "Planets/PlanetsGame.h"
#include "PingPongTask/PingPongGame.h"
#include "Tests/LightingTestGame.h"
#include "Tests/MeshTestGame.h"
#include "Tests/ParticleTestGame.h"
#include "Tests/PhysicsTestGame.h"
#include "Tests/TexturingTestGame.h"
#include <memory>

std::unique_ptr<Game> BuildFirstTaskGame()
{
	return std::make_unique<FirstTaskGame>(L"My3DApp", 800, 800);
}

std::unique_ptr<Game> BuildPingPongTaskGame()
{
	return std::make_unique<PingPongGame>(L"My3DApp PingPong", 1200, 800);
}

std::unique_ptr<Game> BuildPlanetsTaskGame()
{
	return std::make_unique<PlanetsGame>(L"My3DApp Planets", 1280, 720);
}

std::unique_ptr<Game> BuildPhysicsTestGame()
{
	return std::make_unique<PhysicsTestGame>(L"My3DApp PhysicsTest", 1280, 720);
}

std::unique_ptr<Game> BuildMeshTestGame()
{
	return std::make_unique<MeshTestGame>(L"My3DApp MeshTest", 1280, 720);
}

std::unique_ptr<Game> BuildTexturingTestGame()
{
	return std::make_unique<TexturingTestGame>(L"My3DApp TexturingTest", 1280, 720);
}

std::unique_ptr<Game> BuildLightingTestGame()
{
	return std::make_unique<LightingTestGame>(L"My3DApp LightingTest", 1280, 720);
}

std::unique_ptr<Game> BuildParticleTestGame()
{
	return std::make_unique<ParticleTestGame>(L"My3DApp ParticleTest", 1280, 720);
}

std::unique_ptr<Game> BuildKatamaryTaskGame()
{
	return std::make_unique<KatamaryGame>(L"My3DApp KatamaryTask", 1280, 720);
}

int main()
{
	GameInstance RuntimeGameInstance;
	RuntimeGameInstance.Initialize(L"My3DApp MultiSession", 1280, 720);

	GameConfigurator SharedGameForTwoPlayersConfigurator = {};
	SharedGameForTwoPlayersConfigurator.SessionName = L"SharedGameForTwoPlayersSession";
	SharedGameForTwoPlayersConfigurator.ReceiveInputWhenSessionIsInactive = true;
	SharedGameForTwoPlayersConfigurator.Games.push_back(GameDefinition{ 1001, BuildPlanetsTaskGame });
	SharedGameForTwoPlayersConfigurator.Players.push_back(PlayerBinding{ 1, 0, 0, ViewportRectangleNormalized{ 0.0f, 0.0f, 0.5f, 1.0f }, 1001 });
	SharedGameForTwoPlayersConfigurator.Players.push_back(PlayerBinding{ 2, 1, 1, ViewportRectangleNormalized{ 0.5f, 0.0f, 0.5f, 1.0f }, 1001 });

	GameConfigurator TwoPlayersTwoGamesConfigurator = {};
	TwoPlayersTwoGamesConfigurator.SessionName = L"TwoPlayersTwoGamesSession";
	TwoPlayersTwoGamesConfigurator.ReceiveInputWhenSessionIsInactive = true;
	TwoPlayersTwoGamesConfigurator.Games.push_back(GameDefinition{ 2001, BuildKatamaryTaskGame });
	TwoPlayersTwoGamesConfigurator.Games.push_back(GameDefinition{ 2002, BuildLightingTestGame });
	TwoPlayersTwoGamesConfigurator.Players.push_back(PlayerBinding{ 11, 0, 0, ViewportRectangleNormalized{ 0.0f, 0.0f, 0.5f, 0.5f }, 2001 });
	TwoPlayersTwoGamesConfigurator.Players.push_back(PlayerBinding{ 12, 1, 0, ViewportRectangleNormalized{ 0.5f, 0.5f, 0.5f, 0.5f }, 2002 });

	GameConfigurator GameWithoutPlayersConfigurator = {};
	GameWithoutPlayersConfigurator.SessionName = L"GameWithoutPlayersSession";
	GameWithoutPlayersConfigurator.ReceiveInputWhenSessionIsInactive = true;
	GameWithoutPlayersConfigurator.Games.push_back(GameDefinition{ 3001, BuildMeshTestGame });

	std::vector<GameConfigurator> GameConfigurators;
	GameConfigurators.push_back(SharedGameForTwoPlayersConfigurator);
	GameConfigurators.push_back(TwoPlayersTwoGamesConfigurator);
	GameConfigurators.push_back(GameWithoutPlayersConfigurator);
	RuntimeGameInstance.OpenMultipleGames(GameConfigurators);
	RuntimeGameInstance.Run();
	return 0;
}
