#include "Core/Defines.h"
#include <bitset>
#include <vector>

import CPakParser.Serialization.Unversioned;
import CPakParser.Reflection.PropertyIterator;
import CPakParser.Serialization.FArchive;

#if EXTENSIVE_LOGGING
import CPakParser.Logging;
#endif

struct FUnversionedHeader
{
	struct FFragment
	{
		static constexpr uint32_t SkipMax = 127;
		static constexpr uint32_t ValueMax = 127;

		uint8_t SkipNum = 0;
		bool bHasAnyZeroes = false;
		uint8_t ValueNum = 0;
		bool bIsLast = 0;

		static constexpr uint32_t SkipNumMask = 0x007fu;
		static constexpr uint32_t HasZeroMask = 0x0080u;
		static constexpr uint32_t ValueNumShift = 9u;
		static constexpr uint32_t IsLastMask = 0x0100u;

		FFragment() = default;

		__forceinline constexpr FFragment(uint16_t Int)
		{
			SkipNum = static_cast<uint8_t>(Int & SkipNumMask);
			bHasAnyZeroes = (Int & HasZeroMask) != 0;
			ValueNum = static_cast<uint8_t>(Int >> ValueNumShift);
			bIsLast = (Int & IsLastMask) != 0;
		}
	};

	friend FArchive& operator<<(FArchive& Ar, FUnversionedHeader& Header)
	{
		uint32_t ZeroMaskNum = 0;
		uint32_t UnmaskedNum = 0;

		while (true)
		{
			uint16_t Packed;
			Ar << Packed;

			FFragment Fragment(Packed);

			Header.Fragments.push_back(Fragment);

			/*if (Fragment.bHasAnyZeroes)
				ZeroMaskNum += Fragment.ValueNum;
			else UnmaskedNum += Fragment.ValueNum;*/

			(Fragment.bHasAnyZeroes ? ZeroMaskNum : UnmaskedNum) += Fragment.ValueNum;

			if (Fragment.bIsLast)
				break;
		}

		if (ZeroMaskNum)
		{
			std::vector<bool>& ZeroMask = Header.ZeroMask;

			ZeroMask.resize(ZeroMaskNum, 0);

			auto Data = (uint32_t*)ZeroMask[0]._Getptr(); // TODO: change this once phmap is fixed so we can use std::latest

			if (ZeroMaskNum <= 8)
			{
				uint8_t Int;
				Ar << Int;
				*Data = Int;
			}
			else if (ZeroMaskNum <= 16)
			{
				uint16_t Int;
				Ar << Int;
				*Data = Int;
			}
			else
			{
				for (uint32_t Idx = 0, Num = (ZeroMaskNum + 32 - 1) / 32; Idx < Num; ++Idx)
				{
					Ar << Data[Idx];
				}
			}

			Header.bHasNonZeroValues = UnmaskedNum > 0 || std::find(ZeroMask.begin(), ZeroMask.end(), false) != ZeroMask.end();
		}
		else
		{
			Header.bHasNonZeroValues = UnmaskedNum > 0;
		}

		return Ar;
	}

	std::vector<FFragment> Fragments; // TODO: what do we need?
	bool bHasNonZeroValues = false;
	std::vector<bool> ZeroMask;

	__forceinline bool HasValues() const
	{
		return bHasNonZeroValues | ZeroMask.size();
	}

	__forceinline bool HasNonZeroValues() const
	{
		return bHasNonZeroValues;
	}
};

class FUnversionedIterator // TODO: refactor this
{
public:

	__forceinline FUnversionedIterator(const FUnversionedHeader& Header, UStructPtr& Struct)
		: It(Struct)
		, ZeroMask(Header.ZeroMask)
		, FragmentIt(Header.Fragments.data())
		, bDone(!Header.HasValues())
	{
		if (!bDone)
		{
			Skip();
		}
	}

	void Next()
	{
		++It;
		--RemainingFragmentValues;
		ZeroMaskIndex += FragmentIt->bHasAnyZeroes;

		if (RemainingFragmentValues == 0)
		{
			if (FragmentIt->bIsLast)
			{
				bDone = true;
			}
			else
			{
				++FragmentIt;
				Skip();
			}
		}
	}

	explicit operator bool() const
	{
		return !bDone;
	}

	bool IsNonZero() const
	{
		return !FragmentIt->bHasAnyZeroes || !ZeroMask[ZeroMaskIndex];
	}

	__forceinline FProperty* operator*()
	{
		return *It;
	}

private:

	FPropertyIterator It;
	const std::vector<bool>& ZeroMask;
	const FUnversionedHeader::FFragment* FragmentIt = nullptr;
	bool bDone = false;
	uint32_t ZeroMaskIndex = 0;
	uint32_t RemainingFragmentValues = 0;

	void Skip()
	{
		It += FragmentIt->SkipNum;

		while (FragmentIt->ValueNum == 0)
		{
			++FragmentIt;
			It += FragmentIt->SkipNum;
		}

		RemainingFragmentValues = FragmentIt->ValueNum;
	}
};

void FUnversionedSerializer::SerializeUnversionedProperties(UStructPtr Struct, FArchive& Ar, UObjectPtr Object)
{
	FUnversionedHeader Header;
	Ar << Header;

	if (!Header.HasNonZeroValues() or !Header.HasValues())
	{
		return;
	}

	for (FUnversionedIterator It(Header, Struct); It; It.Next())
	{
		if (!It.IsNonZero())
			continue;

		auto Prop = *It;

#if EXTENSIVE_LOGGING
		Log("Serializing property %s %d", Prop->Name.c_str(), (int)Prop->Type);
#endif

		auto Value = Prop->Serialize(Ar);

		if (!Value)
			continue;
		
		Object->PropertyValues.push_back({ Prop->Name, std::move(Value) });
	}
}