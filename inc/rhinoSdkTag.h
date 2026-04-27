//
// Copyright (c) 1993-2015 Robert McNeel & Associates. All rights reserved.
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

#if defined(OPENNURBS_TAG_WIP)

class CRhinoObject;

/// <summary>
/// CRhinoTag contains a tag definition and is stored in the CRhinoDoc tag table.
/// </summary>
class RHINO_SDK_CLASS CRhinoTag : public ON_Tag
{
public:
  static unsigned int NextRuntimeSerialNumber();

  /// <summary>
  /// Tags are always in the document.
  /// </summary>
  /// <returns>
  /// The document that owns this Tags.
  /// </returns>
  CRhinoDoc& Document() const;

  /// <summary>
  /// The worksession reference model serial number is a runtime value.
  /// It is not saved in files and it is generally different the next time a file is read.
  /// </summary>
  /// <returns>
  /// Worksession reference model serial number, where:
  ///   0 : Tag is not in a reference model.
  ///   1 : Tag is in an unidentified worksession reference model.
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
  ///   0 : Tag is not part of a linked instance definition.
  ///   1 - 1000 : Reserved for future use.
  ///   > 1000 : CRhinoDoc::m_instance_defintion_table.FindBySerialNumber()
  ///            can be used to find the instance definition.
  /// </returns>
  unsigned int LinkedInstanceDefinitionSerialNumber() const;

  /// <summary>
  /// Get an array of all of the objects in this tag.
  /// </summary>
  /// <param name="members">Array to append the objects in the tag.</param>
  void Members(ON_SimpleArray<CRhinoObject*>& members) const;

  /// <summary>
  /// Runtime index used to sort tags.
  /// </summary>
  int m_sort_index = -1;

  /// <summary>
  /// Runtime index used when remapping tags for import/export.
  /// </summary>
  int m_remap_index = -1;

  /// <summary>
  /// Runtime serial number of this tag.
  /// This serial number is unique for each tag in an instance of a Rhino application.
  /// This serial number is not saved in files and it is generally different the next time a file is read.
  /// </summary>
  const unsigned int m_runtime_serial_number;

private:
  friend class CRhinoTagTable;
  const class CRhinoTagTable& m_doc_tag_table;

private:
  CRhinoTag(CRhinoTagTable&, const ON_Tag&);
  ~CRhinoTag();
  CRhinoTag() = delete;
  CRhinoTag(const CRhinoTag&) = delete;
  CRhinoTag& operator=(const CRhinoTag&) = delete;

private:
  ON__UINT_PTR m_sdk_reserved0 = 0;
  ON__UINT_PTR m_sdk_reserved1 = 0;
  ON__UINT_PTR m_sdk_reserved2 = 0;
  ON__UINT_PTR m_sdk_reserved3 = 0;
};


/// <summary>
/// CRhinoTagTable contains a document tag objects.
/// </summary>
class RHINO_SDK_CLASS CRhinoTagTable : private ON_SimpleArray<CRhinoTag*>
{
public:
  /// <returns>
  /// CRhinoDocument that owns this tag table.
  /// </returns>
  CRhinoDoc& Document() const;

  /// <returns>
  /// The document table reference.
  /// </returns>
  const class CRhinoDocTableReference& TableReference() const;

  /// <returns>
  /// Number of tags in the tag table.
  /// </returns>
  int TagCount() const;

  /// <summary>
  /// Gets a tag object from the tag table.
  /// </summary>
  /// <param name="tag_index">Zero-based tag index.</param>
  /// <returns>
  /// Pointer to the tag. If tag_index is out of range, nullptr is returned.
  /// Note, this pointer may become invalid after CRhinoTagTable::AddTag is called.
  /// </returns>
  const CRhinoTag* operator[](int tag_index) const;

  /// <summary>
  /// Finds the tag with a given name. Case is ignored.
  /// </summary>
  /// <param name="tag_name">Tag name.</param>
  /// <returns>
  /// >=0 : CRhinoDoc runtime tag index of the tag with name = tag_name.
  /// ON_UNSET_INT_INDEX : tag with specified name does not exist.
  /// </returns>
  int FindTagFromName(const wchar_t* tag_name) const;

  /// <summary>
  /// Finds the tag with a given id.
  /// </summary>
  /// <param name="tag_id">Tag id.</param>
  /// <param name="bSearchDeletedTags">Set true to search deleted tags.</param>
  /// <returns>
  /// >=0 : CRhinoDoc runtime tag index of the tag with id = tag_id.
  /// ON_UNSET_INT_INDEX : tag with specified id does not exist.
  /// </returns>
  int FindTagFromId(
    ON_UUID tag_id,
    bool bSearchDeletedTags
  ) const;

  /// <summary>
  /// Adds a new tag to the tag table.
  /// </summary>
  /// <returns>
  /// >=0 : index of new tag
  /// ON_UNSET_INT_INDEX: tag not added because input is not valid.
  /// </returns>
  int AddTag();

  /// <summary>
  /// Adds a new tag to the tag table.
  /// </summary>
  /// <param name="tag">Tag to add.</param>
  /// <returns>
  /// >=0 : index of new tag
  /// ON_UNSET_INT_INDEX: tag not added because input is not valid.
  /// </returns>
  int AddTag(const ON_Tag& tag);

  /// <summary>
  /// Adds a new tag to the tag table, and adds Rhino object to the tag.
  /// </summary>
  /// <param name="tag_members">Rhino objects to tag.</param>
  /// <returns>
  /// >=0 : index of new tag
  /// ON_UNSET_INT_INDEX: tag not added because input is not valid.
  /// </returns>
  int AddTag(
    ON_SimpleArray<const CRhinoObject*>& tag_members
  );

  /// <summary>
  /// Adds a new tag to the tag table, and adds Rhino object to the tag.
  /// </summary>
  /// <param name="tag">Tag to add.</param>
  /// <param name="tag_members">Rhino objects to tag.</param>
  /// <returns>
  /// >=0 : index of new tag
  /// ON_UNSET_INT_INDEX: tag not added because input is not valid.
  /// </returns>
  int AddTag(
    const ON_Tag& tag,
    ON_SimpleArray<const CRhinoObject*>& tag_members
  );

  /// <summary>
  /// Basic tool used to create a new tag with specified definition to the tag table.
  /// The AddTag() functions use this when addind new tags.
  /// </summary>
  /// <param name="tag">Tag to add.
  /// The name and id are used as candidates.
  /// If they are in use, a new name or id is created.
  /// </param>
  /// <param name="worksession_ref_model_sn">
  /// The worksession reference model serial number:
  /// 0 : Not from a reference model.
  /// 1 : Not saved in files but not part of a worksession reference file.
  /// 2 - 1000 : Reserved for future use.
  /// > 1000 : Norksession reference file serial number.
  /// </param>
  /// <param name="linked_idef_sn">
  /// Linked instance definition serial number:
  ///   0 : Not from a linked instance definition.
  ///   1 - 1000 : Reserved for future use.
  ///   > 1000 : Linked nstance definition serial number.
  /// </param>
  /// >=0 : index of new tag
  /// ON_UNSET_INT_INDEX: tag not added because input is not valid.
  int CreateTag(
    const ON_Tag& tag,
    unsigned int worksession_ref_model_sn,
    unsigned int linked_idef_sn
  );

  /// <summary>
  /// Basic tool used to create a new tag with specified definition to the tag table.
  /// The AddTag() functions use this when addind new tags.
  /// </summary>
  /// <param name="tag">Tag to add.
  /// The name and id are used as candidates.
  /// If they are in use, a new name or id is created.
  /// </param>
  /// <param name="tag_members">Rhino objects to tag.</param>
  /// <param name="worksession_ref_model_sn">
  /// The worksession reference model serial number:
  /// 0 : Not from a reference model.
  /// 1 : Not saved in files but not part of a worksession reference file.
  /// 2 - 1000 : Reserved for future use.
  /// > 1000 : Norksession reference file serial number.
  /// </param>
  /// <param name="linked_idef_sn">
  /// Linked instance definition serial number:
  ///   0 : Not from a linked instance definition.
  ///   1 - 1000 : Reserved for future use.
  ///   > 1000 : Linked nstance definition serial number.
  /// </param>
  /// >=0 : index of new tag
  /// ON_UNSET_INT_INDEX: tag not added because input is not valid.
  int CreateTag(
    const ON_Tag& tag,
    ON_SimpleArray<const CRhinoObject*>& tag_members,
    unsigned int worksession_ref_model_sn,
    unsigned int linked_idef_sn
  );

  /// <summary>
  /// Modifies a tag's settings.
  /// </summary>
  /// <param name="tag">Tag to modify.</param>
  /// <param name="tag_index">Zero-based tag index.</param>
  /// <param name="bQuiet">true to disable message boxes when invalid names are used.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool ModifyTag(
    const ON_Tag& tag,
    int tag_index,
    bool bQuiet = false
  );

  /// <summary>
  /// Deletes a tag.
  /// </summary>
  /// <param name="tag_index">Index of tag to delete.</param>
  /// <returns>
  /// true if successful.
  /// false if tag_index is out of range or thethe tag cannot be deleted
  /// because it contains active geometry.
  /// </returns>
  bool DeleteTag(int tag_index);

  /// <summary>
  /// Deletes a tag.
  /// </summary>
  /// <param name="tag_index">Index of tag to delete.</param>
  /// <param name="remove_objects">Iterates the document and removes objects from the tag.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool DeleteTag(
    int tag_index,
    bool remove_objects
  );

  /// <summary>
  /// Undeletes a tag that has been deleted by DeleteTag().
  /// </summary>
  /// <param name="tag_index">Index of tag to undelete.</param>
  /// <returns>true if successful, false otherwise.</returns>
  bool UndeleteTag(int tag_index);

  /// <summary>
  /// Use this to set CRhinoTag::m_sort_index so that the values of 
  /// CRhinoTag::m_sort_index are from 0 to TagCount() - 1, and
  ///  compare(tag_table[i], tag_table[j]) < 0 ) implies  that 
  /// tag_table[i].m_sort_index < tag_table[j].m_sort_index.
  /// <param name="compare">
  /// Compare function that returns:
  ///   <0 : arg1 < arg2
  ///    0 : arg1 = arg2
  //    >0 : arg1 > arg2
  /// </param>
  /// <param name="compare">
  /// Optional culling function that returns true if the tag should be ignored
  /// when sorting. Tags that are ignored are not included in the list returned
  /// by GetSortedList(). Pass nullptr if you do not need to cull the list.
  /// </param>
  /// <param name="ptr">
  /// Pointer passed as last argument to compare() and cull().
  /// </param>
  void Sort(
    int (*compare)(const CRhinoTag*, const CRhinoTag*, void* p),
    int (*cull)(const CRhinoTag*, void*),
    void* ptr = nullptr
  );

  /// <summary>
  /// Gets an array of tag pointers that are sorted by CRhinoTag::Name().
  /// /// </summary>
  /// <param name="sorted_list">Array of sorted tag pointers.</param>
  /// <param name="bIgnoreDeleted">Set true to ignore deleted tags.</param>
  /// <param name="bIgnoreReference">set true to ignore reference tags.</param>
  /// <remarks>
  /// Use Sort() to set the values of CRhinoTag::m_sort_index.
  /// </remarks>
  void GetSortedList(
    ON_SimpleArray<const CRhinoTag*>& sorted_list,
    bool bIgnoreDeleted,
    bool bIgnoreReference
  ) const;

  /// <summary>
  /// Gets an unused tag name.
  /// </summary>
  /// <param name="unused_tag_name">The unused tag name.</param>
  void GetUnusedTagName(
    ON_wString& unused_tag_name
  );

  /// <summary>
  /// Gets an unused tag name.
  /// </summary>
  /// <param name="root_name">The name's root or prefix. If nullptr, "Tag" is used.</param>
  /// <param name="unused_tag_name">The unused tag name.</param>
  void GetUnusedTagName(
    const wchar_t* root_name,
    ON_wString& unused_tag_name
  );

  /// <summary>
  /// Sets a tag's name to an unused name.
  /// </summary>
  /// <param name="tag_to_name">The tag to assign a unused name.</param>
  void SetUnusedTagName(
    ON_Tag& tag_to_name
  );

  /// <summary>
  /// Sets a tag's name to an unused name.
  /// </summary>
  /// <param name="root_name">The name's root or prefix. If nullptr, "Tag" is used.</param>
  /// <param name="tag_to_name">The tag to assign a unused name.</param>
  void SetUnusedTagName(
    const wchar_t* root_name,
    ON_Tag& tag_to_name
  );

  /// <summary>
  /// Sets a tag's remap index.
  /// </summary>
  /// <param name="tag_index">Index of a existing tag in this tag table.</param>
  /// <param name="remap_index">Remap index.</param>
  void SetRemapIndex(
    int tag_index,
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
  /// Verifies a tag exists in this tag table.
  /// </summary>
  /// <param name="tag">A valid tag.</param>
  /// <returns>True if the tag exists.</returns>
  bool InTagTable(const ON_Tag* tag) const;

  /// <summary>
  /// Verifies a tag index is a valid index of a tag in this table.
  /// </summary>
  /// <param name="tag_index">Tag index.</param>
  /// <returns>True if the tag index is valid.</returns>
  bool IsValidTagTableIndex(int tag_index) const;

private:
  friend class CRhDocSdkExtension;
  friend class CRhinoDoc;

private:
  CRhinoTagTable(CRhinoDoc&);
  ~CRhinoTagTable();

  CRhinoTagTable() = delete;
  CRhinoTagTable(const CRhinoTagTable&) = delete;
  CRhinoTagTable& operator=(const CRhinoTagTable&) = delete;

private:
  CRhinoDoc& m_doc;
  CRhinoDocTableReference m_table_reference;
  ON_ComponentManifest& Internal_Manifest();

private:
  void Destroy(class CRhinoEventWatcher* ew);
  void Destroy();

  ON__UINT_PTR m_sdk_reserved0 = 0;
  ON__UINT_PTR m_sdk_reserved1 = 0;
  ON__UINT_PTR m_sdk_reserved2 = 0;
  ON__UINT_PTR m_sdk_reserved3 = 0;
};

#endif // OPENNURBS_TAG_WIP
