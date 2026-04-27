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

#if defined(OPENNURBS_PAGEVIEWGROUP_WIP)

class CRhinoPageView;

/// <summary>
/// CRhinoPageViewGroup contains a page view group definition,
/// and it is stored in the CRhinoDoc page view group table.
/// </summary>
class RHINO_SDK_CLASS CRhinoPageViewGroup : public ON_PageViewGroup
{
public:
  static unsigned int NextRuntimeSerialNumber();

  /// <summary>
  /// Page view groups are always in the document.
  /// </summary>
  /// <returns>
  /// The document that owns this page view group.
  /// </returns>
  CRhinoDoc& Document() const;

  /// <summary>
  /// The worksession reference model serial number is a runtime value.
  /// It is not saved in files and it is generally different the next time a file is read.
  /// </summary>
  /// <returns>
  /// Worksession reference model serial number, where:
  ///   0 : Group is not in a reference model.
  ///   1 : Group is in an unidentified worksession reference model.
  ///   2 - 1000 : Reserved for future use.
  ///   > 1000 : Serial number of the worksession reference model.
  /// </returns>
  unsigned int WorksessionReferenceModelSerialNumber() const;

  /// <summary>
  /// The linked instance definition serial number is a runtime value.
  /// It is not saved in files and it is generally different the next time a file is read.
  /// </summary>
  /// <returns>
  /// Linked instance definition serial number, where:
  ///   0 : Group is not part of a linked instance definition.
  ///   1 - 1000 : Reserved for future use.
  ///   > 1000 : CRhinoDoc::m_instance_defintion_table.FindBySerialNumber()
  ///            can be used to find the instance definition.
  /// </returns>
  unsigned int LinkedInstanceDefinitionSerialNumber() const;

  /// <summary>
  /// Get an array of all of the page views in this page view group.
  /// </summary>
  /// <param name="members">Array to append the page views in the page view group.</param>
  /// <returns>Number of page views appended to output array.</returns>
  int Members(
    ON_SimpleArray<CRhinoPageView*>& members
  ) const;

  /// <summary>
  /// Runtime index used to sort page view groups.
  /// </summary>
  int m_sort_index = -1;

  /// <summary>
  /// Runtime index used when remapping page view groups for import/export.
  /// </summary>
  int m_remap_index = -1;

  /// <summary>
  /// Runtime serial number of this page view group.
  /// This serial number is unique for each page view group in an instance of a Rhino application.
  /// This serial number is not saved in files and it is generally different the next time a file is read.
  /// </summary>
  const unsigned int m_runtime_serial_number;

private:
  friend class CRhinoPageViewGroupTable;
  const class CRhinoPageViewGroupTable& m_doc_pageview_group_table;
  class CRhPageViewGroupUndo* m_undo = nullptr;

private:
  CRhinoPageViewGroup(CRhinoPageViewGroupTable&, const ON_PageViewGroup&);
  ~CRhinoPageViewGroup();
  CRhinoPageViewGroup() = delete;
  CRhinoPageViewGroup(const CRhinoPageViewGroup&) = delete;
  CRhinoPageViewGroup& operator=(const CRhinoPageViewGroup&) = delete;

  // Delete all CRhinoPageViewGroup.m_undo information
  void ClearUndoRecords();

private:
  ON__UINT_PTR m_sdk_reserved0 = 0;
  ON__UINT_PTR m_sdk_reserved1 = 0;
  ON__UINT_PTR m_sdk_reserved2 = 0;
  ON__UINT_PTR m_sdk_reserved3 = 0;
};

/// <summary>
/// CRhinoPageViewGroupTable contains document page view group objects.
/// </summary>
class RHINO_SDK_CLASS CRhinoPageViewGroupTable : private ON_SimpleArray<CRhinoPageViewGroup*>
{
public:
  /// <returns>
  /// CRhinoDocument that owns this table.
  /// </returns>
  CRhinoDoc& Document() const;

  /// <returns>
  /// The document table reference.
  /// </returns>
  const class CRhinoDocTableReference& TableReference() const;

  /// <returns>
  /// Number of page view groups in the table.
  /// </returns>
  int PageViewGroupCount() const;

  /// <summary>
  /// Gets a page view group from the table.
  /// </summary>
  /// <param name="group_index">Zero-based page view group index.</param>
  /// <returns>
  /// Pointer to the page view. If group_index is out of range, nullptr is returned.
  /// Note, this pointer may become invalid after CRhinoPageViewGroupTable::AddPageViewGroup is called.
  /// </returns>
  const CRhinoPageViewGroup* operator[](int group_index) const;

  /// <summary>
  /// Gets a page view group by component manifest.
  /// </summary>
  /// <param name="manifest_item">The component manifest item.</param>
  /// <returns>
  /// If successful, a pointer to the page view group is returned.
  /// If not successful, nullptr is returned.
  /// </returns>
  const CRhinoPageViewGroup* PageViewGroup(
    const ON_ComponentManifestItem& manifest_item
  ) const;

  /// <summary>
  /// Gets a page view group by component manifest.
  /// </summary>
  /// <param name="manifest_item">The component manifest item.</param>
  /// <param name="not_found">The value to return if not found.</param>
  /// <returns>
  /// If successful, a pointer to the page view group is returned.
  /// If not successful, nullptr is returned.
  /// </returns>
  const CRhinoPageViewGroup* PageViewGroup(
    const ON_ComponentManifestItem& manifest_item,
    const CRhinoPageViewGroup* not_found
  ) const;

  /// <summary>
  /// Gets a component manifest item by page view group name.
  /// </summary>
  /// <param name="group_name">The page view group name.</param>
  /// <returns>
  /// If successful, a ON_ModelComponent::Type::PageViewGroup component manifest item.
  /// If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  /// </returns>
  const ON_ComponentManifestItem& PageViewGroupManifestItemFromName(
    const wchar_t* group_name
  ) const;

  /// <summary>
  /// Gets a page view group by name.
  /// </summary>
  /// <param name="group_name">The page view group name.</param>
  /// <returns>
  /// If successful, a pointer to the page view group is returned.
  /// If not successful, nullptr is returned.
  /// </returns>
  const CRhinoPageViewGroup* PageViewGroupFromName(
    const wchar_t* group_name
  ) const;

  /// <summary>
  /// Gets a component manifest item by page view group id.
  /// </summary>
  /// <param name="group_id">The page view group id.</param>
  /// <param name="bSearchDeleted">Set true to search for deleted page view groups.</param>
  /// <returns>
  /// If successful, a ON_ModelComponent::Type::PageViewGroup component manifest item.
  /// If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  /// </returns>
  const ON_ComponentManifestItem& PageViewGroupManifestItemFromId(
    ON_UUID group_id,
    bool bSearchDeleted
  ) const;

  /// <summary>
  /// Gets a page view group by id.
  /// </summary>
  /// <param name="group_id">The page view group id.</param>
  /// <param name="bSearchDeleted">Set true to search for deleted page view groups.</param>
  /// <returns>
  /// If successful, a pointer to the page view group is returned.
  /// If not successful, nullptr is returned.
  /// </returns>
  const CRhinoPageViewGroup* PageViewGroupFromId(
    ON_UUID group_id,
    bool bSearchDeleted
  ) const;

  /// <summary>
  /// Gets a component manifest item by page view group index.
  /// </summary>
  /// <param name="group_index">The page view group index.</param>
  /// <param name="bSearchDeleted">Set true to search for deleted page view groups.</param>
  /// <returns>
  /// If successful, a ON_ModelComponent::Type::PageViewGroup component manifest item.
  /// If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  /// </returns>
  const ON_ComponentManifestItem& PageViewGroupManifestItemFromIndex(
    int group_index,
    bool bSearchDeleted
  ) const;

  /// <summary>
  /// Gets a page view group by index.
  /// </summary>
  /// <param name="group_index">The page view group index.</param>
  /// <param name="bSearchDeleted">Set true to search for deleted page view groups.</param>
  /// <returns>
  /// If successful, a pointer to the page view group is returned.
  /// If not successful, nullptr is returned.
  /// </returns>
  const CRhinoPageViewGroup* PageViewGroupFromIndex(
    int group_index,
    bool bSearchDeleted
  ) const;

  /// <summary>
  /// Adds a new page view group to the table.
  /// </summary>
  /// <returns>
  /// If successful, a ON_ModelComponent::Type::PageViewGroup component manifest item.
  /// If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  /// </returns>
  const ON_ComponentManifestItem& AddPageViewGroup();

  /// <summary>
  /// Adds a new page view group to the table.
  /// </summary>
  /// <param name="group">Page view group to add.</param>
  /// <returns>
  /// If successful, a ON_ModelComponent::Type::PageViewGroup component manifest item.
  /// If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  /// </returns>
  const ON_ComponentManifestItem& AddPageViewGroup(
    const ON_PageViewGroup& group
  );

  /// <summary>
  /// Adds a new page view group to the table, and adds page views to the group.
  /// </summary>
  /// <param name="group_members">Rhino page views to group.</param>
  /// <returns>
  /// If successful, a ON_ModelComponent::Type::PageViewGroup component manifest item.
  /// If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  /// </returns>
  const ON_ComponentManifestItem& AddPageViewGroup(
    const ON_SimpleArray<CRhinoPageView*>& group_members
  );

  /// <summary>
  /// Adds a new page view group to the table, and adds page views to the group.
  /// </summary>
  /// <param name="group">Page view group to add.</param>
  /// <param name="group_members">Rhino page views to group.</param>
  /// <returns>
  /// If successful, a ON_ModelComponent::Type::PageViewGroup component manifest item.
  /// If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  /// </returns>
  const ON_ComponentManifestItem& AddPageViewGroup(
    const ON_PageViewGroup& group,
    const ON_SimpleArray<CRhinoPageView*>& group_members
  );

  /// <summary>
  /// Basic tool used to add a new new page view group, with specified definition, to the table.
  /// The AddPageViewGroup() functions use this when adding new age view groups.
  /// </summary>
  /// <param name="group">Page view group to add.</param>
  /// <param name="worksession_ref_model_sn">
  /// The worksession reference model serial number:
  ///   0 : Not from a reference model.
  ///   1 : Not saved in files but not part of a worksession reference file.
  ///   2 - 1000 : Reserved for future use.
  ///   > 1000 : Norksession reference file serial number.
  /// </param>
  /// <param name="linked_idef_sn">
  /// Linked instance definition serial number:
  ///   0 : Not from a linked instance definition.
  ///   1 - 1000 : Reserved for future use.
  ///   > 1000 : Linked nstance definition serial number.
  /// </param>
  /// <returns>
  /// If successful, a ON_ModelComponent::Type::PageViewGroup component manifest item.
  /// If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  /// </returns>
  const ON_ComponentManifestItem& CreatePageViewGroup(
    const ON_PageViewGroup& group,
    unsigned int worksession_ref_model_sn,
    unsigned int linked_idef_sn
  );

  /// <summary>
  /// Basic tool used to add a new new page view group, with specified definition, to the table.
  /// The AddPageViewGroup() functions use this when adding new age view groups.
  /// </summary>
  /// <param name="group">Page view group to add.</param>
  /// <param name="group_members">Rhino page views to group.</param>
  /// <param name="worksession_ref_model_sn">
  /// The worksession reference model serial number:
  ///   0 : Not from a reference model.
  ///   1 : Not saved in files but not part of a worksession reference file.
  ///   2 - 1000 : Reserved for future use.
  ///   > 1000 : Norksession reference file serial number.
  /// </param>
  /// <param name="linked_idef_sn">
  /// Linked instance definition serial number:
  ///   0 : Not from a linked instance definition.
  ///   1 - 1000 : Reserved for future use.
  ///   > 1000 : Linked nstance definition serial number.
  /// </param>
  /// <returns>
  /// If successful, a ON_ModelComponent::Type::PageViewGroup component manifest item.
  /// If not successful, ON_ComponentManifestItem::UnsetItem is returned.
  /// </returns>
  const ON_ComponentManifestItem& CreatePageViewGroup(
    const ON_PageViewGroup& group,
    const ON_SimpleArray<CRhinoPageView*>& group_members,
    unsigned int worksession_ref_model_sn,
    unsigned int linked_idef_sn
  );

  /// <summary>
  /// Modifies a page view group.
  /// </summary>
  /// <param name="group">The modified page view group.</param>
  /// <param name="group_index">Zero-based index of page view group to modify.</param>
  /// <param name="bQuiet">true to disable any warning or error messages.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool ModifyPageViewGroup(
    const ON_PageViewGroup& group,
    int group_index,
    bool bQuiet = false
  );

  /// <summary>
  /// Modifies a page view group.
  /// </summary>
  /// <param name="group">The modified page view group.</param>
  /// <param name="item">Component index item of page view group to modify.</param>
  /// <param name="bQuiet">true to disable any warning or error messages.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool ModifyPageViewGroup(
    const ON_PageViewGroup& group, 
    const ON_ComponentManifestItem& item, 
    bool bQuiet
  );

  /// <summary>
  /// If the page view group has been modified and the modification can be undone,
  /// then UndoModifyPageViewGroup() will restore the page view group to its previous state.
  /// </summary>
  /// <param name="group_index">Zero-based index of page view group.</param>
  /// <param name="undo_record_sn">Undo record serial number.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool UndoModifyPageViewGroup(
    int group_index,
    unsigned int undo_record_sn
  );

  /// <summary>
  /// Deletes a page view group.
  /// </summary>
  /// <param name="group_index">Zero-based index of page view group to delete.</param>
  /// <param name="bQuiet">true to disable any warning or error messages.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool DeletePageViewGroup(
    int group_index,
    bool bQuiet
  );

  /// <summary>
  /// Deletes a page view group.
  /// </summary>
  /// <param name="group_index">Zero-based index of page view group to delete.</param>
  /// <param name="bRemovePageViews">Iterates the document and removes page views from the group.</param>
  /// <param name="bQuiet">true to disable any warning or error messages.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool DeletePageViewGroup(
    int group_index,
    bool bRemovePageViews,
    bool bQuiet
  );

  /// <summary>
  /// Deletes a page view group.
  /// </summary>
  /// <param name="item">Component index item of page view group to delete.</param>
  /// <param name="bRemovePageViews">Iterates the document and removes page views from the group.</param>
  /// <param name="bQuiet">true to disable any warning or error messages.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool DeletePageViewGroup(
    const ON_ComponentManifestItem& item,
    bool bRemovePageViews,
    bool bQuiet
  );

  /// <summary>
  /// Undeletes a page view group that has been deleted by DeletePageViewGroup().
  /// </summary>
  /// <param name="group_index">Zero-based index of page view group to undelete.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool UndeletePageViewGroup(
    int group_index
  );

  /// <summary>
  /// Use this to set CRhinoPageViewGroup::m_sort_index so that the values of 
  /// CRhinoPageViewGroup::m_sort_index are from 0 to PageViewGroupCount() - 1, and
  /// compare(table[i], table[j]) < 0 ) implies that table[i].m_sort_index < table[j].m_sort_index.
  /// </summary>
  /// <param name="compare">
  /// Compare function that returns:
  ///   <0 : arg1 < arg2
  ///    0 : arg1 = arg2
  //    >0 : arg1 > arg2
  /// </param>
  /// <param name="cull">
  /// Optional culling function that returns true if the page view group should be ignored
  /// when sorting. Page view group that are ignored are not included in the list returned
  /// by GetSortedList(). Pass nullptr if you do not need to cull the list.
  /// </param>
  /// <param name="ptr">
  /// Pointer passed as last argument to compare() and cull().
  /// </param>
  void Sort(
    int (*compare)(const CRhinoPageViewGroup*, const CRhinoPageViewGroup*, void*),
    int (*cull)(const CRhinoPageViewGroup*, void*),
    void* ptr = nullptr
  );

  /// <summary>
  /// Gets an array of page view group pointers that are sorted by CRhinoPageViewGroup::Name().
  /// </summary>
  /// <param name="sorted_list">Array of sorted page view group pointers.</param>
  /// <param name="bIgnoreDeleted">true to ignore deleted page view groups.</param>
  /// <param name="bIgnoreReference">true to ignore reference page view groups.</param>
  /// <remarks>
  /// Use Sort() to set the values of CRhinoPageViewGroup::m_sort_index.
  /// </remarks>
  void GetSortedList(
    ON_SimpleArray<const CRhinoPageViewGroup*>& sorted_list,
    bool bIgnoreDeleted,
    bool bIgnoreReference
  ) const;

  /// <summary>
  /// Gets an unused page view group name.
  /// </summary>
  /// <param name="unused_group_name">The unused page view name.</param>
  void GetUnusedPageViewGroupName(
    ON_wString& unused_group_name
  );

  /// <summary>
  /// Gets an unused page view group name.
  /// </summary>
  /// <param name="root_name">The name's root or prefix.</param>
  /// <param name="unused_group_name">The unused page view name.</param>
  void GetUnusedPageViewGroupName(
    const wchar_t* root_name,
    ON_wString& unused_group_name
  );

  /// <summary>
  /// Sets a page view group's name to an unused name.
  /// </summary>
  /// <param name="group_to_name">The page view group to assign a unused name.</param>
  void SetUnusedPageViewGroupName(
    ON_PageViewGroup& group_to_name
  );

  /// <summary>
  /// Sets a page view group's name to an unused name.
  /// </summary>
  /// <param name="root_name">The name's root or prefix. If nullptr, "PageViewGroup" is used.</param>
  /// <param name="group_to_name">The page view group to assign a unused name.</param>
  void SetUnusedPageViewGroupName(
    const wchar_t* root_name,
    ON_PageViewGroup& group_to_name
  );

  /// <summary>
  /// Sets a page view group's remap index.
  /// </summary>
  /// <param name="group_index">Index of a existing page view group in this table.</param>
  /// <param name="remap_index">Remap index.</param>
  void SetRemapIndex(
    int group_index,
    int remap_index
  );

  /// <summary>
  /// Table logging helper.
  /// </summary>
  void ListTable(
    ON_TextLog& text_log,
    unsigned int level_of_detail
  ) const;

  /// <summary>
  /// Verifies a page view group exists in this table.
  /// </summary>
  /// <param name="group">A valid page view group.</param>
  /// <returns>true if the page view group exists, false otherwise.</returns>
  bool InPageViewGroupTable(const ON_PageViewGroup* group) const;

  /// <summary>
  /// Verifies a page view group index is a valid index in this table.
  /// </summary>
  /// <param name="group_index">Zero-based page view group index.</param>
  /// <returns>True if the page view group index is valid, false otherwise.</returns>
  bool IsValidPageViewGroupTableIndex(int group_index) const;

  void ChangeSortOrder(const ON_SimpleArray<ON_2dex>& sort_order);
  bool UnsortPageViewGroup(int pageview_group_index, int sort_index);

private:
  friend class CRhDocSdkExtension;
  friend class CRhinoDoc;

  bool Internal_DeleteHelper(
    int group_index,
    bool bRemovePageViews,
    bool bQuiet,
    int* delete_referenced,
    int* reference_file_warning_enabled
  );

  unsigned int Internal_PurgeWorkSessionReferenceModel(
    unsigned int worksession_ref_model_sn
  );

  bool Internal_AddUndoModifyRecord(
    int group_index
  );

  void Internal_ClearUndoRecords();
  void Internal_Destroy();

private:
  CRhinoPageViewGroupTable(CRhinoDoc&);
  ~CRhinoPageViewGroupTable();
  // NO implementation to prohibit use
  CRhinoPageViewGroupTable() = delete;
  CRhinoPageViewGroupTable(const CRhinoPageViewGroupTable&) = delete;
  CRhinoPageViewGroupTable& operator=(const CRhinoPageViewGroupTable&) = delete;

private:
  CRhinoDoc& m_doc;
  CRhinoDocTableReference m_table_reference;
  ON_ComponentManifest& Internal_Manifest();

private:
  ON__UINT_PTR m_sdk_reserved0 = 0;
  ON__UINT_PTR m_sdk_reserved1 = 0;
  ON__UINT_PTR m_sdk_reserved2 = 0;
  ON__UINT_PTR m_sdk_reserved3 = 0;
};

#endif // OPENNURBS_PAGEVIEWGROUP_WIP
