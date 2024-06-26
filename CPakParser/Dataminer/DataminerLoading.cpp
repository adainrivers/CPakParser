#include "Dataminer.h"

#include "Core/Globals/GlobalContext.h"
#include "Core/Reflection/Mappings.h"

import CPakParser.Serialization.FArchive;
import CPakParser.Files.SerializableFile;
import CPakParser.Logging;

bool Dataminer::LoadTypeMappings(std::string UsmapFilePath)
{
	return Mappings::RegisterTypesFromUsmap(UsmapFilePath, Context->ObjectArray);
}

std::future<bool> Dataminer::LoadTypeMappingsAsync(std::string UsmapFilePath)
{
	return std::async(std::launch::async, &Dataminer::LoadTypeMappings, this, UsmapFilePath);
}

void Dataminer::SerializeFileInternal(FGameFilePath& FilePath, TSharedPtr<ISerializableFile> OutFile)
{
	auto Entry = Context->FilesManager.FindFile(FilePath);
	auto Reader = Entry.CreateReader();

	if (!Reader)
		return;

	OutFile->Serialize(*Reader);
}

UPackagePtr Dataminer::LoadPackage(FGameFilePath Path)
{
	FExportState State;
	State.LoadTargetOnly = false;

	return LoadPackage(Path, State);
}

UPackagePtr Dataminer::LoadPackage(FFileEntryInfo& Entry)
{
	FExportState State;
	State.LoadTargetOnly = false;

	return LoadPackage(Entry, State);
}

UPackagePtr Dataminer::LoadPackage(FGameFilePath Path, FExportState& State)
{
	auto AssetPath = Path.WithExtension(".uasset");
	auto Entry = Context->FilesManager.FindFile(AssetPath);

	if (!Entry.IsValid())
		return nullptr;

	return LoadPackage(Entry, State);
}

UPackagePtr Dataminer::LoadPackage(FFileEntryInfo& Entry, FExportState& State)
{
	auto Reader = Entry.CreateReader();

	if (!Reader)
	{
		LogError("Could not create reader");
		return nullptr;
	}

	return Entry.GetAssociatedFile()->CreatePackage(*Reader, Context, State);
}