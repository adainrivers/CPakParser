export module CPakParser.Versioning.CustomVersion;

import <vector>;
import CPakParser.Serialization.FArchive;
import CPakParser.Misc.FGuid;

export typedef std::vector<struct FCustomVersion> FCustomVersionArray;

export enum class ECustomVersionSerializationFormat
{
	Unknown,
	Guids,
	Enums,
	Optimized,

	CustomVersion_Automatic_Plus_One,
	Latest = CustomVersion_Automatic_Plus_One - 1
};

export struct FCustomVersion
{
	FCustomVersion() = default;

	friend class FCustomVersionContainer;

	FGuid Key;
	int32_t Version;
	int32_t ReferenceCount;

	bool operator==(FGuid InKey) const
	{
		return Key == InKey;
	}

	bool operator!=(FGuid InKey) const
	{
		return Key != InKey;
	}

	friend FArchive& operator<<(FArchive& Ar, FCustomVersion& Version)
	{
		Ar << Version.Key;
		Ar << Version.Version;

		return Ar;
	}
};

export class FCustomVersionContainer
{
public:

	void Serialize(FArchive& Ar, ECustomVersionSerializationFormat Format = ECustomVersionSerializationFormat::Latest)
	{
		switch (Format) // implement the deprecated formats if needed but i doubt it ever will
		{
		case ECustomVersionSerializationFormat::Optimized:
		{
			Ar << Versions;
		}
		break;
		}
	}

	FCustomVersionArray& GetVersions()
	{
		return Versions;
	}

	const FCustomVersion* TryGetVersion(const FGuid& Guid) const
	{
		for (int i = 0; i < Versions.size(); i++)
		{
			auto& V = Versions[i];

			if (V == Guid)
				return &V;
		}

		return nullptr;
	}

private:

	FCustomVersionArray Versions;
};