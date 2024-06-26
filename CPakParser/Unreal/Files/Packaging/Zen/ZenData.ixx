export module CPakParser.Zen.Data;

import <vector>;
import CPakParser.Package.ObjectIndex;
import CPakParser.Core.UObject;
import CPakParser.Package.Id;
import CPakParser.Names.NameMap;
import CPakParser.Core.FName;
import CPakParser.Versions.PackageFileVersion;
import CPakParser.Versioning.CustomVersion;
import CPakParser.Package.Index;

export enum class EExportFilterFlags : uint8_t
{
	None,
	NotForClient,
	NotForServer
};

export struct FExportBundleEntry
{
	enum EExportCommandType
	{
		ExportCommandType_Create,
		ExportCommandType_Serialize,
		ExportCommandType_Count
	};

	uint32_t LocalExportIndex;
	uint32_t CommandType;
};

export struct FDependencyBundleHeader
{
	int32_t FirstEntryIndex;
	uint32_t EntryCount[FExportBundleEntry::ExportCommandType_Count][FExportBundleEntry::ExportCommandType_Count];
};

export struct FExportBundleHeader
{
	uint64_t SerialOffset;
	uint32_t FirstEntryIndex;
	uint32_t EntryCount;
};

export struct FExportMapEntry
{
	uint64_t CookedSerialOffset = 0;
	uint64_t CookedSerialSize = 0;
	FMappedName ObjectName;
	FPackageObjectIndex OuterIndex;
	FPackageObjectIndex ClassIndex;
	FPackageObjectIndex SuperIndex;
	FPackageObjectIndex TemplateIndex;
	uint64_t PublicExportHash;
	UObject::Flags ObjectFlags = UObject::Flags::RF_NoFlags;
	EExportFilterFlags FilterFlags = EExportFilterFlags::None;
	uint8_t Pad[3] = {};
};

export struct FBulkDataMapEntry
{
	int64_t SerialOffset;
	int64_t DuplicateSerialOffset;
	int64_t SerialSize;
	uint32_t Flags;
	uint32_t Pad;
};

export enum class EZenPackageVersion : uint32_t
{
	Initial,

	LatestPlusOne,
	Latest = LatestPlusOne - 1
};

export struct FZenPackageVersioningInfo
{
	EZenPackageVersion ZenVersion;
	FPackageFileVersion PackageVersion;
	int32_t LicenseeVersion;
	FCustomVersionContainer CustomVersions;

	friend FArchive& operator<<(FArchive& Ar, FZenPackageVersioningInfo& ExportBundleEntry);
};

export struct FZenPackageSummary
{
	uint32_t bHasVersioningInfo;
	uint32_t HeaderSize;
	FMappedName Name;
	uint32_t PackageFlags;
	uint32_t CookedHeaderSize;
	int32_t ImportedPublicExportHashesOffset;
	int32_t ImportMapOffset;
	int32_t ExportMapOffset;
	int32_t ExportBundleEntriesOffset;
	int32_t DependencyBundleHeadersOffset;
	int32_t DependencyBundleEntriesOffset;
	int32_t ImportedPackageNamesOffset;
};

export struct FZenPackageHeaderData
{
	uint32_t ExportCount = 0;
	uint8_t* AllExportDataPtr;
	FZenPackageVersioningInfo VersioningInfo;
	FNameMap NameMap;
	std::string PackageName;
	FZenPackageSummary PackageSummary;
	std::vector<uint64_t> ImportedPublicExportHashes;
	std::vector<FPackageObjectIndex> ImportMap;
	std::vector<FExportMapEntry> ExportMap;
	std::vector<FPackageId> ImportedPackageIds;
	std::vector<FName> ImportedPackageNames;
	std::vector<FExportBundleEntry> ExportBundleEntries;
	std::vector<FBulkDataMapEntry> BulkDataMap;
	std::vector<FDependencyBundleHeader> DependencyBundleHeaders;
	std::vector<FPackageIndex> DependencyBundleEntries;


	__forceinline bool HasFlags(uint32_t Flags)
	{
		return PackageSummary.PackageFlags & Flags;
	}
};