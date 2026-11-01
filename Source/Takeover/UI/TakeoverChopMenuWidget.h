#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "TakeoverChopMenuWidget.generated.h"

class UTakeoverChopMenuWidget;
class UTextBlock;
class UVerticalBox;

/** A button that tells the chop menu which part it stands for. INDEX_NONE is the Finish button. */
UCLASS()
class TAKEOVER_API UTakeoverChopMenuButton : public UButton
{
	GENERATED_BODY()

public:
	void Bind(UTakeoverChopMenuWidget* InMenu, int32 InPartIndex);

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> Label;

private:
	UFUNCTION()
	void HandleClicked();

	TWeakObjectPtr<UTakeoverChopMenuWidget> Menu;
	int32 PartIndex = INDEX_NONE;
};

/**
 * The chop shop menu: one button per part, a status line, and Finish. It builds its own widget tree in C++, so
 * it needs no Widget Blueprint; a Widget Blueprint subclass can still replace the look later.
 * Reads and drives UMissionSubsystem's chop session.
 */
UCLASS()
class TAKEOVER_API UTakeoverChopMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void HandleButton(int32 PartIndex);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Chop", meta = (ClampMin = "8"))
	int32 FontSize = 24;

private:
	UTakeoverChopMenuButton* AddButton(UVerticalBox* Box, int32 PartIndex);
	UTextBlock* MakeText(const FText& Text);
	void Refresh();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTakeoverChopMenuButton>> PartButtons;

	UPROPERTY(Transient)
	TObjectPtr<UTakeoverChopMenuButton> FinishButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;
};
