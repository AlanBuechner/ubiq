#pragma once
#include "Utils/Types.h"

namespace GUI
{
	class PanelBase;
}

namespace GUI
{
	/*
	* PanelRef is a weak ref to panels
	* the panel is owned by the context and will be cleaned up by the context
	* this object exists only as a way to check if a panel is valid
	*/
	class PanelRef
	{
	private:
		struct PanelControleBlock
		{
			PanelBase* panel = nullptr;
			uint32 refCount = 0;
		};

	public:
		PanelRef(PanelBase* panel);
		PanelRef(const PanelRef& other);

		~PanelRef();

		PanelBase* Get() { return Valid() ? m_ControleBlock->panel : nullptr; }
		template<class T>
		T* GetAs() { return Valid() ? (T*)m_ControleBlock->panel : nullptr; }

		PanelBase* operator->() { return Valid() ? m_ControleBlock->panel : nullptr; }
		operator PanelBase* () { return Get(); }

		bool Valid() { return m_ControleBlock != nullptr && m_ControleBlock->panel != nullptr; }

	private:
		PanelControleBlock* m_ControleBlock = nullptr;
	};
}
