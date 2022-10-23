// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimeApiRequestBlueprint.h"
#include "Http.h"
#include "Json.h"

FString UAnimeApiRequestBlueprint::Tags_URL = TEXT("https://api.jikan.moe/v4/genres/anime");
TArray<FString> UAnimeApiRequestBlueprint::Tags;

TArray<FString> UAnimeApiRequestBlueprint::GetTags()
{
	//If the tags array is empty, request for tags.
	if (Tags.Num() <= 0) {
		UE_LOG(LogTemp, Log, TEXT("Requesting URL: %s"), &TEXT("https://api.jikan.moe/v4/genres/anime"));

		FHttpModule* http = &FHttpModule::Get();
		TSharedRef<IHttpRequest> Request = http->CreateRequest();
		Request->SetURL(TEXT("https://api.jikan.moe/v4/genres/anime"));
		Request->OnProcessRequestComplete().BindStatic(&UAnimeApiRequestBlueprint::OnResponseReceived);
		Request->SetHeader("Content-Type", "application/json");
		Request->SetVerb("GET");
		Request->ProcessRequest();
	}

	return Tags;
}

void UAnimeApiRequestBlueprint::ClearTags()
{
	Tags.Empty();
}

TArray<TSharedPtr<FJsonValue>> UAnimeApiRequestBlueprint::ParseJsonString(FString JsonResponse)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonResponse);
	FJsonSerializer::Deserialize(Reader, JsonObject);

	auto data = JsonObject->GetArrayField("data");

	return data;
}

void UAnimeApiRequestBlueprint::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TArray<TSharedPtr<FJsonValue>> Results = ParseJsonString(Response->GetContentAsString());

	UE_LOG(LogTemp, Log, TEXT("Wtf? Results: %i"), Results.Num());

	for (int i = 0; i < Results.Num(); i++) {
		FString TagName = Results[i]->AsObject()->GetStringField("name");
		Tags.Add(TagName);
	}
}


