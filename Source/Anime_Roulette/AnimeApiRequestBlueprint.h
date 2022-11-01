// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Http.h"
#include "Json.h"
#include "AnimeApiRequestBlueprint.generated.h"

USTRUCT(BlueprintType)
struct ANIME_ROULETTE_API FTagInfo
{
	GENERATED_BODY()

public:
	FTagInfo() {}
	FTagInfo(const int& _Id, const FString& _Name) {
		Id = _Id;
		Name = _Name;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;
};

USTRUCT(BlueprintType)
struct ANIME_ROULETTE_API FAnimeInfo
{
	GENERATED_BODY()

public:
	FAnimeInfo() {}
	FAnimeInfo(const int& _Id) {
		Id = _Id;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ImageUrl;
};

USTRUCT(BlueprintType)
struct ANIME_ROULETTE_API FSearchParams
{
	GENERATED_BODY()

public:
	FSearchParams() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTagInfo> IncludedTags;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTagInfo> ExcludedTags;
};

UCLASS()
class ANIME_ROULETTE_API UAnimeApiRequestBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Tags", CompactNodeTitle = "GetTags"), Category = "Anime API Requests")
	static TArray<FTagInfo> GetTags();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Results", CompactNodeTitle = "GetResults"), Category = "Anime API Requests")
	static TArray<FAnimeInfo> GetResults();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ask Results", CompactNodeTitle = "AskResults"), Category = "Anime API Requests")
	static bool AskResults(const FSearchParams& SearchParams);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clear Tags", CompactNodeTitle = "ClearTags"), Category = "Anime API Requests")
	static void ClearTags();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clear Results", CompactNodeTitle = "ClearResults"), Category = "Anime API Requests")
	static void ClearResults();

private:
	static FString TagsURL;
	static FString SearchURL;
	static FString LastSearchURL;
	static int PageCount;
	static int CurrentPage;
	static TArray<FTagInfo> Tags;
	static TArray<FAnimeInfo> AnimeResults;

	static void ParseJsonString(const FString& JsonResponse, TArray<TSharedPtr<FJsonValue>>& OutData, TArray<TSharedPtr<FJsonValue>>& OutPagination);
	static void SendRequest(const FString& URL, void (*function)(FHttpRequestPtr, FHttpResponsePtr, bool));
	static void OnTagsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	static void OnResultsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
};
