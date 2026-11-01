#include "UI/TakeoverChopMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Mission/MissionSubsystem.h"

#define LOCTEXT_NAMESPACE "TakeoverChopMenu"

namespace
{
	UMissionSubsystem* ChopMenuMission(const UUserWidget& Widget)
	{
		const UWorld* World = Widget.GetWorld();
		return World ? World->GetSubsystem<UMissionSubsystem>() : nullptr;
	}
}

void UTakeoverChopMenuButton::Bind(UTakeoverChopMenuWidget* InMenu, int32 InPartIndex)
{
	Menu = InMenu;
	PartIndex = InPartIndex;
	OnClicked.AddUniqueDynamic(this, &UTakeoverChopMenuButton::HandleClicked);
}

void UTakeoverChopMenuButton::HandleClicked()
{
	if (UTakeoverChopMenuWidget* MenuWidget = Menu.Get())
	{
		MenuWidget->HandleButton(PartIndex);
	}
}

TSharedRef<SWidget> UTakeoverChopMenuWidget::RebuildWidget()
{
	// A Widget Blueprint subclass brings its own tree; only build one when there is none
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
		Background->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.8f));
		Background->SetPadding(FMargin(80.f));
		WidgetTree->RootWidget = Background;

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Rows"));
		Background->SetContent(Box);

		Box->AddChildToVerticalBox(MakeText(LOCTEXT("Title", "Chop shop")));

		PartButtons.Reset();
		if (const UMissionSubsystem* Mission = ChopMenuMission(*this))
		{
			for (int32 Index = 0; Index < Mission->GetChopSession().GetParts().Num(); ++Index)
			{
				PartButtons.Add(AddButton(Box, Index));
			}
		}

		StatusText = MakeText(FText::GetEmpty());
		Box->AddChildToVerticalBox(StatusText);

		FinishButton = AddButton(Box, INDEX_NONE);
		FinishButton->Label->SetText(LOCTEXT("Finish", "Sell and finish"));
	}

	return Super::RebuildWidget();
}

void UTakeoverChopMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Refresh();

	// Gamepad navigation needs a focused button to start from
	UButton* First = PartButtons.IsEmpty() ? FinishButton.Get() : PartButtons[0].Get();
	if (First)
	{
		First->SetKeyboardFocus();
	}
}

void UTakeoverChopMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Refresh();
}

UTakeoverChopMenuButton* UTakeoverChopMenuWidget::AddButton(UVerticalBox* Box, int32 PartIndex)
{
	UTakeoverChopMenuButton* Button = WidgetTree->ConstructWidget<UTakeoverChopMenuButton>(UTakeoverChopMenuButton::StaticClass());
	Button->Label = MakeText(FText::GetEmpty());
	Button->SetContent(Button->Label);
	Button->Bind(this, PartIndex);

	if (UVerticalBoxSlot* RowSlot = Box->AddChildToVerticalBox(Button))
	{
		RowSlot->SetPadding(FMargin(0.f, 6.f));
	}
	return Button;
}

UTextBlock* UTakeoverChopMenuWidget::MakeText(const FText& Text)
{
	UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Block->SetText(Text);
	FSlateFontInfo Font = Block->GetFont();
	Font.Size = FontSize;
	Block->SetFont(Font);
	return Block;
}

void UTakeoverChopMenuWidget::HandleButton(int32 PartIndex)
{
	UMissionSubsystem* Mission = ChopMenuMission(*this);
	if (!Mission)
	{
		return;
	}

	if (PartIndex == INDEX_NONE)
	{
		Mission->FinishChop();
	}
	else
	{
		Mission->StartChopRemoval(PartIndex);
	}
}

void UTakeoverChopMenuWidget::Refresh()
{
	const UMissionSubsystem* Mission = ChopMenuMission(*this);
	if (!Mission || !StatusText)
	{
		return;
	}

	const FTakeoverChopSession& Session = Mission->GetChopSession();
	const TArray<FTakeoverChopPart>& Parts = Session.GetParts();
	for (int32 Index = 0; Index < PartButtons.Num() && Index < Parts.Num(); ++Index)
	{
		const FTakeoverChopPart& Part = Parts[Index];
		FText Label;
		if (Part.bRemoved)
		{
			Label = FText::Format(LOCTEXT("Removed", "{0}: removed (${1})"), Part.DisplayName, Part.Value);
		}
		else if (Session.GetRemovingIndex() == Index)
		{
			Label = FText::Format(LOCTEXT("Removing", "{0}: removing {1}%"), Part.DisplayName, FMath::RoundToInt(Session.GetRemovalProgress() * 100.f));
		}
		else
		{
			Label = FText::Format(LOCTEXT("Part", "{0}: ${1}"), Part.DisplayName, Part.Value);
		}
		PartButtons[Index]->Label->SetText(Label);
		PartButtons[Index]->SetIsEnabled(!Part.bRemoved && !Session.IsRemoving());
	}

	StatusText->SetText(Parts.IsEmpty()
		? LOCTEXT("NoParts", "Nothing to strip: the mission config has no chop parts table.")
		: FText::Format(LOCTEXT("Payout", "Parts sold so far: ${0}"), Session.GetPayout()));
}

#undef LOCTEXT_NAMESPACE
