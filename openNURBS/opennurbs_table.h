//
// Copyright (c) 1993-2022 Robert McNeel & Associates. All rights reserved.
// OpenNURBS, Rhinoceros, and Rhino3D are registered trademarks of Robert
// McNeel & Associates.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
// ALL IMPLIED WARRANTIES OF FITNESS FOR ANY PARTICULAR PURPOSE AND OF
// MERCHANTABILITY ARE HEREBY DISCLAIMED.
//
// For complete openNURBS copyright information see <http://www.opennurbs.org>.
//
////////////////////////////////////////////////////////////////

#if !defined(OPENNURBS_TABLE_INC_)
#define OPENNURBS_TABLE_INC_

#if defined(OPENNURBS_ANNOTATION_TABLE_WIP)

#include "opennurbs_archivable_dictionary.h"
#include "opennurbs_annotationbase.h"

class ON_TableGridlinePrivate;
class ON_TableCellStylePrivate;
class ON_TableRowPrivate;
class ON_TableColumnPrivate;
class ON_TableMergeRegionPrivate;
class ON_TableCellPrivate;
class ON_TablePrivate;
class ON_AnnotationTablePrivate;

class ON_CLASS ON_TableDef
{
public:
  enum eRowType : unsigned int
  {
    data_row = 0,
    title_row = 1,
    header_row = 2,
    footer_row = 3
  };

  enum eCellAlignment : unsigned int
  {
    top_left = 0,
    top_center = 1,
    top_right = 2,
    middle_left = 3,
    middle_center = 4,
    middle_right = 5,
    bottom_left = 6,
    bottom_center = 7,
    bottom_right = 8
  };

  enum eCellContentType : unsigned int
  {
    empty_content = 0,
    text_content = 1,
    number_content = 2,
    formula_content = 3,
    field_content = 4,
    block_content = 5
  };
};

class ON_CLASS ON_TableGridline
{
public:
  ON_TableGridline();
  ~ON_TableGridline();
  ON_TableGridline(const ON_TableGridline&);
  ON_TableGridline& operator=(const ON_TableGridline&);

  bool IsValid(ON_TextLog* text_log = 0) const;
  bool Write(ON_BinaryArchive& archive) const;
  bool Read(ON_BinaryArchive& archive);

  bool Visible() const;
  void SetVisible(bool visible);
  ON_Color Color() const;
  void SetColor(ON_Color color);
  double PlotWeightMM() const;
  void SetPlotWeightMM(double plot_weight_mm);

private:
  ON_TableGridlinePrivate* m_private_ON_TableGridline;
};

class ON_CLASS ON_TableCellStyle
{
public:
  ON_TableCellStyle();
  ~ON_TableCellStyle();
  ON_TableCellStyle(const ON_TableCellStyle&);
  ON_TableCellStyle& operator=(const ON_TableCellStyle&);

  static const ON_TableCellStyle Empty;

  bool IsValid(ON_TextLog* text_log = 0) const;
  bool Write(ON_BinaryArchive& archive) const;
  bool Read(ON_BinaryArchive& archive);

  ON__INT32 Id() const;
  void SetId(ON__INT32 id);
  ON_TableDef::eCellAlignment Alignment() const;
  void SetAlignment(ON_TableDef::eCellAlignment alignment);
  ON_Color TextColor() const;
  void SetTextColor(ON_Color color);
  ON_Color FillColor() const;
  void SetFillColor(ON_Color color);
  bool FillEnabled() const;
  void SetFillEnabled(bool enabled);
  double TextHeight() const;
  void SetTextHeight(double text_height);
  double Margin(int index) const;
  bool SetMargin(int index, double margin);
  const ON_TableGridline& Gridline(int index) const;
  bool SetGridline(int index, const ON_TableGridline& gridline);
  const wchar_t* FontName() const;
  void SetFontName(const wchar_t* font_name);

private:
  ON_TableCellStylePrivate* m_private_ON_TableCellStyle;
};

class ON_CLASS ON_TableRow
{
public:
  ON_TableRow();
  ~ON_TableRow();
  ON_TableRow(const ON_TableRow&);
  ON_TableRow& operator=(const ON_TableRow&);

  static const ON_TableRow Empty;

  bool IsValid(ON_TextLog* text_log = 0) const;
  bool Write(ON_BinaryArchive& archive) const;
  bool Read(ON_BinaryArchive& archive);

  double Height() const;
  void SetHeight(double height);
  ON__INT32 StyleId() const;
  void SetStyleId(ON__INT32 style_id);
  bool Hidden() const;
  void SetHidden(bool hidden);
  ON_TableDef::eRowType RowType() const;
  void SetRowType(ON_TableDef::eRowType row_type);

private:
  ON_TableRowPrivate* m_private_ON_TableRow;
};

class ON_CLASS ON_TableColumn
{
public:
  ON_TableColumn();
  ~ON_TableColumn();
  ON_TableColumn(const ON_TableColumn&);
  ON_TableColumn& operator=(const ON_TableColumn&);

  static const ON_TableColumn Empty;

  bool IsValid(ON_TextLog* text_log = 0) const;
  bool Write(ON_BinaryArchive& archive) const;
  bool Read(ON_BinaryArchive& archive);

  double Width() const;
  void SetWidth(double width);
  ON__INT32 StyleId() const;
  void SetStyleId(ON__INT32 style_id);
  bool Hidden() const;
  void SetHidden(bool hidden);

private:
  ON_TableColumnPrivate* m_private_ON_TableColumn;
};

class ON_CLASS ON_TableMergeRegion
{
public:
  ON_TableMergeRegion();
  ~ON_TableMergeRegion();
  ON_TableMergeRegion(const ON_TableMergeRegion&);
  ON_TableMergeRegion& operator=(const ON_TableMergeRegion&);

  static const ON_TableMergeRegion Empty;

  bool IsValid(ON_TextLog* text_log = 0) const;
  bool Write(ON_BinaryArchive& archive) const;
  bool Read(ON_BinaryArchive& archive);
  bool Contains(ON__UINT32 row, ON__UINT32 col) const;
  bool IsOwner(ON__UINT32 row, ON__UINT32 col) const;
  ON__UINT32 TopRow() const;
  ON__UINT32 LeftColumn() const;
  ON__UINT32 RowCount() const;
  ON__UINT32 ColumnCount() const;
  void SetBounds(ON__UINT32 top_row, ON__UINT32 left_col, ON__UINT32 row_count, ON__UINT32 col_count);

private:
  ON_TableMergeRegionPrivate* m_private_ON_TableMergeRegion;
};

class ON_CLASS ON_TableCell
{
public:
  ON_TableCell();
  ~ON_TableCell();
  ON_TableCell(const ON_TableCell&);
  ON_TableCell& operator=(const ON_TableCell&);

  static const ON_TableCell Empty;

  bool IsValid(ON_TextLog* text_log = 0) const;
  bool Write(ON_BinaryArchive& archive) const;
  bool Read(ON_BinaryArchive& archive);

  const wchar_t* GetContentString() const;
  void SetContentString(const wchar_t* content);

  ON__INT32 StyleId() const;
  void SetStyleId(ON__INT32 style_id);
  ON__UINT32 RowSpan() const;
  void SetRowSpan(ON__UINT32 row_span);
  ON__UINT32 ColumnSpan() const;
  void SetColumnSpan(ON__UINT32 col_span);
  ON_TableDef::eCellContentType ContentType() const;
  void SetContentType(ON_TableDef::eCellContentType content_type);
  double NumberValue() const;
  void SetNumberValue(double number_value);
  const wchar_t* Formula() const;
  void SetFormula(const wchar_t* formula);
  const wchar_t* FieldExpression() const;
  void SetFieldExpression(const wchar_t* field_expression);
  ON_UUID BlockId() const;
  void SetBlockId(ON_UUID block_id);
  bool IsMergeOwner() const;
  void SetIsMergeOwner(bool is_merge_owner);

private:
  ON_TableCellPrivate* m_private_ON_TableCell;
};

class ON_CLASS ON_Table : public ON_Object
{
  ON_OBJECT_DECLARE(ON_Table);

public:
  ON_Table();
  virtual ~ON_Table();
  ON_Table(const ON_Table&);
  ON_Table& operator=(const ON_Table&);

  static const ON_Table Empty;
  static const double MinColWidth;
  static const double MinRowHeight;
  static const double MinTableWidth;
  static const double MinTableHeight;

  bool Create(
    ON__UINT32 row_count,
    ON__UINT32 column_count,
    bool has_title_row,
    bool has_header_row);

  void Destroy();

  bool IsValid(ON_TextLog* text_log = 0) const;
  void Dump(ON_TextLog& text_log) const;
  bool Write(ON_BinaryArchive& archive) const;
  bool Read(ON_BinaryArchive& archive);

  ON_Table* DuplicateTable() const;

  ON__UINT32 RowCount() const;
  void SetRowCount(ON__UINT32 rowcount);

  ON__UINT32 ColCount() const;
  void SetColCount(ON__UINT32 colcount);

  ON_TableCell* GetCell(ON__UINT32 row, ON__UINT32 col);
  bool GetCell(ON__UINT32 row, ON__UINT32 col, ON_TableCell& cell) const;

  const ON_TableCellStyle& CellStyle(ON__UINT32 row, ON__UINT32 col) const;
  const ON_TableCellStyle& CellStyle(ON__INT32 id) const;
  bool AppendCellStyle(const ON_TableCellStyle& style);
  bool ReplaceCellStyle(const ON_TableCellStyle& style);

  double TotalWidth() const;
  bool SetTotalWidth(double width);

  double TotalHeight() const;
  bool SetTotalHeight(double height);

  bool AppendRows(double height, ON__UINT32 num_rows = 1);

  bool SetColumnWidth(ON__UINT32 col, double width);
  bool SetRowHeight(ON__UINT32 row, double height);
  double ColumnWidth(ON__UINT32 col) const;
  double RowHeight(ON__UINT32 row) const;
  const ON_TableRow* Row(ON__UINT32 row) const;
  ON_TableRow* Row(ON__UINT32 row);
  const ON_TableColumn* Column(ON__UINT32 col) const;
  ON_TableColumn* Column(ON__UINT32 col);
  bool SetRowStyle(ON__UINT32 row, ON__INT32 style_id);
  bool SetColumnStyle(ON__UINT32 col, ON__INT32 style_id);
  ON__INT32 RowStyle(ON__UINT32 row) const;
  ON__INT32 ColumnStyle(ON__UINT32 col) const;
  ON_TableDef::eRowType RowType(ON__UINT32 row) const;
  bool SetRowType(ON__UINT32 row, ON_TableDef::eRowType row_type);
  bool HideRow(ON__UINT32 row, bool hidden);
  bool HideColumn(ON__UINT32 col, bool hidden);
  bool RowHidden(ON__UINT32 row) const;
  bool ColumnHidden(ON__UINT32 col) const;

  bool HasTitleRow() const;
  bool HasHeaderRow() const;

  int MergeRegionCount() const;
  const ON_TableMergeRegion* MergeRegion(int index) const;
  bool AppendMergeRegion(const ON_TableMergeRegion& merge_region);
  int CellMergeRegion(ON__UINT32 row, ON__UINT32 col) const;

  const ON_TableCellStyle& EffectiveCellStyle(ON__UINT32 row, ON__UINT32 col) const;

  bool GetCellCorners(
    ON_3dPoint& upper_left,
    ON_3dPoint& lower_left,
    ON_3dPoint& lower_right,
    ON_3dPoint& upper_right,
    ON__UINT32 row,
    ON__UINT32 col) const;

private:
  int Internal_CellIndex(ON__UINT32 row, ON__UINT32 col) const;
  void Internal_ResizeCells(ON__UINT32 old_rows, ON__UINT32 old_cols);
  int Internal_StyleIndexFromId(ON__INT32 id) const;
  ON__INT32 Internal_NextStyleId() const;
  bool Internal_IsValidStyleId(ON__INT32 id) const;
  void Internal_InitializeRows(ON__UINT32 row_count);
  void Internal_InitializeColumns(ON__UINT32 column_count);
  void Internal_ApplyBuiltInRowTypes();
  void Internal_ResetMergeState();
  bool Internal_MergeRegionsOverlap(int a, int b) const;

private:
  ON_TablePrivate* m_private_ON_Table;
};

class ON_CLASS ON_AnnotationTable : public ON_Annotation
{
  ON_OBJECT_DECLARE(ON_AnnotationTable);

public:
  ON_AnnotationTable();
  ON_AnnotationTable(const ON_AnnotationTable& src);
  virtual ~ON_AnnotationTable();
  ON_AnnotationTable& operator=(const ON_AnnotationTable& src);

  static const ON_AnnotationTable Empty;

  bool Create(
    ON__UINT32 row_count,
    ON__UINT32 column_count,
    bool has_title_row,
    bool has_header_row);

  void Destroy();

  bool IsValid(ON_TextLog* text_log = 0) const;
  void Dump(ON_TextLog& text_log) const;
  bool Write(ON_BinaryArchive& archive) const;
  bool Read(ON_BinaryArchive& archive);

  int Dimension() const;
  bool GetBBox(double* boxmin, double* boxmax, bool bGrowBox = false) const;

  bool GetAnnotationBoundingBox(
    const ON_Viewport* vp,
    const ON_DimStyle* dimstyle,
    double dimscale,
    double* boxmin,
    double* boxmax,
    bool bGrow = false) const;

  bool GetTextXform(
    const ON_Viewport* vp,
    const ON_DimStyle* dimstyle,
    double dimscale,
    ON_Xform& text_xform_out) const;

  const ON_Table* Table() const;
  ON_Table* TableToModify();
  bool SetTable(const ON_Table& table);

  ON__UINT32 RowCount() const;
  void SetRowCount(ON__UINT32 rowcount);

  ON__UINT32 ColCount() const;
  void SetColCount(ON__UINT32 colcount);

  ON_TableCell* GetCell(ON__UINT32 row, ON__UINT32 col);
  bool GetCell(ON__UINT32 row, ON__UINT32 col, ON_TableCell& cell) const;

  double TotalWidth() const;
  bool SetTotalWidth(double width);

  double TotalHeight() const;
  bool SetTotalHeight(double height);

  bool AppendRows(double height, ON__UINT32 num_rows = 1);

  const ON_TableCellStyle& CellStyle(ON__UINT32 row, ON__UINT32 col) const;
  const ON_TableCellStyle& CellStyle(ON__INT32 id) const;
  bool AppendCellStyle(const ON_TableCellStyle& style);
  bool ReplaceCellStyle(const ON_TableCellStyle& style);

  const ON_ArchivableDictionary& DwgFidelity() const;
  ON_ArchivableDictionary& DwgFidelityToModify();

  const ON_ArchivableDictionary& ExternalDataLink() const;
  ON_ArchivableDictionary& ExternalDataLinkToModify();

  const ON_ArchivableDictionary& AutoCADParityState() const;
  ON_ArchivableDictionary& AutoCADParityStateToModify();

  bool SetDwgStyleId(const ON_UUID& style_id);
  ON_UUID DwgStyleId() const;

private:
  void Internal_CopyFrom(const ON_AnnotationTable& src);

private:
  ON_AnnotationTablePrivate* m_private_ON_AnnotationTable;
};

#endif

#endif

