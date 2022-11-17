export module CPakParser.Paths.TopLevelAssetPath;

export import CPakParser.Core.FName;
import CPakParser.Serialization.FArchive;

export class FTopLevelAssetPath
{
public:

	FTopLevelAssetPath() = default;

	FTopLevelAssetPath(FName& Name)
	{
		auto NameStr = Name.ToString();
		std::string_view View = NameStr;

		if (View.empty() || View == "None")
			return;

		if (View[0] != '/' || View[View.size() - 1] == '\'')
		{
			auto Index = View.find('\'');
			if (Index != std::string_view::npos and View[View.size() - 1] == '\'')
				View = View.substr(Index + 1, View.size() - Index - 2);

			if (View.empty() || View[0] != '/')
				return;
		}

		auto DotIndex = View.find('.');

		if (DotIndex == std::string::npos)
			return;

		auto PackageNameView = View.substr(0, DotIndex);
		auto AssetNameView = View.substr(PackageNameView.size() + 1);

		if (AssetNameView.empty())
		{
			PackageName = FName(PackageNameView);
			AssetName = FName();
			return;
		}

		if (AssetNameView.find('.') != std::string_view::npos or
			AssetNameView.find(':') != std::string_view::npos)
		{
			return;
		}

		PackageName = FName(PackageNameView);
		AssetName = FName(AssetNameView);
	}

	__forceinline std::string GetPackageName() const { return PackageName.ToString(); }
	__forceinline std::string GetAssetName() const { return AssetName.ToString(); }

	friend class FArchive& operator<<(class FArchive& Ar, FTopLevelAssetPath& Value)
	{
		Ar << Value.PackageName;
		Ar << Value.AssetName;

		return Ar;
	}

	std::string ToString()
	{
		auto PackageNameStr = GetPackageName();
		auto AssetNameStr = GetAssetName();

		if (!AssetNameStr.empty())
			return PackageNameStr + '.' + AssetNameStr;

		return PackageNameStr;
	}

private:

	FName PackageName;
	FName AssetName;
};