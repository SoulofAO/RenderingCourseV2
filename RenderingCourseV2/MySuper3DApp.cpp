#include "Abstracts/Core/GameConfigurator.h"
#include "Abstracts/Core/GameInstance.h"
#include "FirstTask/FirstTaskGame.h"
#include "KatamaryTask/KatamaryGame.h"
#include "Planets/PlanetsGame.h"
#include "PingPongTask/PingPongGame.h"
#include "Abstracts/Core/SelectionGame.h"
#include "Tests/LightingTestGame.h"
#include "Tests/MeshTestGame.h"
#include "Tests/ParticleTestGame.h"
#include "Tests/PhysicsTestGame.h"
#include "Tests/TexturingTestGame.h"
#include <memory>

constexpr int SelectionGameIdentifier = 1000;
constexpr int FirstTaskGameIdentifier = 1001;
constexpr int PingPongGameIdentifier = 1002;
constexpr int PlanetsGameIdentifier = 1003;
constexpr int PhysicsTestGameIdentifier = 1004;
constexpr int MeshTestGameIdentifier = 1005;
constexpr int TexturingTestGameIdentifier = 1006;
constexpr int LightingTestGameIdentifier = 1007;
constexpr int ParticleTestGameIdentifier = 1008;
constexpr int KatamaryTaskGameIdentifier = 1009;
constexpr int SingleSessionIdentifier = 1;
constexpr int SinglePlayerIdentifier = 1;

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

GameConfigurator BuildSinglePlayerGameConfigurator(
	const wchar_t* SessionName,
	int GameIdentifier,
	const std::function<std::unique_ptr<Game>()>& GameFactory)
{
	GameConfigurator NewGameConfigurator = {};
	NewGameConfigurator.SessionName = SessionName;
	NewGameConfigurator.ReceiveInputWhenSessionIsInactive = true;
	NewGameConfigurator.Games.push_back(GameDefinition{ GameIdentifier, GameFactory });
	NewGameConfigurator.Players.push_back(PlayerBinding{ SinglePlayerIdentifier, 0, 0, ViewportRectangleNormalized{ 0.0f, 0.0f, 1.0f, 1.0f }, GameIdentifier });
	return NewGameConfigurator;
}

GameConfigurator BuildTwoPlayerSplitScreenGameConfigurator(
	const wchar_t* SessionName,
	int GameIdentifier,
	const std::function<std::unique_ptr<Game>()>& GameFactory)
{
	GameConfigurator NewGameConfigurator = {};
	NewGameConfigurator.SessionName = SessionName;
	NewGameConfigurator.ReceiveInputWhenSessionIsInactive = true;
	NewGameConfigurator.Games.push_back(GameDefinition{ GameIdentifier, GameFactory });
	NewGameConfigurator.Players.push_back(PlayerBinding{ 1, 0, 0, ViewportRectangleNormalized{ 0.0f, 0.0f, 0.5f, 1.0f }, GameIdentifier });
	NewGameConfigurator.Players.push_back(PlayerBinding{ 2, 1, 0, ViewportRectangleNormalized{ 0.5f, 0.0f, 0.5f, 1.0f }, GameIdentifier });
	return NewGameConfigurator;
}

std::unique_ptr<Game> BuildPhysicsTestGame();
std::unique_ptr<Game> BuildMeshTestGame();
std::unique_ptr<Game> BuildTexturingTestGame();
std::unique_ptr<Game> BuildLightingTestGame();
std::unique_ptr<Game> BuildParticleTestGame();
std::unique_ptr<Game> BuildKatamaryTaskGame();

std::unique_ptr<Game> BuildSelectionGame()
{
	std::unique_ptr<SelectionGame> NewSelectionGame = std::make_unique<SelectionGame>(L"My3DApp Game Selection", 1280, 720);
	NewSelectionGame->SetSelectionContext(SingleSessionIdentifier, SinglePlayerIdentifier);
	NewSelectionGame->SetAvailableGameSelections(std::vector<SelectionGameEntry>
		{
			SelectionGameEntry{ "First Task Game", BuildSinglePlayerGameConfigurator(L"FirstTaskSession", FirstTaskGameIdentifier, BuildFirstTaskGame) },
			SelectionGameEntry{ "PingPong Game", BuildSinglePlayerGameConfigurator(L"PingPongSession", PingPongGameIdentifier, BuildPingPongTaskGame) },
			SelectionGameEntry{ "Planets Game", BuildSinglePlayerGameConfigurator(L"PlanetsSession", PlanetsGameIdentifier, BuildPlanetsTaskGame) },
			SelectionGameEntry{ "Physics Test Game", BuildSinglePlayerGameConfigurator(L"PhysicsTestSession", PhysicsTestGameIdentifier, BuildPhysicsTestGame) },
			SelectionGameEntry{ "Mesh Test Game", BuildSinglePlayerGameConfigurator(L"MeshTestSession", MeshTestGameIdentifier, BuildMeshTestGame) },
			SelectionGameEntry{ "Texturing Test Game", BuildSinglePlayerGameConfigurator(L"TexturingTestSession", TexturingTestGameIdentifier, BuildTexturingTestGame) },
			SelectionGameEntry{ "Lighting Test Game", BuildSinglePlayerGameConfigurator(L"LightingTestSession", LightingTestGameIdentifier, BuildLightingTestGame) },
			SelectionGameEntry{ "Particle Test Game", BuildSinglePlayerGameConfigurator(L"ParticleTestSession", ParticleTestGameIdentifier, BuildParticleTestGame) },
			SelectionGameEntry{ "Katamary Game", BuildSinglePlayerGameConfigurator(L"KatamarySession", KatamaryTaskGameIdentifier, BuildKatamaryTaskGame) },
			SelectionGameEntry{ "Planets SplitScreen 2 Players", BuildTwoPlayerSplitScreenGameConfigurator(L"PlanetsSplitScreenSession", PlanetsGameIdentifier, BuildPlanetsTaskGame) }
		});
	return NewSelectionGame;
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
	SingleGameSinglePlayerConfigurator.Games.push_back(GameDefinition{ SelectionGameIdentifier, BuildSelectionGame });
	SingleGameSinglePlayerConfigurator.Players.push_back(PlayerBinding{ SinglePlayerIdentifier, 0, 0, ViewportRectangleNormalized{ 0.0f, 0.0f, 1.0f, 1.0f }, SelectionGameIdentifier });
	
	RuntimeGameInstance.OpenMultipleGames(std::vector<GameConfigurator>{ SingleGameSinglePlayerConfigurator });
	RuntimeGameInstance.Run();
	return 0;
}
