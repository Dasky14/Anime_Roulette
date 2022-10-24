// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ApiEventManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventDelegate_OnTagsUpdated);

/**
 * 
 */
UCLASS()
class ANIME_ROULETTE_API UApiEventManager : public UObject
{
	GENERATED_BODY()

public:
	~UApiEventManager();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get EventManager Instance", CompactNodeTitle = "GetEMInstance"), Category = "Anime API Events")
	static UApiEventManager* GetInstance();

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Anime API Events")
	FEventDelegate_OnTagsUpdated OnTagsUpdated;

private:
	static UApiEventManager* Instance;
};