// Copyright Epic Games, Inc. All Rights Reserved.

#include "FeelingBotGameMode.h"
#include "FeelingBotCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFeelingBotGameMode::AFeelingBotGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
