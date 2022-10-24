// Fill out your copyright notice in the Description page of Project Settings.


#include "ApiEventManager.h"

UApiEventManager* UApiEventManager::Instance;

UApiEventManager::~UApiEventManager()
{
	if (Instance == this)
		Instance = NULL;
	OnTagsUpdated.Clear();
}

UApiEventManager* UApiEventManager::GetInstance()
{
	if (Instance == NULL)
		Instance = NewObject<UApiEventManager>();
	return Instance;
}
