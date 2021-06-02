// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASAttachEditor.h"
#include "GASAttachEditorStyle.h"
#include "GASAttachEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "SGASAttachEditor.h"

static const FName GASAttachEditorTabName("GASAttachEditor");

#define LOCTEXT_NAMESPACE "FGASAttachEditorModule"

void FGASAttachEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FGASAttachEditorStyle::Initialize();
	FGASAttachEditorStyle::ReloadTextures();

	FGASAttachEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FGASAttachEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FGASAttachEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGASAttachEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GASAttachEditorTabName, FOnSpawnTab::CreateRaw(this, &FGASAttachEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FGASAttachEditorTabTitle", "查看角色携带GA"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FGASAttachEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName(TEXT("GameplayCueApp")));

		if (GameplayCheckEditorTab.IsValid())
		{
			GameplayCheckEditorTab.Pin()->RequestCloseTab();
		}
	}

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FGASAttachEditorStyle::Shutdown();

	FGASAttachEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GASAttachEditorTabName);
}

TSharedRef<SDockTab> FGASAttachEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{

	return SAssignNew(GameplayCheckEditorTab, SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			CreateGASCheckTool().ToSharedRef()
		];
}

TSharedPtr<SWidget> FGASAttachEditorModule::CreateGASCheckTool()
{
	TSharedPtr<SWidget> ReturnWidget;
	if (IsInGameThread())
	{
		TSharedPtr<SGASAttachEditor> SharedPtr = SNew(SGASAttachEditor);
		ReturnWidget = SharedPtr;
	}
	return ReturnWidget;
}

static void GASAttachEditorShow(UWorld* InWorld)
{
	FGlobalTabmanager::Get()->TryInvokeTab(GASAttachEditorTabName);
}

FAutoConsoleCommandWithWorld AbilitySystemDebugNextCategoryCmd(
	TEXT("GASAttachEditorShow"),
	TEXT("打开查看角色GA的编辑器"),
	FConsoleCommandWithWorldDelegate::CreateStatic(GASAttachEditorShow)
);

void FGASAttachEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(GASAttachEditorTabName);
}

void FGASAttachEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowGlobalTabSpawners");
			Section.AddMenuEntryWithCommandList(FGASAttachEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	/*{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FGASAttachEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}*/
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGASAttachEditorModule, GASAttachEditor)