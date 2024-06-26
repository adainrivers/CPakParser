module;

#include "Core/Defines.h"

export module CPakParser.Reflection.NumericProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;

#define QUICK_NUM_CAST(type) \
	{ \
	type TempVal = Val; \
	memcpy(OutBuffer, &TempVal, sizeof(type)); \
	break; \
	} \

export template <typename NumType, EPropertyType NumPropType>
class TNumericProperty : public FProperty
{
public:

	class Value : public IPropValue
	{
	public:

		NumType Val = 0;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			if (Type == NumPropType)
			{
				return true;
			}

			switch (Type)
			{
			case EPropertyType::ByteProperty:
			case EPropertyType::Int8Property:
			case EPropertyType::UInt16Property:
			case EPropertyType::Int16Property:
			case EPropertyType::UInt32Property:
			case EPropertyType::IntProperty:
			case EPropertyType::Int64Property:
			case EPropertyType::UInt64Property:
			case EPropertyType::FloatProperty:
			case EPropertyType::DoubleProperty:
				return true;
			default:
				return false;
			}
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			if (Type == NumPropType)
				memcpy(OutBuffer, &Val, sizeof(NumType));

			switch (Type)
			{
			case EPropertyType::ByteProperty: QUICK_NUM_CAST(uint8_t);
			case EPropertyType::Int8Property: QUICK_NUM_CAST(int8_t);
			case EPropertyType::UInt16Property: QUICK_NUM_CAST(uint16_t);
			case EPropertyType::Int16Property: QUICK_NUM_CAST(int16_t);
			case EPropertyType::UInt32Property: QUICK_NUM_CAST(uint32_t);
			case EPropertyType::IntProperty: QUICK_NUM_CAST(int32_t);
			case EPropertyType::Int64Property: QUICK_NUM_CAST(int64_t);
			case EPropertyType::UInt64Property: QUICK_NUM_CAST(uint64_t);
			case EPropertyType::FloatProperty: QUICK_NUM_CAST(float);
			case EPropertyType::DoubleProperty: QUICK_NUM_CAST(double);
			}
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();
		Ar << Ret->Val;

		return std::move(Ret);
	}
};

export typedef TNumericProperty<float, EPropertyType::FloatProperty> FFloatProperty;
export typedef TNumericProperty<double, EPropertyType::DoubleProperty> FDoubleProperty;
export typedef TNumericProperty<int8_t, EPropertyType::Int8Property> FInt8Property;
export typedef TNumericProperty<int16_t, EPropertyType::Int16Property> FInt16Property;
export typedef TNumericProperty<int32_t, EPropertyType::IntProperty> FIntProperty;
export typedef TNumericProperty<int64_t, EPropertyType::Int64Property> FInt64Property;
export typedef TNumericProperty<uint16_t, EPropertyType::UInt16Property> FUInt16Property;
export typedef TNumericProperty<uint32_t, EPropertyType::UInt32Property> FUInt32Property;
export typedef TNumericProperty<uint64_t, EPropertyType::UInt64Property> FUInt64Property;

export class FByteProperty : public TNumericProperty<uint8_t, EPropertyType::ByteProperty>
{
public:

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		return std::move(TNumericProperty::Serialize(Ar)); // TODO: enum crap
	}
};