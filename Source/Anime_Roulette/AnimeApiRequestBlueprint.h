// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Http.h"
#include "Json.h"
#include "AnimeApiRequestBlueprint.generated.h"

/**
 * 
 */
UCLASS()
class ANIME_ROULETTE_API UAnimeApiRequestBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Tags", CompactNodeTitle = "GetTags"), Category = "Anime API Requests")
	static TArray<FString> GetTags();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clear Tags", CompactNodeTitle = "ClearTags"), Category = "Anime API Requests")
	static void ClearTags();

private:
	static FString Tags_URL;
	static TArray<FString> Tags;

	static TArray<TSharedPtr<FJsonValue>> ParseJsonString(FString JsonResponse);
	static void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
};
