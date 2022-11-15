// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ApiEventManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventDelegate_OnTagsUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventDelegate_OnResultsUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventDelegate_OnProgressUpdated, int, CurrentPage, int, PageCount);

/**
 * 
 */
UCLASS(Blueprintable)
class ANIME_ROULETTE_API UApiEventManager : public UObject
{
	GENERATED_BODY()

public:
	~UApiEventManager();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get EventManager Instance", CompactNodeTitle = "GetEMInstance"), Category = "Anime API Events")
	static UApiEventManager* GetInstance();

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Anime API Events")
	FEventDelegate_OnTagsUpdated OnTagsUpdated;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Anime API Events")
	FEventDelegate_OnResultsUpdated OnResultsUpdated;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Anime API Events")
	FEventDelegate_OnProgressUpdated OnProgressUpdated;

private:
	static UApiEventManager* Instance;
};
