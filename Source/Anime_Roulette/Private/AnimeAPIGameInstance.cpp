// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimeAPIGameInstance.h"
#include "Http.h"
#include "Json.h"
#include "TimerManager.h"
#include "Misc/DateTime.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include <Anime_Roulette/ApiEventManager.h>
#include <random>

UAnimeAPIGameInstance::UAnimeAPIGameInstance() {
	TagsURL = TEXT("https://api.jikan.moe/v4/genres/anime");
	SearchURL = TEXT("https://api.jikan.moe/v4/anime");
	LastSearchURL = TEXT("");
	PageCount = 1;
	CurrentPage = 1;
	LastRateLimitReset = 0;
}

TArray<FTagInfo> UAnimeAPIGameInstance::GetTags()
{
	//If the tags array is empty, request for tags.
	if (TagsList.Num() <= 0)
		SendRequest(TagsURL, RequestType::Tags);

	return TagsList;
}

TArray<FAnimeInfo> UAnimeAPIGameInstance::GetResults()
{
	return AnimeResultsList;
}

FAnimeInfo UAnimeAPIGameInstance::GetRandomResult()
{
	int seed = FDateTime::Now().GetMillisecond();

	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distribution(0, AnimeResultsList.Num() - 1);

	int randomNum = distribution(generator);
	FAnimeInfo result = AnimeResultsList[randomNum];
	UE_LOG(LogTemp, Log, TEXT("Random result %i: %s"), randomNum, *result.Name);

	return result;
}

TArray<FAnimeInfo> UAnimeAPIGameInstance::GetSixRandomResults()
{
	TArray<FAnimeInfo> Randomised = AnimeResultsList;

	int32 LastIndex = Randomised.Num() - 1;
	for (int i = 0; i <= LastIndex; i++) {
		int32 SwapIndex = FMath::RandRange(i, LastIndex);
		if (i != SwapIndex) {
			Randomised.Swap(i, SwapIndex);
		}
	}

	TArray<FAnimeInfo> Results = { };

	int32 AddIndex = 0;
	for (int i = 0; i < 6; i++) {
		if (AddIndex == Randomised.Num()) {
			AddIndex = 0;
		}
		Results.Add(Randomised[AddIndex]);
		AddIndex++;
		UE_LOG(LogTemp, Log, TEXT("Random result %i: %s"), i, *Results[i].Name);
	}

	return Results;
}

int UAnimeAPIGameInstance::GetResultCount()
{
	return AnimeResultsList.Num();
}

bool UAnimeAPIGameInstance::AskResults(const FSearchParams& SearchParams)
{
	FString FinalURL = SearchURL;

	FinalURL.Append("?page=1");
	FinalURL.Append("&min_score=" + FString::SanitizeFloat(SearchParams.MinScore));
	FinalURL.Append("&max_score=" + FString::SanitizeFloat(SearchParams.MaxScore));

	if (SearchParams.IncludedTags.Num() > 0) {
		FinalURL.Append("&genres=");
		for (int i = 0; i < SearchParams.IncludedTags.Num(); i++) {
			FTagInfo tag = SearchParams.IncludedTags[i];

			FinalURL.Append(FString::FromInt(tag.Id));
			UE_LOG(LogTemp, Log, TEXT("Included Tag: %s, %i"), *tag.Name, tag.Id);

			if (i < SearchParams.IncludedTags.Num() - 1)
				FinalURL.AppendChar(',');
		}
	}

	if (SearchParams.ExcludedTags.Num() > 0) {
		FinalURL.Append("&genres_exclude=");
		for (int i = 0; i < SearchParams.ExcludedTags.Num(); i++) {
			FTagInfo tag = SearchParams.ExcludedTags[i];

			FinalURL.Append(FString::FromInt(tag.Id));
			UE_LOG(LogTemp, Log, TEXT("Excluded Tag: %s, %i"), *tag.Name, tag.Id);

			if (i < SearchParams.ExcludedTags.Num() - 1)
				FinalURL.AppendChar(',');
		}
	}

	if (SearchParams.Status != "-") {
		FinalURL.Append("&status=");
		FinalURL.Append(SearchParams.Status);
	}

	if (SearchParams.Rating != "-") {
		FinalURL.Append("&rating=");
		FinalURL.Append(SearchParams.Rating);
	}

	if (SearchParams.Sort != "-" && SearchParams.OrderBy != "-") {
		FinalURL.Append("&sort=");
		FinalURL.Append(SearchParams.Sort);

		FinalURL.Append("&order_by=");
		FinalURL.Append(SearchParams.OrderBy);
	}

	LastSearchURL = FinalURL;

	LastRateLimitReset = FPlatformTime::Seconds();
	SendRequest(FinalURL, RequestType::Results);

	return true;
}

void UAnimeAPIGameInstance::ClearTags()
{
	TagsList.Empty();
}

void UAnimeAPIGameInstance::ClearResults()
{
	AnimeResultsList.Empty();
}

void UAnimeAPIGameInstance::SetSearchWidgetRef(UUserWidget* target)
{
	SearchWidget = target;
}

UUserWidget* UAnimeAPIGameInstance::GetSearchWidgetRef()
{
	return SearchWidget;
}

void UAnimeAPIGameInstance::SetDieRef(AActor* target)
{
	Die = target;
}

AActor* UAnimeAPIGameInstance::GetDieRef()
{
	return Die;
}

void UAnimeAPIGameInstance::ParseJsonString(const FString& JsonResponse, TArray<TSharedPtr<FJsonValue>>& OutData, TSharedPtr<FJsonObject>& OutPagination)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonResponse);
	FJsonSerializer::Deserialize(Reader, JsonObject);

	if (JsonObject->HasField("data"))
		OutData = JsonObject->GetArrayField("data");
	if (JsonObject->HasField("pagination"))
		OutPagination = JsonObject->GetObjectField("pagination");
}

void UAnimeAPIGameInstance::SendRequest(const FString& URL, const RequestType& Type)
{
	UE_LOG(LogTemp, Log, TEXT("Requesting URL: %s"), *URL);

	FHttpModule* http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = http->CreateRequest();
	Request->SetURL(URL);

	switch (Type)
	{
		case Tags:
			Request->OnProcessRequestComplete().BindUObject(this, &UAnimeAPIGameInstance::OnTagsResponseReceived);
			break;
		case Results:
			Request->OnProcessRequestComplete().BindUObject(this, &UAnimeAPIGameInstance::OnResultsResponseReceived);
			break;
	}

	Request->SetHeader("Content-Type", "application/json");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void UAnimeAPIGameInstance::NextPageRequest()
{
	FString LastPage = TEXT("?page=" + FString::FromInt(CurrentPage));
	FString NextPage = TEXT("?page=" + FString::FromInt(CurrentPage + 1));
	FString NextURL = LastSearchURL.Replace(*LastPage, *NextPage);

	LastSearchURL = NextURL;

	SendRequest(NextURL, RequestType::Results);
}

void UAnimeAPIGameInstance::OnTagsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TArray<TSharedPtr<FJsonValue>> Data;
	TSharedPtr<FJsonObject> Pagination;
	ParseJsonString(Response->GetContentAsString(), Data, Pagination);

	for (int i = 0; i < Data.Num(); i++) {
		FString TagName = Data[i]->AsObject()->GetStringField("name");
		int TagId = Data[i]->AsObject()->GetIntegerField("mal_id");
		TagsList.Add(FTagInfo(TagId, TagName));
	}

	UApiEventManager* EventManager = UApiEventManager::GetInstance();
	EventManager->OnTagsUpdated.Broadcast();
}

void UAnimeAPIGameInstance::OnResultsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	UE_LOG(LogTemp, Log, TEXT("Results received!"));

	UApiEventManager* EventManager = UApiEventManager::GetInstance();
	UWorld* TheWorld = GetWorld();

	// Check Response for 429 rate limit status
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, JsonObject);

	// Retry in 5 seconds if rate limited
	if (JsonObject->HasField("status") && JsonObject->GetStringField("status") == "429") {
		UE_LOG(LogTemp, Log, TEXT("You are being rate limited!"));

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &UAnimeAPIGameInstance::NextPageRequest);
		TheWorld->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 5.0f, false);

		EventManager->OnResultsUpdated.Broadcast();
		return;
	}

	// If data is valid, continue to add to results.
	TArray<TSharedPtr<FJsonValue>> Data;
	TSharedPtr<FJsonObject> Pagination;
	ParseJsonString(Response->GetContentAsString(), Data, Pagination);

	// Return if there are zero results
	int Items = Pagination->GetObjectField("items")->GetIntegerField("total");
	if (Items == 0) {
		EventManager->OnProgressUpdated.Broadcast(0, 0);
		EventManager->OnResultsUpdated.Broadcast();
		return;
	}

	// Get pagination information
	CurrentPage = Pagination->GetIntegerField("current_page");
	PageCount = Pagination->GetIntegerField("last_visible_page");

	UE_LOG(LogTemp, Log, TEXT("Pages: %i/%i"), CurrentPage, PageCount);

	// Go through page anime information
	for (int i = 0; i < Data.Num(); i++) {
		FAnimeInfo newAnime;
		newAnime.Id = Data[i]->AsObject()->GetIntegerField("mal_id");
		newAnime.Name = Data[i]->AsObject()->GetStringField("title");
		newAnime.ImageUrl = Data[i]->AsObject()->GetObjectField("images")->GetObjectField("jpg")->GetStringField("image_url");
		newAnime.Type = Data[i]->AsObject()->GetStringField("type");
		newAnime.Score = Data[i]->AsObject()->GetNumberField("score");
		newAnime.Rating = Data[i]->AsObject()->GetStringField("rating");
		newAnime.Status = Data[i]->AsObject()->GetStringField("status");
		newAnime.EpisodeCount = Data[i]->AsObject()->GetIntegerField("episodes");
		newAnime.Duration = Data[i]->AsObject()->GetStringField("duration");
		newAnime.Synopsis = Data[i]->AsObject()->GetStringField("synopsis");

		AnimeResultsList.Add(newAnime);
	}

	if (CurrentPage < PageCount) {
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &UAnimeAPIGameInstance::NextPageRequest);
		TheWorld->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1.0f, false);

		EventManager->OnProgressUpdated.Broadcast(CurrentPage, PageCount);
	}

	if (CurrentPage >= PageCount) {
		EventManager->OnProgressUpdated.Broadcast(CurrentPage, PageCount);
		EventManager->OnResultsUpdated.Broadcast();
	}
}
