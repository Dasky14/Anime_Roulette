// Fill out your copyright notice in the Description page of Project Settings.


#include "ApiEventManager.h"

UApiEventManager* UApiEventManager::Instance;

UApiEventManager::~UApiEventManager()
{
	UE_LOG(LogTemp, Log, TEXT("ApiEventManager Destroyed!"));

	if (Instance == this)
		Instance = NULL;
}

UApiEventManager* UApiEventManager::GetInstance()
{
	if (Instance == NULL)
		Instance = NewObject<UApiEventManager>();
	return Instance;
}
