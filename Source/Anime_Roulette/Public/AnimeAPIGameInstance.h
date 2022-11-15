// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Http.h"
#include "Json.h"
#include "AnimeAPIGameInstance.generated.h"

enum RequestType { Tags, Results };

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MinScore;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MaxScore;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Status;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Rating;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString OrderBy;
};

/**
 * 
 */
UCLASS()
class ANIME_ROULETTE_API UAnimeAPIGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UAnimeAPIGameInstance();

public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Tags", CompactNodeTitle = "GetTags"), Category = "Anime API Requests")
	TArray<FTagInfo> GetTags();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Results", CompactNodeTitle = "GetResults"), Category = "Anime API Requests")
	TArray<FAnimeInfo> GetResults();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Random Result", CompactNodeTitle = "GetRandomResult"), Category = "Anime API Requests")
	FAnimeInfo GetRandomResult();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Result Count", CompactNodeTitle = "GetResultCount"), Category = "Anime API Requests")
	int GetResultCount();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ask Results", CompactNodeTitle = "AskResults"), Category = "Anime API Requests")
	bool AskResults(const FSearchParams& SearchParams);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clear Tags", CompactNodeTitle = "ClearTags"), Category = "Anime API Requests")
	void ClearTags();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clear Results", CompactNodeTitle = "ClearResults"), Category = "Anime API Requests")
	void ClearResults();

private:
	FString TagsURL;
	FString SearchURL;
	FString LastSearchURL;
	int PageCount;
	int CurrentPage;
	double LastRateLimitReset;
	TArray<FTagInfo> TagsList;
	TArray<FAnimeInfo> AnimeResultsList;
	AActor* WorldRefObject;

	void ParseJsonString(const FString& JsonResponse, TArray<TSharedPtr<FJsonValue>>& OutData, TSharedPtr<FJsonObject>& OutPagination);
	void SendRequest(const FString& URL, const RequestType& Type);
	void NextPageRequest();
	void OnTagsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnResultsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
};