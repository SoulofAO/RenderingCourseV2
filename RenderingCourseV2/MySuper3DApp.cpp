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

GameConfigurator BuildSplitScreenValidationConfigurator()
{
	GameConfigurator SplitScreenValidationConfigurator = {};
	SplitScreenValidationConfigurator.SessionName = L"SplitScreenValidationSession";
	SplitScreenValidationConfigurator.ReceiveInputWhenSessionIsInactive = true;
	SplitScreenValidationConfigurator.Games.push_back(GameDefinition{ 4001, BuildLightingTestGame });
	SplitScreenValidationConfigurator.Games.push_back(GameDefinition{ 4002, BuildParticleTestGame });
	SplitScreenValidationConfigurator.Games.push_back(GameDefinition{ 4003, BuildKatamaryTaskGame });
	SplitScreenValidationConfigurator.Games.push_back(GameDefinition{ 4004, BuildMeshTestGame });
	SplitScreenValidationConfigurator.Players.push_back(PlayerBinding{ 21, 0, 0, ViewportRectangleNormalized{ 0.0f, 0.0f, 0.5f, 0.5f }, 4001 });
	SplitScreenValidationConfigurator.Players.push_back(PlayerBinding{ 22, 1, 0, ViewportRectangleNormalized{ 0.5f, 0.0f, 0.5f, 0.5f }, 4002 });
	SplitScreenValidationConfigurator.Players.push_back(PlayerBinding{ 23, 2, 0, ViewportRectangleNormalized{ 0.0f, 0.5f, 0.5f, 0.5f }, 4003 });
	SplitScreenValidationConfigurator.Players.push_back(PlayerBinding{ 24, 3, 0, ViewportRectangleNormalized{ 0.5f, 0.5f, 0.5f, 0.5f }, 4004 });
	return SplitScreenValidationConfigurator;
}

int main()
{
	GameInstance RuntimeGameInstance;
	RuntimeGameInstance.Initialize(L"My3DApp SingleSession", 1280, 720);

	GameConfigurator SingleGameSinglePlayerConfigurator = {}; //BuildSplitScreenValidationConfigurator();
	SingleGameSinglePlayerConfigurator.SessionName = L"SingleGameSinglePlayerSession";
	SingleGameSinglePlayerConfigurator.ReceiveInputWhenSessionIsInactive = true;
	SingleGameSinglePlayerConfigurator.Games.push_back(GameDefinition{ 1001, BuildPlanetsTaskGame });
	SingleGameSinglePlayerConfigurator.Players.push_back(PlayerBinding{ 1, 0, 0, ViewportRectangleNormalized{ 0.0f, 0.0f, 1.0f, 1.0f }, 1001 });

	RuntimeGameInstance.OpenMultipleGames(std::vector<GameConfigurator>{ SingleGameSinglePlayerConfigurator });
	RuntimeGameInstance.Run();
	return 0;
}
