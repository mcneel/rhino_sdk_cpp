//
// Copyright (c) 1993-2026 Robert McNeel & Associates. All rights reserved.
// Rhinoceros is a registered trademark of Robert McNeel & Associates.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
// ALL IMPLIED WARRANTIES OF FITNESS FOR ANY PARTICULAR PURPOSE AND OF
// MERCHANTABILITY ARE HEREBY DISCLAIMED.
//				
// For complete Rhino SDK copyright information see <http://www.rhino3d.com/developer>.
//
////////////////////////////////////////////////////////////////

#pragma once

class RHINO_SDK_CLASS CRhinoMarkup : public ON_Markup
{
public:
  static unsigned int NextRuntimeSerialNumber();

  /*
  Returns:
    The Rhino document that owns this markup.
  */
  CRhinoDoc& Document() const;

  /*
  Returns:
    When a component is in a model for reference, this value identifies the
    reference model.
  Remarks:
    Reference components are not saved in .3dm archives.
    Typically this value is set and locked by the code that adds
    a component to a model.
    This value is copied by the copy constructor and operator=.
    This value is not saved in .3dm archives.

    In Rhino, this value is a worksession model runtime serial number and
    these values are used.
           0: not from a worksession reference model
           1: from an unidentified reference model
      2-1000: reserved for future use
       >1000: worksession reference model serial number
  */
  unsigned int WorksessionReferenceModelSerialNumber() const;

  /*
  Returns:
    When a component is in a model as part of the information required
    for a linked instance definition, this value identifies the the linked
    instance definition reference model.
  Remarks:
    Reference components are not saved in .3dm archives.
    Typically this value is set and locked by the code that adds
    a component to a model.
    This value is copied by the copy constructor and operator=.
    This value is not saved in .3dm archives.

    In Rhino, this value is a linked instance definition runtime serial number
    and  these values are used..
            0: Active model component.
       1-1000: reserved for future use
        >1000: linked instance definition serial number
  */
  unsigned int LinkedInstanceDefinitionSerialNumber() const;

  /*
  Description:
    Used for markup index remapping during I/O operations.
    It is a runtime variable and should be ignored by
    everything except CRhinoDoc::Read3DM() and CRhinoDoc::Write3DM().
  */
  int m_sort_index = -1;

  /*
  Description:
    Runtime index used when remapping markups during I/O operations.
  */
  int m_remap_index = -1;

  /*
  Description:
    Runtime serial number of this markup.
    This serial number is unique for each markup in an instance of a Rhino application.
    This serial number is not saved in files and it is generally different the next time a file is read.
  */
  const unsigned int m_runtime_serial_number;

private:
  friend class CRhinoMarkupTable;
  const class CRhinoMarkupTable& m_doc_markup_table;

  // Constructors and destructors are private because only CRhinoMarkupTable
  // should be creating CRhinoMarkup objects.
  CRhinoMarkup(CRhinoMarkupTable&, const ON_Markup&);
  ~CRhinoMarkup();
  CRhinoMarkup() = delete;
  CRhinoMarkup(const CRhinoMarkup&) = delete;
  CRhinoMarkup& operator=(const CRhinoMarkup&) = delete;

  // Delete all CRhinoMarkup.m_undo information
  void ClearUndoRecords();

  class CRhMarkupUndo* m_undo = nullptr;
  ON__UINT_PTR m_sdk_reserved0 = 0;
  ON__UINT_PTR m_sdk_reserved1 = 0;
  ON__UINT_PTR m_sdk_reserved2 = 0;
  ON__UINT_PTR m_sdk_reserved3 = 0;
};


//////////////////////////////////////////////////////////////////////////
//
// CRhinoMarkupTable
//

class RHINO_SDK_CLASS CRhinoMarkupTable : private ON_SimpleArray<CRhinoMarkup*>
{
public:
  CRhinoDoc& Document() const;
  const class CRhinoDocTableReference& TableReference() const;

  /*
  Returns:
    Number of markups in the table.
  Remarks:
    This count spans slots emptied by a purge - see the PURGED TABLE SLOTS
    note in rhinoSdkDoc.h - so it can be larger than the number of markups
    that are actually there, and operator[] returns nullptr for the difference.
  */
  int MarkupCount() const;

  /*
  Description:
    Conceptually, the markup table is an array of MarkupCount() markups.  The
    operator[] can be used to get individual markups. A markup is either active
    or deleted and this state is reported by CRhinoMarkup::IsDeleted().
  Parameters:
    markup_index [in] - zero based array index
  Returns:
    If successful, a pointer to the markup is returned.
    If not successful or if the index is out-of-range, nullptr is returned.
    Note, this pointer may become invalid after AddMarkup() is called.
  Remarks:
    nullptr is also returned for an in-range index whose slot was emptied by a
    purge, which happens whenever a worksession reference model carrying a
    markup is detached. Such slots are normal, they are spanned by
    MarkupCount(), and they accumulate for the life of the document, so any
    code that walks this table by index must expect nullptr and skip it. See
    the PURGED TABLE SLOTS note in rhinoSdkDoc.h.
  */
  const CRhinoMarkup* operator[](int markup_index) const;

  /*
  Description:
    Gets a markup by component manifest.
  Parameters:
    markup_manifest_item [in] - the component manifest item.
  Returns:
    If successful, a pointer to the markup is returned.
    If not successful, nullptr is returned.
  */
  const CRhinoMarkup* Markup(
    const ON_ComponentManifestItem& markup_manifest_item
  ) const;

  /*
  Description:
    Gets a markup by component manifest.
  Parameters:
    markup_manifest_item [in] - the component manifest item.
    not_found [in] - the value to return if not found.
Returns:
    If successful, a pointer to the markup is returned.
    If not successful, not_found is returned is specified. Otherwise, nullptr is returned.
  */
  const CRhinoMarkup* Markup(
    const ON_ComponentManifestItem& markup_manifest_item,
    const CRhinoMarkup* not_found
  ) const;

  /*
  Description:
    Gets a component manifest item by markup name.
  Parameters:
    markup_name [in] - the markup name.
  Returns:
    If successful, a ON_ModelComponent::Type::Markup component manifest item.
    If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  */
  const ON_ComponentManifestItem& MarkupManifestItemFromName(
    const wchar_t* markup_name
  ) const;

  /*
  Description:
     Gets a markup by name.
  Parameters:
    markup_name [in] - the markup name.
  Returns:
    If successful, a pointer to the markup is returned.
    If not successful, nullptr is returned.
  */
  const CRhinoMarkup* MarkupFromName(
    const wchar_t* markup_name
  ) const;

  /*
  Description:
    Gets a component manifest item by markup id.
  Parameters:
    markup_id [in] - the markup id.
    bSearchDeletedSectionStyles [in] - set true to search for deleted markups.
  Returns:
    If successful, a ON_ModelComponent::Type::Markup component manifest item.
    If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  */
  const ON_ComponentManifestItem& MarkupManifestItemFromId(
    ON_UUID markup_id,
    bool bSearchDeletedMarkups
  ) const;

  /*
  Description:
    Gets a markup by markup id.
  Parameters:
    markup_id [in] - the markup id.
    bSearchDeletedMarkups [in] - set true to search for deleted markups.
  Returns:
    If successful, a pointer to the markup is returned.
    If not successful, nullptr is returned.
  */
  const CRhinoMarkup* MarkupFromId(
    ON_UUID markup_id,
    bool bSearchDeletedMarkups
  ) const;

  /*
  Description:
    Gets a component manifest item by markup index.
  Parameters:
    markup_index [in] - the markup index.
    bSearchDeletedMarkups [in] - set true to search for deleted markups.
  Returns:
    If successful, a ON_ModelComponent::Type::Markup component manifest item.
    If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  */
  const ON_ComponentManifestItem& MarkupManifestItemFromIndex(
    int markup_index,
    bool bSearchDeletedMarkups
  ) const;

  /*
  Description:
    Gets a markup by index.
  Parameters:
    markup_index [in] - the markup index.
    bSearchDeletedMarkups [in] - set true to search for deleted markups.
  Returns:
    If successful, a pointer to the markup is returned.
    If not successful, nullptr is returned.
  */
  const CRhinoMarkup* MarkupFromIndex(
    int markup_index,
    bool bSearchDeletedMarkups
  ) const;

  /*
  Description:
    Gets an unused markup name.
  Parameters:
    unused_markup_name [out] - an unused markup name.
  */
  void GetUnusedMarkupName(
    ON_wString& unused_markup_name
  ) const;

  /*
  Description:
    Gets an unused markup name.
  Parameters:
    root_name [in] - the root name for the new markup name.
    unused_markup_name [out] - an unused markup name.
  */
  void GetUnusedMarkupName(
    const wchar_t* root_name,
    ON_wString& unused_markup_name
  ) const;

  /*
  Description:
    Basic tool used to create a new markup with specified definition to the
    markup table. The AddMarkup() functions all use CreateMarkup() when a new
    markup needs to be added to the table.
  Parameters:
    markup - [in]
      The name and id are used as candidates. If they are in use, a new name
      or id is created.
    worksession_ref_model_sn - [in]
          0: not from a reference model
          1: not saved in files but not part of a worksession reference file
    2-1000: reserved for future use
      >1000: worksession reference file serial number
    linked_idef_sn - [in]
          0: not from a liked instance definition
    1-1000: reserved for future use
      >1000: group instance definition serial number
  Returns:
    index of new markup on success.
    ON_UNSET_INT_INDEX on failure
  */
  const ON_ComponentManifestItem& CreateMarkup(
    const ON_Markup& markup,
    unsigned int worksession_ref_model_sn,
    unsigned int linked_idef_sn
  );

  /*
  Description:
    Adds a new unset markup to the table.
  Returns:
    index for the markup on success
  */
  const ON_ComponentManifestItem& AddMarkup();

  /*
  Description:
    Adds a new markup to the table.
  Parameters:
    markup - [in] The markup to add.
    bReference - [in] true if this markup is a reference markup.
  //                  Reference markups are not saved in files.
 Returns:
    index for the markup on success.
    ON_UNSET_INT_INDEX if input is not valid.
  */
  const ON_ComponentManifestItem& AddMarkup(const ON_Markup& markup, bool bReference);

  /*
  Description:
    Change markup settings
  Parameters:
    markup [in] - New settings. This information is copied.
    markup_index [in] - Zero based index of markup to modify.
    bQuiet [in] - Set true to disable any warning or error messages.
  Returns:
    true if successful. false otherwise.
  */
  bool ModifyMarkup(const ON_Markup& markup, int markup_index, bool bQuiet);
  bool ModifyMarkup(const ON_Markup& markup, const ON_ComponentManifestItem& item, bool bQuiet);

  /*
  Description:
    If the markup has been modified and the modification can be undone,
    then UndoModifyMarkup() will restore the markup to its previous state.
  Returns:
    True if the markup had been modified and the modifications were undone.
  */
  bool UndoModifyMarkup(int markup_index, unsigned int undo_record_sn);

  /*
  Description:
    Delete a markup.
  Parameters:
    markup_index [in] - Zero based index of markup to delete.
                        Must be in the range 0 <= index < MarkupCount().
    bQuiet [in] - Set true to disable any warning or error messages.
  Returns:
     true if successful, or false if index is out of range or the markup cannot be deleted.
  */
  bool DeleteMarkup(int markup_index, bool bQuiet);
  bool DeleteMarkup(const ON_ComponentManifestItem& item, bool bQuiet);
  
  /*
  Description:
    Delete multiple markups.
  Parameters:
    markup_count    - [in] length of markup_indices[] array.
    markup_indices  - [in] array zero based indices of markup to delete.
                      The indices must be in the range 0 <= index < MarkupCount().
    bQuiet          - [in] If true, no warning message box appears if a markup
                      cannot be deleted because it is in use.
  Returns:
    Number of markups that were deleted. If this is < markup_count,
    you can dig through the table and check which ones were not deleted.
  */
  int DeleteMarkups(
    int markup_count,
    const int* markup_indices,
    bool bQuiet
  );

  /*
  Description:
    Delete multiple markups.
  Parameters:
    markup_count    - [in] length of markup_indices[] array.
    markup_indices  - [in] array zero based indices of markup to delete.
                      The indices must be in the range 0 <= index < MarkupCount().
    bQuiet          - [in] If true, no warning message box appears if a markup
                      cannot be deleted because it is in use.
    delete_warning  - [in] Action to take when multiple delete warnings occur, where:
                      0 == No to all, 1 == Yes to all, 2 == Ask the user.
  Returns:
    Number of markups that were deleted. If this is < markup_count,
    you can dig through the table and check which ones were not deleted.
  */
  int DeleteMarkups(
    int markup_count,
    const int* markup_indices,
    bool bQuiet,
    int delete_warning
  );

  /*
  Description:
    Undeletes a markup that has been deleted by DeleteMarkup().
  Parameters:
    markup_index - [in] Zero based index of markup to delete.
                   Must be in the range 0 <= index < MarkupCount().
   Returns:
     true if successful, false otherwise.
  */
  bool UndeleteMarkup(int markup_index);

  /*
  Returns:
    true if the markup is an element in this table.
  */
  bool InMarkupTable(const ON_Markup* markup) const;

  /*
  Returns:
    true if the markup index is a valid index of a markup element in this table.
  */
  bool IsValidMarkupTableIndex(int index) const;
  
  /*
  Description:
    Use this to set CRhinoMarkup::m_sort_index so that
    the values of CRhinoMarkup::m_sort_index run from 0 to MarkupCount() - 1
    and compare(CRhinoMarkupTable[i],CRhinoMarkupTable[j]) < 0) implies
    CRhinoMarkupTable[i].m_sort_index < CRhinoMarkupTable[j].m_sort_index.
  Parameters:
    compare [in] - Compare function that returns <0 if arg1<arg2, 0 if arg1=arg2, and >0 if arg1>arg2.
    cull [in] - optional filter function with prototype that returns true if the markup should be ignored when sorting.
                Markups that are ignored are not included in the list returned by GetSortedList().
                Pass nullptr if you do not need to cull section style.
    ptr [in] - pointer passed as last argument to compare() and cull().
  Remarks:
    After calling Sort(), you can repeatedly call GetSortedList() to get a sorted list of CRhinoMarkup pointers.
    Sort() and GetSortedList() do not modify the order or persistent information in the table.
    They are intended to be used to get sorted lists for dialogs, etc.
  */
  void Sort(int (*compar)(const CRhinoMarkup*, const CRhinoMarkup*, void*),
    int (*cull)(const CRhinoMarkup*, void*),
    void* ptr
  );

  /*
  Description:
    Gets an array of pointers to markups that is sorted by
    the values of CRhinoMarkup::Name().
  Parameters:
    sorted_list [out] - this array is returned with length MarkupCount()
                        and is sorted by the values of CRhinoMarkup::Name().
    bIgnoreDeleted [in] - set true to ignore deleted section styles.
  Remarks:
    Use Sort() to set the values of CRhinoMarkup::m_sort_index.
  */
  void GetSortedList(
    ON_SimpleArray<const CRhinoMarkup*>& sorted_list,
    bool bIgnoreDeleted = false
  ) const;

  /*
  Description:
    Gets an array of pointers to markups that is sorted by
    the values of CRhinoMarkup::Name().
  Parameters:
    sorted_list [out] - this array is returned with length MarkupCount()
                        and is sorted by the values of CRhinoMarkup::Name().
    bIgnoreDeleted [in] - set true to ignore deleted markups.
    bIgnoreReference [in] - set true to ignore reference markups.
  Remarks:
    Use Sort() to set the values of CRhinoMarkup::m_sort_index.
  */
  void GetSortedList(
    ON_SimpleArray<const CRhinoMarkup*>& sorted_list,
    bool bIgnoreDeleted,
    bool bIgnoreReference
  ) const;

  void SetRemapIndex(int markup_index, int remap_index);

  // Built-in markups.
  // Localized names, id set, unique, and persistent
  const CRhinoMarkup& Default; // index = -1, id set, unique, and persistent

private:
  friend class CRhDocSdkExtension;
  friend class CRhinoDoc;

  bool Internal_DeleteMarkupHelper(
    int markup_index,
    bool bQuiet,
    int* delete_referenced_markups,
    int* reference_file_warning_enabled
  );

  unsigned int Internal_PurgeWorkSessionReferenceModel(unsigned int worksession_ref_model_sn);
  bool Internal_AddUndoModifyMarkupRecord(int markup_index);
  void Internal_ClearUndoRecords();
  void Internal_Destroy();

private:
  CRhinoMarkupTable(CRhinoDoc&);
  ~CRhinoMarkupTable();
  // NO implementation to prohibit use
  CRhinoMarkupTable() = delete;
  CRhinoMarkupTable(const CRhinoMarkupTable&) = delete;
  CRhinoMarkupTable& operator=(const CRhinoMarkupTable&) = delete;

private:
  CRhinoDoc& m_doc;
  CRhinoDocTableReference m_table_reference;
  ON_ComponentManifest& Internal_Manifest();

private:
  // This must be private.
  CRhinoMarkup m_Default;
  ON__UINT_PTR m_sdk_reserved0 = 0;
  ON__UINT_PTR m_sdk_reserved1 = 0;
  ON__UINT_PTR m_sdk_reserved2 = 0;
  ON__UINT_PTR m_sdk_reserved3 = 0;
};

