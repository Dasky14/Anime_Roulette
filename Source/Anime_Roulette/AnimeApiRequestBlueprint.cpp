// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimeApiRequestBlueprint.h"
#include "Http.h"
#include "Json.h"
#include <Anime_Roulette/ApiEventManager.h>

FString UAnimeApiRequestBlueprint::TagsURL = TEXT("https://api.jikan.moe/v4/genres/anime");
FString UAnimeApiRequestBlueprint::SearchURL = TEXT("https://api.jikan.moe/v4/anime");
FString UAnimeApiRequestBlueprint::LastSearchURL = TEXT("");
int UAnimeApiRequestBlueprint::PageCount = 1;
int UAnimeApiRequestBlueprint::CurrentPage = 1;
TArray<FTagInfo> UAnimeApiRequestBlueprint::Tags;
TArray<FAnimeInfo> UAnimeApiRequestBlueprint::AnimeResults;

TArray<FTagInfo> UAnimeApiRequestBlueprint::GetTags()
{
	//If the tags array is empty, request for tags.
	if (Tags.Num() <= 0)
		SendRequest(TagsURL, &UAnimeApiRequestBlueprint::OnTagsResponseReceived);

	return Tags;
}

TArray<FAnimeInfo> UAnimeApiRequestBlueprint::GetResults()
{
	return AnimeResults;
}

/// <summary>
/// Sends the HTTP query for anime results.
/// </summary>
/// <param name="SearchParams">Search options struct.</param>
/// <returns>Whether query was sent successfully.</returns>
bool UAnimeApiRequestBlueprint::AskResults(const FSearchParams& SearchParams)
{
	FString FinalURL = SearchURL;

	bool first = true;

	if (SearchParams.IncludedTags.Num() > 0) {
		FinalURL.Append(first ? "?" : "&");
		first = false;

		FinalURL.Append("genres=");
		for (int i = 0; i < SearchParams.IncludedTags.Num(); i++) {
			FTagInfo tag = SearchParams.IncludedTags[i];

			FinalURL.Append(FString::FromInt(tag.Id));
			UE_LOG(LogTemp, Log, TEXT("Included Tag: %s, %i"), *tag.Name, tag.Id);

			if (i < SearchParams.IncludedTags.Num() - 1)
				FinalURL.AppendChar(',');
		}
	}

	if (SearchParams.ExcludedTags.Num() > 0) {
		FinalURL.Append(first ? "?" : "&");
		first = false;

		FinalURL.Append("genres_exclude=");
		for (int i = 0; i < SearchParams.ExcludedTags.Num(); i++) {
			FTagInfo tag = SearchParams.ExcludedTags[i];

			FinalURL.Append(FString::FromInt(tag.Id));
			UE_LOG(LogTemp, Log, TEXT("Excluded Tag: %s, %i"), *tag.Name, tag.Id);

			if (i < SearchParams.ExcludedTags.Num() - 1)
				FinalURL.AppendChar(',');
		}
	}

	LastSearchURL = FinalURL;

	SendRequest(FinalURL, &UAnimeApiRequestBlueprint::OnResultsResponseReceived);

	return true;
}

/// <summary>
/// Clears the tags array. Does not broadcast OnTagsUpdated.
/// </summary>
void UAnimeApiRequestBlueprint::ClearTags()
{
	Tags.Empty();
}

/// <summary>
/// Clears the results array. Does not broadcast OnResultsUpdated.
/// </summary>
void UAnimeApiRequestBlueprint::ClearResults()
{
	AnimeResults.Empty();
}

void UAnimeApiRequestBlueprint::ParseJsonString(const FString& JsonResponse, TArray<TSharedPtr<FJsonValue>>& OutData, TArray<TSharedPtr<FJsonValue>>& OutPagination)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonResponse);
	FJsonSerializer::Deserialize(Reader, JsonObject);

	if (JsonObject->HasField("data"))
		OutData = JsonObject->GetArrayField("data");
	if (JsonObject->HasField("pagination"))
		OutPagination = JsonObject->GetArrayField("pagination");
}

/// <summary>
/// Sends an HTTP GET request with a given URL, and binds the given function to the OnProcessRequestComplete event.
/// </summary>
/// <param name="URL">URL for the GET request</param>
/// <param name="function">A reference to a function to call after completion</param>
void UAnimeApiRequestBlueprint::SendRequest(const FString& URL, void(*function)(FHttpRequestPtr, FHttpResponsePtr, bool))
{
	UE_LOG(LogTemp, Log, TEXT("Requesting URL: %s"), *URL);

	FHttpModule* http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = http->CreateRequest();
	Request->SetURL(URL);
	Request->OnProcessRequestComplete().BindStatic(function);
	Request->SetHeader("Content-Type", "application/json");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void UAnimeApiRequestBlueprint::OnTagsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TArray<TSharedPtr<FJsonValue>> Data;
	TArray<TSharedPtr<FJsonValue>> Pagination;
	ParseJsonString(Response->GetContentAsString(), Data, Pagination);

	for (int i = 0; i < Data.Num(); i++) {
		FString TagName = Data[i]->AsObject()->GetStringField("name");
		int TagId = Data[i]->AsObject()->GetIntegerField("mal_id");
		Tags.Add(FTagInfo(TagId, TagName));
	}

	UApiEventManager* EventManager = UApiEventManager::GetInstance();
	EventManager->OnTagsUpdated.Broadcast();
}

void UAnimeApiRequestBlueprint::OnResultsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TArray<TSharedPtr<FJsonValue>> Data;
	TArray<TSharedPtr<FJsonValue>> Pagination;
	ParseJsonString(Response->GetContentAsString(), Data, Pagination);

	for (int i = 0; i < Data.Num(); i++) {
		FAnimeInfo newAnime;
		newAnime.Id = Data[i]->AsObject()->GetIntegerField("mal_id");
		newAnime.Name = Data[i]->AsObject()->GetStringField("title");
		newAnime.ImageUrl = Data[i]->AsObject()->GetObjectField("images")->GetObjectField("jpg")->GetStringField("image_url");

		AnimeResults.Add(newAnime);
		UE_LOG(LogTemp, Log, TEXT("Added %i: (%i) %s"), i, newAnime.Id, *newAnime.Name);
	}

	// TODO: Make this thing recursive as long as there are pages left to get, but only broadcast the update on first page loaded.
	// Maybe add some global event to broadcast when the entire search is complete?




	UApiEventManager* EventManager = UApiEventManager::GetInstance();
	EventManager->OnResultsUpdated.Broadcast();
}
