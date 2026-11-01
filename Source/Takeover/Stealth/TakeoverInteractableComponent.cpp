#include "Stealth/TakeoverInteractableComponent.h"

void UTakeoverInteractableComponent::Interact(ATakeoverCharacter* Character)
{
	if (bEnabled)
	{
		OnInteract.Broadcast(Character);
	}
}
