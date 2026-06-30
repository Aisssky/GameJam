#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConfirmDialogWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogAction);

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API UConfirmDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Dialog")
	void SetupDialog(const TMap<FString, FString>& Params);

	UPROPERTY(BlueprintAssignable, Category = "Dialog")
	FOnDialogAction OnConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "Dialog")
	FOnDialogAction OnCancelled;

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void Close();
};
