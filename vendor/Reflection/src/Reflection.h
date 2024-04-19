#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

#include "Hash.h"

namespace Reflect {

	template <class T> 
	T* CreateInstance(void* location = nullptr) {
		if (location == nullptr)
			return new T();
		return new(location) T;
	}

	template <class T>
	void DestroyInstance(void* location, bool freeMemory) {
		if (location != nullptr)
		{
			if(freeMemory)
				delete (T*)location;
			else
				((T*)location)->~T();
		}

	}

	class Class {
	public:
		typedef std::function<void* (void*)> CreateFunc;
		typedef std::function<void (void*, bool)> DestroyFunc;

		Class() = default;
		Class(
			const std::string& name, 
			const std::string& sname, 
			uint64_t typeID, 
			uint64_t size, 
			const std::string& group,
			CreateFunc createFunc, 
			DestroyFunc destrouFunc,
			const std::vector<std::string>& flags
		)
			: name(name), sname(sname), typeID(typeID), size(size), group(group), createFunc(createFunc), destroyFunc(destroyFunc)
		{
			for (const std::string& flag : flags)
				this->flags.emplace(flag);
		}

		const std::string& GetName() const { return name; }
		const std::string& GetSname() const { return sname; }
		const uint64_t GetTypeID() const { return typeID; }
		const uint64_t GetSize() const { return size; }
		const std::string& GetGroup() const { return group; }

		void* CreateInstance(void* location = nullptr) const { return createFunc(location); }
		void DestroyInstance(void* location, bool freeMemory = true) const { destroyFunc(location, freeMemory); }

		bool HasFlag(const std::string& flag) const { return flags.find(flag) != flags.end(); }

	private:
		std::string name;
		std::string sname;
		uint64_t typeID;
		uint64_t size;
		std::string group;
		CreateFunc createFunc;
		DestroyFunc destroyFunc;
		std::unordered_set<std::string> flags;
	};

	class Registry {
		static Registry* s_Instance;

	public:
		static Registry* GetRegistry();

		const Class& GetClass(const std::string& name) { return m_Classes[name]; }
		const std::vector<const Class*>& GetGroup(const std::string& groupName) { return m_ClassGroups[groupName]; }

		void AddClass(const Class& c);

		template <class T> class Add {
		public:
			Add(const std::string& name, const std::string& sname, const std::string& group, const std::vector<std::string>& flags) {
				Registry::GetRegistry()->AddClass(Class(name, sname, crc32(sname.c_str(), sname.size()), sizeof(T), group, CreateInstance<T>, DestroyInstance<T>, flags));
			}
		};

	private:
		std::unordered_map<std::string, Class> m_Classes;
		std::unordered_map<std::string, std::vector<const Class*>> m_ClassGroups;
	};

	class Reflected
	{
	public:
		virtual const Reflect::Class& GetClass() = 0;
		virtual std::string GetTypeName() = 0;
		virtual uint64_t GetTypeID() = 0;
	};

} // namespace Reflect

// requires the full semantic name of the class i.e. (SomeNamespace::SomeClass)
#define REFLECTED_BODY(sname)	\
								static const Reflect::Class& GetStaticClass() { const auto& c = Reflect::Registry::GetRegistry()->GetClass(#sname); return c; }\
								static std::string GetStaticTypeName() { return #sname; }\
								static uint64_t GetStaticTypeID() { const uint64 tid = GetStaticClass().GetTypeID(); return tid; }\
								virtual const Reflect::Class& GetClass() { return GetStaticClass(); }\
								virtual std::string GetTypeName() override { return GetStaticTypeName(); }\
								virtual uint64_t GetTypeID() override { return GetStaticTypeID(); }

// STRIP_REFLECTION is used so the IDE does not yell at you because it does not know what __attribute__ is
#if defined(STRIP_REFLECTION)
	#define CLASS(...) class
	#define PROPERTY(...)
#else
	#define CLASS(...) class __attribute__((annotate("reflect-class," #__VA_ARGS__)))
	#define PROPERTY(...) __attribute__((annotate("reflect-property;" #__VA_ARGS__)))
#endif

#define LINK_REFLECTION_DATA(name) void DeadLink##name(); void* _DeadLink##name = &DeadLink##name;
