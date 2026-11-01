#include "Stealth/TakeoverKeySpot.h"

#include "Components/StaticMeshComponent.h"
#include "Player/TakeoverCharacter.h"
#include "Stealth/TakeoverInteractableComponent.h"

#define LOCTEXT_NAMESPACE "TakeoverKeySpot"

ATakeoverKeySpot::ATakeoverKeySpot()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotMesh"));
	SpotMesh->SetupAttachment(Root);

	Interactable = CreateDefaultSubobject<UTakeoverInteractableComponent>(TEXT("Interactable"));
	Interactable->SetupAttachment(Root);
	Interactable->Prompt = LOCTEXT("Search", "Search");
}

void ATakeoverKeySpot::BeginPlay()
{
	Super::BeginPlay();

	Interactable->OnInteract.AddUniqueDynamic(this, &ATakeoverKeySpot::HandleSearch);
}

void ATakeoverKeySpot::ResetSearch(bool bInHasKeys)
{
	bHasKeys = bInHasKeys;
	Interactable->bEnabled = true;
}

void ATakeoverKeySpot::HandleSearch(ATakeoverCharacter* Character)
{
	Interactable->bEnabled = false;
	if (!Character)
	{
		return;
	}

	if (bHasKeys)
	{
		bHasKeys = false;
		Character->GiveKeys();
		Character->ShowHint(LOCTEXT("Found", "Found the car keys."));
	}
	else
	{
		Character->ShowHint(LOCTEXT("Empty", "Nothing here."));
	}
}

#undef LOCTEXT_NAMESPACE
