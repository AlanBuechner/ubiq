#pragma once
#include <string>
#include <unordered_map>
#include "Utils/Vector.h"

namespace Engine
{
	class ObjectDescription;

	class ConverterBase
	{
	public:
		virtual void EncodeObj() {}
		virtual void DecodeObj() {}

	public:
		static void GetConverterMap() {};

		static std::unordered_map<uint64, ConverterBase*>& GetObjectConverterFunctions();
		class AddObjectConverter
		{
		public:
			AddObjectConverter(uint64 typeID, ConverterBase* converter) {
				GetObjectConverterFunctions().emplace(typeID, converter);
			}
		};
#define ADD_OBJECT_CONVERTER(type, converter) static Engine::ConverterBase::AddObjectConverter CAT(converter,__LINE__)(typeid(type).hash_code(), new converter());
		
	};

	template<typename T>
	struct Convert : public ConverterBase
	{};

	// --------------------------------- Object Description --------------------------------- //
	class ObjectDescription
	{
	public:
		enum class Type
		{
			String,
			Array,
			Object,
			Uint,
			Sint,
			Float,
			Bool,
		};

	public:
		ObjectDescription() = default;
		ObjectDescription(Type type);
		ObjectDescription(const ObjectDescription& other) = default;
		ObjectDescription& operator=(const ObjectDescription& other) = default;
		ObjectDescription(ObjectDescription&& other) = default;

		bool IsString() const { return m_Type == Type::String; }
		bool IsArray() const { return m_Type == Type::Array; }
		bool IsObject() const { return m_Type == Type::Object; }
		bool IsUint() const { return m_Type == Type::Uint; }
		bool IsSint() const { return m_Type == Type::Sint; }
		bool IsInt() const { return IsUint() || IsSint(); }
		bool IsFloat() const { return m_Type == Type::Float; }
		bool IsNumber() const { return IsInt() || IsFloat(); }
		bool IsBool() const { return m_Type == Type::Bool; }
		Type GetType() const { return m_Type; }

		template<typename T>
		T Get() const;

		template<typename T>
		bool TryGet(T& data) const;

		std::string& GetAsString() { return m_String; };
		Utils::Vector<ObjectDescription>& GetAsObjectArray() { return m_Vector; }
		std::unordered_map<std::string, ObjectDescription>& GetAsDescriptionMap() { return m_Enteries; }
		uint64& GetAsUint() { return m_Uint; }
		int64& GetAsInt() { return m_Sint; }
		double& GetAsFloat() { return m_Float; }
		bool& GetAsBool() { return m_Bool; }

		const std::string& GetAsString() const { return m_String; }
		const Utils::Vector<ObjectDescription>& GetAsObjectArray() const { return m_Vector; }
		const std::unordered_map<std::string, ObjectDescription>& GetAsDescriptionMap() const { return m_Enteries; }
		const uint64 GetAsUint() const { return m_Uint; }
		const int64 GetAsInt() const { return m_Sint; }
		const double GetAsFloat() const { return m_Float; }
		const bool GetAsBool() const { return m_Bool; }

		template<typename T>
		static ObjectDescription CreateFrom(const T& data);

		ObjectDescription& operator[](uint32 i) { return m_Vector[i]; }
		ObjectDescription& operator[](const std::string& i) { return m_Enteries[i]; }

		const ObjectDescription& operator[](uint32 i) const { return m_Vector[i]; }
		const ObjectDescription& operator[](const std::string& i) const { return m_Enteries.at(i); }

		bool HasEntery(const std::string& key);

	private:
		Type m_Type;
		union
		{
			uint64 m_Uint;
			int64 m_Sint;
			double m_Float;
			bool m_Bool;
		};
		std::string m_String;
		Utils::Vector<ObjectDescription> m_Vector;
		std::unordered_map<std::string, ObjectDescription> m_Enteries;
	};


	// --------------------------------- Function Definitions --------------------------------- //

	template<typename T>
	T ObjectDescription::Get() const
	{
		T out;
		Convert<T> converter;
		CORE_ASSERT(converter.Decode(out, *this), "Failed to decode object description to data type");
		return out;
	}

	template<typename T>
	bool ObjectDescription::TryGet(T& data) const
	{
		Convert<T> converter;
		return converter.Decode(data);
	}

	template<typename T>
	ObjectDescription ObjectDescription::CreateFrom(const T& data)
	{
		Convert<T> converter;
		return converter.Encode(data);
	}

	// --------------------------------- Converters --------------------------------- //
	template<>
	struct Convert<std::string>
	{
	public:
		static ObjectDescription Encode(const std::string& str)
		{
			ObjectDescription description(ObjectDescription::Type::String);
			description.GetAsString() = str;
			return description;
		}

		static bool Decode(std::string& out, const ObjectDescription& in)
		{
			if (in.GetType() != ObjectDescription::Type::String)
				return false;

			out = in.GetAsString();
			return true;
		}
	};


#define NumConverter(ctype, type, func)\
	template<>\
	struct Convert<ctype>{\
	public:\
		static ObjectDescription Encode(ctype val){\
			ObjectDescription description(ObjectDescription::Type::type);\
			description.func() = val;\
			return description;\
		}\
		static bool Decode(ctype& out, const ObjectDescription& in){\
			if(!in.IsNumber()) return false;\
			else if(in.IsFloat()) out = in.GetAsFloat();\
			else if(in.IsUint()) out = in.GetAsUint();\
			else if(in.IsSint()) out = in.GetAsInt();\
			return true;\
		}\
	};

#define UIntConverter(type) NumConverter(type, Uint, GetAsUint)
#define IntConverter(type) NumConverter(type, Sint, GetAsInt)
#define FloatConverter(type) NumConverter(type, Float, GetAsFloat)

	UIntConverter(uint8);
	UIntConverter(uint16);
	UIntConverter(uint32);
	UIntConverter(uint64);

	IntConverter(int8);
	IntConverter(int16);
	IntConverter(int32);
	IntConverter(int64);

	FloatConverter(float);
	FloatConverter(double);

#undef FloatConverter
#undef IntConverter
#undef UIntConverter

	template<>
		struct Convert<bool> {
		public:
			static ObjectDescription Encode(bool val) {
				ObjectDescription description(ObjectDescription::Type::Bool);
				description.GetAsBool() = val; 
				return description; 
		}
		static bool Decode(bool& out, const ObjectDescription& in) {
			if (!in.IsBool() || !in.IsNumber()) return false;
			else if (in.IsBool()) out = in.GetAsBool();
			else if (in.IsFloat()) out = in.GetAsFloat();
			else if (in.IsUint()) out = in.GetAsUint();
			else if (in.IsSint()) out = in.GetAsInt();
			return true;
		}
	};

	template<typename T>
	class Convert<Utils::Vector<T>>
	{
	public:
		static ObjectDescription Encode(const Utils::Vector<T>& vec)
		{
			ObjectDescription description(ObjectDescription::Type::Array);
			Utils::Vector<ObjectDescription>& vector = description.GetAsObjectArray();
			vector.Resize(vec.Count());
			for (uint32 i = 0; i < vec.Count(); i++)
				vector[i] = ObjectDescription::CreateFrom<T>(vec[i]);
			return description;
		}

		static bool Decode(Utils::Vector<T>& out, const ObjectDescription& in)
		{
			if (in.GetType() != ObjectDescription::Type::Array)
				return false;

			const Utils::Vector<ObjectDescription>& descriptions = in.GetAsObjectArray();
			out.Resize(descriptions.Count());
			for (uint32 i = 0; i < descriptions.Count(); i++)
				out[i] = descriptions[i].Get<T>();
			return true;
		}
	};

}
