#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <typeinfo>
#include <iostream>

namespace Reflect {

	template <class T> T* CreateInstance() {
		return new T();
	}

	class Class {
	public:
		typedef std::function<void* ()> CreateFunc;

		Class() = default;
		Class(const std::string& name, const std::string& sname, uint64_t typeID, const std::string& group, CreateFunc createFunc)
			: name(name), sname(sname), typeID(typeID), group(group), createFunc(createFunc)
		{}

		const std::string& GetName() const { return name; }
		const std::string& GetSname() const { return sname; }
		const uint64_t GetTypeID() const { return typeID; }
		const std::string& GetGroup() const { return group; }

		void* CreateInstance() const { return createFunc(); }

	private:
		std::string name;
		std::string sname;
		uint64_t typeID;
		std::string group;
		CreateFunc createFunc;
	};

	class Registry {
		static Registry* s_Instance;

	public:

		const Class& GetClass(const std::string& name);

		void AddClass(const Class& c);
		static Registry* GetRegistry();

		template <class T> class Add {
		public:
			Add(const std::string& name, const std::string& sname, const std::string& group, Class::CreateFunc createFunc) {
				Registry::GetRegistry()->AddClass(Class(name, sname, typeid(T).hash_code(), group, createFunc));
			}
		};

	private:
		std::unordered_map<std::string, Class> m_Classes;
		std::unordered_map<std::string, std::vector<const Class*>> m_ClassGroups;
	};

} // namespace Reflect


#define CLASS(...) class __attribute__((annotate("reflect-class," #__VA_ARGS__)))
#define PROPERTY(...) __attribute__((annotate("reflect-property;" #__VA_ARGS__)))

#define LINK_REFLECTION_DATA(name) void DeadLink##name(); void* _DeadLink##name = &DeadLink##name;
