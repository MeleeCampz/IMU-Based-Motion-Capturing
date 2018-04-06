#include "FPS_IKEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

IMPLEMENT_GAME_MODULE(FFPS_IKEditorModule, FPS_IKEditor);

DEFINE_LOG_CATEGORY(FPS_IKEditor)

#define LOCTEXT_NAMESPACE "FPS_IKEditor"

void FFPS_IKEditorModule::StartupModule()
{
	UE_LOG(FPS_IKEditor, Warning, TEXT("FPS_IKEditor: Log Started"));
}

void FFPS_IKEditorModule::ShutdownModule()
{
	UE_LOG(FPS_IKEditor, Warning, TEXT("FPS_IKEditor: Log Ended"));
}

#undef LOCTEXT_NAMESPACE