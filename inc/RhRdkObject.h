
#pragma once // Public SDK header

#include "RhRdkDefinitions.h"

/** \class CRhRdkObject

	Base class for objects which are created by RDK but deleted by the client (or vice-versa).
	Overrides of new and delete allow you to delete such objects from your client DLL without any complications.

*/
class RHRDK_SDK CRhRdkObject
{
public:
	CRhRdkObject();
	virtual ~CRhRdkObject();

	//[SDK_UNFREEZE]
	//CRhRdkObject(const CRhRdkObject&);
	//const CRhRdkObject& operator=(const CRhRdkObject&);

	void* operator new(size_t size);
	void* operator new[] (size_t size);
	void  operator delete (void* buf);
	void  operator delete[] (void* buf);

	static void PopulateDebuggingInfo(void);
	static void CheckMemory(void);

	/** Debugging aid for hunting leaks of RDK objects. Records which RDK objects exist
		at the moment of the call. Only does anything in a debug build with
		CHECK_MEMORY_LEAKS commented in; see RhRdkObject.cpp. */
	static void SetLeakCheckpoint(void);

	/** Reports RDK objects that were created after the last SetLeakCheckpoint() call and
		are still alive, grouped by the call stack that created them. Call
		SetLeakCheckpoint() before an operation and this afterwards to see what that
		operation leaked. Both are callable from the debugger. */
	static void ReportLeaksSinceCheckpoint(void);

	/** Emergency virtual function for future expansion. */
	virtual void* EVF(const wchar_t* wszFunc, void* pvData);

private:
	class CImpl;
	CImpl* m_pImpl;
};
