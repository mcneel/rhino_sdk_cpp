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

#if defined(INCLUDE_RHINO_SECTION_STYLE_TABLE)

//////////////////////////////////////////////////////////////////////////
//
// CRhinoSectionStyle contains a named section style definition and
// is stored in the CRhinoDoc section style table.
//

class RHINO_SDK_CLASS CRhinoSectionStyle : public ON_SectionStyle
{
public:
  static unsigned int NextRuntimeSerialNumber();

  /*
  Returns:
    The Rhino document that owns this section style.
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
    Used for section style index remapping during I/O operations.
    It is a runtime variable and should be ignored by
    everything except CRhinoDoc::Read3DM() and CRhinoDoc::Write3DM().
  */
  int m_sort_index = -1;

  /*
  Description:
    Runtime index used when remapping section styles during I/O operations.
  */
  int m_remap_index = -1;

  /*
  Description:
    Runtime serial number of this section style.
    This serial number is unique for each section style in an instance of a Rhino application.
    This serial number is not saved in files and it is generally different the next time a file is read.
  */
  const unsigned int m_runtime_serial_number;

private:
  friend class CRhinoSectionStyleTable;
  const class CRhinoSectionStyleTable& m_doc_section_style_table;

  // Constructors and destructors are private because only CRhinoSectionStyleTable
  // should be creating CRhinoSectionStyle objects.
  CRhinoSectionStyle(CRhinoSectionStyleTable&, const ON_SectionStyle&);
  ~CRhinoSectionStyle();
  // NO implementation to prohibit use
  CRhinoSectionStyle() = delete;
  CRhinoSectionStyle(const CRhinoSectionStyle&) = delete;
  CRhinoSectionStyle& operator=(const CRhinoSectionStyle&) = delete;

  // Delete all CRhinoSectionStyle.m_undo information
  void ClearUndoRecords();

  class CRhSectionStyleUndo* m_undo = nullptr;
  ON__UINT_PTR m_sdk_reserved0 = 0;
  ON__UINT_PTR m_sdk_reserved1 = 0;
  ON__UINT_PTR m_sdk_reserved2 = 0;
  ON__UINT_PTR m_sdk_reserved3 = 0;
};


//////////////////////////////////////////////////////////////////////////
//
// CRhinoSectionStyleTable
//

class RHINO_SDK_CLASS CRhinoSectionStyleTable : private ON_SimpleArray<CRhinoSectionStyle*>
{
public:
  CRhinoDoc& Document() const;
  const class CRhinoDocTableReference& TableReference() const;

  /*
  Returns:
    Number of section styles in the table.
  */
  int SectionStyleCount() const;

  /*
  Description:
    Conceptually, the section style table is an array of SectionStyleCount()
    section styles.  The operator[] can be used to get individual section
    styles.  A section style is either active or deleted and this state is
    reported by CRhinoSectionStyle::IsDeleted().
  Parameters:
    section_style_index [in] - zero based array index
  Returns:
    If successful, a pointer to the section style is returned.
    If not successful or if the index is out-of-range, nullptr is returned.
    Note, this pointer may become invalid after AddSectionStyle() is called.
  */
  const CRhinoSectionStyle* operator[](int section_style_index) const;

  /*
  Description:
    Gets a section style by component manifest.
  Parameters:
    section_style_manifest_item [in] - the component manifest item.
  Returns:
    If successful, a pointer to the section style is returned.
    If not successful, nullptr is returned.
  */
  const CRhinoSectionStyle* SectionStyle(
    const ON_ComponentManifestItem& section_style_manifest_item
  ) const;

  /*
  Description:
    Gets a section style by component manifest.
  Parameters:
    section_style_manifest_item [in] - the component manifest item.
  Returns:
    If successful, a pointer to the section style is returned.
    If not successful, not_found is returned is specified. Otherwise, nullptr is returned.
  */
  const CRhinoSectionStyle* SectionStyle(
    const ON_ComponentManifestItem& section_style_manifest_item,
    const CRhinoSectionStyle* not_found
  ) const;

  /*
  Description:
    Gets a component manifest item by section style name.
  Parameters:
    section_style_name [in] - the section style name.
  Returns:
    If successful, a ON_ModelComponent::Type::SectionStyle component manifest item.
    If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  */
  const ON_ComponentManifestItem& SectionStyleManifestItemFromName(
    const wchar_t* section_style_name
  ) const;

  /*
  Description:
     Gets a section style by section style name.
  Parameters:
    section_style_name [in] - the section style name.
  Returns:
    If successful, a pointer to the section style is returned.
    If not successful, nullptr is returned.
  */
  const CRhinoSectionStyle* SectionStyleFromName(
    const wchar_t* section_style_name
  ) const;

  /*
  Description:
    Gets a component manifest item by section style id.
  Parameters:
    section_style_id [in] - the section style id.
    bSearchDeletedSectionStyles [in] - set true to search for deleted section styles.
  Returns:
    If successful, a ON_ModelComponent::Type::SectionStyle component manifest item.
    If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  */
  const ON_ComponentManifestItem& SectionStyleManifestItemFromId(
    ON_UUID section_style_id,
    bool bSearchDeletedSectionStyles
  ) const;

  /*
  Description:
    Gets a section style by section style id.
  Parameters:
    section_style_id [in] - the section style id.
    bSearchDeletedSectionStyles [in] - set true to search for deleted section styles.
  Returns:
    If successful, a pointer to the section style is returned.
    If not successful, nullptr is returned.
  */
  const CRhinoSectionStyle* SectionStyleFromId(
    ON_UUID section_style_id,
    bool bSearchDeletedSectionStyles
  ) const;

  /*
  Description:
    Gets a component manifest item by section style index.
  Parameters:
    section_style_index [in] - the section style index.
    bSearchDeletedSectionStyles [in] - set true to search for deleted section styles.
  Returns:
    If successful, a ON_ModelComponent::Type::SectionStyle component manifest item.
    If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  */
  const ON_ComponentManifestItem& SectionStyleManifestItemFromIndex(
    int section_style_index,
    bool bSearchDeletedSectionStyles
  ) const;

  /*
  Description:
    Gets a section style by section style index.
  Parameters:
    section_style_index [in] - the section style index.
    bSearchDeletedSectionStyles [in] - set true to search for deleted section styles.
  Returns:
    If successful, a pointer to the section style is returned.
    If not successful, nullptr is returned.
  */
  const CRhinoSectionStyle* SectionStyleFromIndex(
    int section_style_index,
    bool bSearchDeletedSectionStyles
  ) const;

  /*
  Description:
    Gets an unused section style name.
  Parameters:
    unused_section_style_name [out] - an unused section style index.
  */
  void GetUnusedSectionStyleName(
    ON_wString& unused_section_style_name
  ) const;

  /*
  Description:
    Gets an unused section style name.
  Parameters:
    root_name [in] - the root name for the new section style name.
    unused_section_style_name [out] - an unused section style index.
  */
  void GetUnusedSectionStyleName(
    const wchar_t* root_name, 
    ON_wString& unused_section_style_name
  ) const;

  /*
  Description:
    Basic tool used to create a new section style with specified definition to the section style table.
    The AddSectionStyle() functions all use CreateSectionStyle() when a new section style needs to be
    added to the section style table.
  Parameters:
    section_style - [in]
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
    index of new section style on success.
    ON_UNSET_INT_INDEX on failuer
  */
  const ON_ComponentManifestItem& CreateSectionStyle(
    const ON_SectionStyle& section_style,
    unsigned int worksession_ref_model_sn,
    unsigned int linked_idef_sn
  );

  /*
  Description:
    Adds a new unset section style to the table.
  Returns:
    index for the section style on success
  */
  const ON_ComponentManifestItem& AddSectionStyle();

  /*
  Description:
    Adds a new section style to the table.
  Parameters:
    section_style - [in] The section style to add.
    bReference - [in] true if this section style is a reference section style.
  //                  Reference section styles are not saved in files.
 Returns:
    index for the section style on success.
    ON_UNSET_INT_INDEX if input is not valid.
  */
  const ON_ComponentManifestItem& AddSectionStyle(const ON_SectionStyle& section_style, bool bReference);

  /*
  Description:
    Change section style settings
  Parameters:
    section_style [in] - New settings. This information is copied.
    section_style_index [in] - Zero based index of section style to modify.
    bQuiet [in] - Set true to disable any warning or error messages.
  Returns:
    true if successful. false otherwise.
  */
  bool ModifySectionStyle(const ON_SectionStyle& section_style, int section_style_index, bool bQuiet);
  bool ModifySectionStyle(const ON_SectionStyle& section_style, const ON_ComponentManifestItem& item, bool bQuiet);

  /*
  Description:
    If the section style has been modified and the modification can be undone,
    then UndoModifySectionStyle() will restore the section style to its previous state.
  Returns:
    True if the section style had been modified and the modifications were undone.
  */
  bool UndoModifySectionStyle(int section_style_index, unsigned int undo_record_sn);

  /*
  Description:
    Delete a section style.
  Parameters:
    section_style_index [in] - Zero based index of section style to delete.
                               Must be in the range 0 <= index < SectionStyleCount().
    bQuiet [in] - Set true to disable any warning or error messages.
  Returns:
     true if successful, or false if index is out of range or the section style cannot be deleted.
  */
  bool DeleteSectionStyle(int section_style_index, bool bQuiet);
  bool DeleteSectionStyle(const ON_ComponentManifestItem& item, bool bQuiet);
  
  /*
  Description:
    Delete multiple section styles.
  Parameters:
    section_style_count    - [in] length of section_style_indices[] array.
    section_style_indices  - [in] array zero based indices of section style to delete.
                             The indices must be in the range 0 <= index < SectionStyleCount().
    bQuiet                 - [in] If true, no warning message box appears if a section style
                             cannot be deleted because it is in use.
  Returns:
    Number of section styles that were deleted. If this is < section_style_count,
    you can dig through the section style table and check which ones were not deleted.
  */
  int DeleteSectionStyles(
    int section_style_count,
    const int* section_style_indices,
    bool bQuiet
  );

  /*
  Description:
    Delete multiple section styles.
  Parameters:
    section_style_count    - [in] length of section_style_indices[] array.
    section_style_indices  - [in] array zero based indices of section style to delete.
                             The indices must be in the range 0 <= index < SectionStyleCount().
    bQuiet                 - [in] If true, no warning message box appears if a section style
                             cannot be deleted because it is in use.
    delete_warning         - [in] Action to take when multiple delete warnings occur, where:
                             0 == No to all, 1 == Yes to all, 2 == Ask the user.
  Returns:
    Number of section styles that were deleted. If this is < section_style_count,
    you can dig through the section style table and check which ones were not deleted.
  */
  int DeleteSectionStyles(
    int section_style_count, 
    const int* section_style_indices, 
    bool bQuiet,
    int delete_warning
  );

  /*
  Description:
    Undeletes a section style that has been deleted by DeleteSectionStyle().
  Parameters:
    section_style_index [in] - Zero based index of section style to delete.
                               Must be in the range 0 <= index < SectionStyleCount().
   Returns:
     true if successful, false otherwise.
  */
  bool UndeleteSectionStyle(int section_style_index);

  /*
  Description:
    Returns true if a section style is use by an object, layer, or instance definition.
  Parameters:
    hatch_pattern_index - [in] index of section style to query.
  Returns:
    true if successful.
  */
  bool InUse(int section_style_index) const;
  bool InUse(int section_style_index, int* idef_count, int* object_count, int* layer_count) const;

  /*
  Returns:
    true if the section style is an element in this table.
  */
  bool InSectionStyleTable(const ON_SectionStyle* section_style) const;

  /*
  Returns:
    true if the section style index is a valid index of a section style element in this table.
  */
  bool IsValidSectionStyleTableIndex(int section_style_index) const;

  /*
  Description:
    Use this to set CRhinoSectionStyle::m_sort_index so that
    the values of CRhinoSectionStyle::m_sort_index run from 0 to SectionStyleCount() - 1
    and compare(CRhinoSectionStyleTable[i],CRhinoSectionStyleTable[j]) < 0) implies 
    CRhinoSectionStyleTable[i].m_sort_index < CRhinoSectionStyleTable[j].m_sort_index.
  Parameters:
    compare [in] - Compare function that returns <0 if arg1<arg2, 0 if arg1=arg2, and >0 if arg1>arg2.
    cull [in] - optional filter function with prototype that returns true if the section style should be ignored when sorting.
                Section styles that are ignored are not included in the list returned by GetSortedList().
                Pass nullptr if you do not need to cull section style.
    ptr [in] - pointer passed as last argument to compare() and cull().
  Remarks:
    After calling Sort(), you can repeatedly call GetSortedList() to get a sorted list of CRhinoSectionStyle pointers.
    Sort() and GetSortedList() do not modify the order or persistent information in the table.
    They are intended to be used to get sorted lists for dialogs, etc.
  */
  void Sort(int (*compar)(const CRhinoSectionStyle*, const CRhinoSectionStyle*, void*),
    int (*cull)(const CRhinoSectionStyle*, void*),
    void* ptr
  );

  /*
  Description:
    Gets an array of pointers to section styles that is sorted by
    the values of CRhinoSectionStyle::Name().
  Parameters:
    sorted_list [out] - this array is returned with length HatchPatternCount()
                        and is sorted by the values of CRhinoSectionStyle::Name().
    bIgnoreDeleted [in] - set true to ignore deleted section styles.
  Remarks:
    Use Sort() to set the values of CRhinoSectionStyle::m_sort_index.
  */
  void GetSortedList(
    ON_SimpleArray<const CRhinoSectionStyle*>& sorted_list,
    bool bIgnoreDeleted = false
  ) const;

  /*
  Description:
    Gets an array of pointers to section styles that is sorted by
    the values of CRhinoSectionStyle::Name().
  Parameters:
    sorted_list [out] - this array is returned with length HatchPatternCount()
                        and is sorted by the values of CRhinoSectionStyle::Name().
    bIgnoreDeleted [in] - set true to ignore deleted section styles.
    bIgnoreReference [in] - set true to ignore reference section styles.
  Remarks:
    Use Sort() to set the values of CRhinoSectionStyle::m_sort_index.
  */
  void GetSortedList(
    ON_SimpleArray<const CRhinoSectionStyle*>& sorted_list,
    bool bIgnoreDeleted,
    bool bIgnoreReference
  ) const;

  void SetRemapIndex(int section_style_index, int remap_index);

  // Built-in section styles.
  // Localized names, id set, unique, and persistent
  const CRhinoSectionStyle& Default; // index = -1, id set, unique, and persistent

private:
  friend class CRhDocSdkExtension;
  friend class CRhinoDoc;
  CRhinoSectionStyleTable(CRhinoDoc&);
  ~CRhinoSectionStyleTable();

  bool Internal_DeleteSectionStyleHelper(
    int section_style_index,
    bool bQuiet,
    int* delete_referenced_section_styles,
    int* reference_file_warning_enabled
  );

  unsigned int Internal_PurgeWorkSessionReferenceModel(unsigned int worksession_ref_model_sn);
  bool Internal_AddUndoModifySectionStyleRecord(int section_style_index);
  void Internal_ClearUndoRecords();
  void Internal_Destroy();

private:
  CRhinoSectionStyleTable() = delete;
  CRhinoSectionStyleTable(const CRhinoSectionStyleTable&) = delete;
  CRhinoSectionStyleTable& operator=(const CRhinoSectionStyleTable&) = delete;

private:
  CRhinoDoc& m_doc;
  CRhinoDocTableReference m_table_reference;
  ON_ComponentManifest& Internal_Manifest();

private:
  // This must be private.
  CRhinoSectionStyle m_Default;
  ON__UINT_PTR m_sdk_reserved0 = 0;
  ON__UINT_PTR m_sdk_reserved1 = 0;
  ON__UINT_PTR m_sdk_reserved2 = 0;
  ON__UINT_PTR m_sdk_reserved3 = 0;
};

#endif
