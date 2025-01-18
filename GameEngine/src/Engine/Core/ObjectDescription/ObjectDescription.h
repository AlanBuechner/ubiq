#pragma once
#include <string>
#include <unordered_map>
#include "Utils/Vector.h"

namespace Engine
{
	class ObjectDescription;

	template<typename T>
	struct Convert;

	// --------------------------------- Object Description --------------------------------- //
	class ObjectDescription
	{
	public:
		enum class Type
		{
			String,
			Array,
			Object,
		};

	public:
		ObjectDescription() = default;
		ObjectDescription(Type type);
		ObjectDescription(const ObjectDescription& other) = default;
		ObjectDescription& operator=(const ObjectDescription& other) = default;
		ObjectDescription(ObjectDescription&& other) = default;

		bool IsNumber(bool* isFloat = nullptr) const;
		Type GetType() const { return m_Type; }

		template<typename T>
		T Get() const;

		template<typename T>
		bool TryGet(T& data) const;

		std::string& GetAsString() { return m_String; };
		Utils::Vector<ObjectDescription>& GetAsObjectArray() { return m_Vector; }
		std::unordered_map<std::string, ObjectDescription>& GetAsDescriptionMap() { return m_Enteries; }

		const std::string& GetAsString() const { return m_String; }
		const Utils::Vector<ObjectDescription>& GetAsObjectArray() const { return m_Vector; }
		const std::unordered_map<std::string, ObjectDescription>& GetAsDescriptionMap() const { return m_Enteries; }

		template<typename T>
		static ObjectDescription CreateFrom(const T& data);

		ObjectDescription& operator[](uint32 i) { return m_Vector[i]; }
		ObjectDescription& operator[](const std::string& i) { return m_Enteries[i]; }

		const ObjectDescription& operator[](uint32 i) const { return m_Vector[i]; }
		const ObjectDescription& operator[](const std::string& i) const { return m_Enteries.at(i); }

	private:
		Type m_Type;

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

#define NumConverter(type, func, f)\
	template<>\
	struct Convert<type>{\
	public:\
		static ObjectDescription Encode(type val){\
			ObjectDescription description(ObjectDescription::Type::String);\
			description.GetAsString() = std::to_string(val);\
			return description;\
		}\
		static bool Decode(type& out, const ObjectDescription& in){\
			if (in.GetType() != ObjectDescription::Type::String)\
				return false;\
			bool isFloat;\
			if (!in.IsNumber(&isFloat) || (f ? !isFloat : isFloat))\
				return false;\
			out = std::func(in.GetAsString());\
			return true;\
		}\
	};

#define IntConverter(type) NumConverter(type, stoi, false)

	// int
	IntConverter(uint8);
	IntConverter(uint16);
	IntConverter(uint32);
	IntConverter(uint64);

	IntConverter(int8);
	IntConverter(int16);
	IntConverter(int32);
	IntConverter(int64);

	// float
	NumConverter(float, stof, true)
	NumConverter(double, stod, true)

#undef IntConverter
#undef NumConverter

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
