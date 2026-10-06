//
// Copyright (c) 1993-2026 Robert McNeel & Associates. All rights reserved.
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
//
//   Machinery for checking that the memory layout of an SDK class has not
//   changed. The expectations themselves live in two tables per library:
//
//     opennurbs_plus_class_layout_table.h   OpenNURBS classes
//     rhinoSdkClassLayoutTable.h            RhinoCore SDK classes
//
//   See RH-98462.
//
////////////////////////////////////////////////////////////////

#if !defined(OPENNURBS_PLUS) || !defined(OPENNURBS_PLUS_INC_) || defined(OPENNURBS_PUBLIC)
#error This file should not be distributed with the public opennurbs source code toolkit.
#endif

#if !defined(OPENNURBS_CLASS_LAYOUT_INC_)
#define OPENNURBS_CLASS_LAYOUT_INC_

////////////////////////////////////////////////////////////////
//
// Changing the size of a class, or the offset of one of its public members,
// breaks binary compatibility with every plug-in built against an earlier SDK.
//
// In C++20, the checks are compiled into the libraries, so they run at
// compile time during an ordinary OpenNURBS or RhinoCore build.
// In C++11, the test suites walk the same rows at runtime and print what went wrong.
//
// Reading a row
// -------------
//   ON_CLASS_LAYOUT_SIZE  (kind, class,         base, ON+, win, mac, ios, lin, android)
//   ON_CLASS_LAYOUT_OFFSET(      class, member, base, ON+, win, mac, ios, lin, android)
//
//   An offset row needs a member with an address of its own. A reference
//   member and a bit-field have none, the compilers disagree about what
//   offsetof means for them, so neither gets a row.
//
//   kind  class or struct, whichever the header uses. A size row forward-declares
//         the class so the row can ask whether it exists at all.
//
//   base  The expected value in the public flavor. Everything else is a delta
//         from it, so a class that lays out identically everywhere reads
//         "368, 0, 0, 0, 0, 0" and anything unusual stands out.
//
//   ON+   Bytes to add when OPENNURBS_PLUS is defined. It sits next to the
//         base it modifies; the platform columns follow.
//
//   windows
//   mac
//   ios
//   linux
//   android
//         Bytes to add to base on that platform. Maybe because of padding,
//         different alignment, sizeof(long), friend classes, etc.
//
//   ON_NA Not in this configuration. Any column may read ON_NA instead
//         of a number, which means this row is not in this config.
//
// A row is checked in both directions:
//   builds need to have a row with a size defined,
//   builds need to not have a row where that says ON_NA.
//
// When a check fails, it is likely because you are attempting to break the SDK layout.
//   If you are sure the change is safe, update the row accordingly.
//
// There are two ways the tests can run:
//   ON_CLASS_LAYOUT_COMPILE_TIME_CHECKS
//         C++20. Every row is a static_assert, so a layout change fails the build.
//   ON_CLASS_LAYOUT_RUNTIME_CHECKS
//         C++11. The same rows, built into an array for a test suite to walk and report at run time
//
// Below C++11 neither is available, and neither is there on a platform the
// tables have no column for, so there the tables compile to nothing.
//
////////////////////////////////////////////////////////////////

#if defined(ON_COMPILING_OPENNURBS) || defined(OPENNURBS_INC_) || defined(OPENNURBS_PUBLIC_INC_)
// opennurbs is available
#else
#error Include opennurbs.h or opennurbs_public.h before opennurbs_plus_class_layout.h.
#endif

#include <stddef.h>
#include <string>

#if defined(_MSVC_LANG)
#define ON_CLASS_LAYOUT_LANG _MSVC_LANG
#else
#define ON_CLASS_LAYOUT_LANG __cplusplus
#endif

// Every number in the tables was measured on a 64 bit build of one of three
// platforms. Anywhere else - 32 bit WebAssembly, say - there is nothing to
// compare against, so the tables compile to nothing instead of failing the
// build. Measure that platform and add a column to cover it.
#if (defined(ON_RUNTIME_WIN) || defined(ON_RUNTIME_APPLE) \
     || defined(ON_RUNTIME_LINUX)) && defined(ON_64BIT_RUNTIME)
#define ON_CLASS_LAYOUT_PLATFORM 1
#else
#define ON_CLASS_LAYOUT_PLATFORM 0
#endif

#if ON_CLASS_LAYOUT_LANG >= 202002L && ON_CLASS_LAYOUT_PLATFORM
#define ON_CLASS_LAYOUT_COMPILE_TIME_CHECKS 1
#else
#define ON_CLASS_LAYOUT_COMPILE_TIME_CHECKS 0
#endif

#if ON_CLASS_LAYOUT_LANG >= 201103L && ON_CLASS_LAYOUT_PLATFORM
#define ON_CLASS_LAYOUT_RUNTIME_CHECKS 1
#else
#define ON_CLASS_LAYOUT_RUNTIME_CHECKS 0
#endif

#if ON_CLASS_LAYOUT_COMPILE_TIME_CHECKS
#if !ON_CLASS_LAYOUT_RUNTIME_CHECKS
#error "ON Class layout compile-time checks cannot run if runtime checks are not built."
#endif
#endif

// Is this the build the table describes
// -------------------------------------
// A table describes a library, in the configuration that library is built in,
// and the build that compiles a class is the one that can measure it. Other
// translation units reach these headers, and they are not that build: on Linux
// the test suite's rhtests.h hand-rolls its own BOOL, HDC and RECT and never
// defines RHINO_SDK_MFC, so a third of the SDK is not declared there at all.
//
// So whether a class is here at all is only asked where the answer is the
// library's. A class a test build cannot see says nothing about the library's
// layout, and a row for it says nothing there. A value that build does measure
// is still checked, wherever it is measured.
//
// ON_COMPILING_OPENNURBS is the OpenNURBS library's own define. RHINO_SDK_MFC
// comes from stdafx.h, stdMac.h and stdLinux.h, the three prefix headers a
// RhinoCore translation unit starts from, and from nothing that reaches the
// SDK another way.
#if defined(ON_COMPILING_OPENNURBS) || defined(RHINO_SDK_MFC)
#define ON_CLASS_LAYOUT_LIBRARY 1
#else
#define ON_CLASS_LAYOUT_LIBRARY 0
#endif

// Comparing names needs a loop, so it can only be constexpr from C++14. The
// consteval table checks call it, and they only exist at C++20.
#if ON_CLASS_LAYOUT_LANG >= 201402L
#define ON_CLASS_LAYOUT_CONSTEXPR14 constexpr
#else
#define ON_CLASS_LAYOUT_CONSTEXPR14 inline
#endif

#if ON_CLASS_LAYOUT_RUNTIME_CHECKS

namespace ON_ClassLayout
{
  // A column that reads ON_NA: the class or member is not in that configuration.
  // Both sentinels are negative and every column of a row is ptrdiff_t, so a
  // row can hold either one without a narrowing conversion.
  constexpr ptrdiff_t NotAvailable = -2;

  // What a measurement reports when the class or member is not in this build.
  constexpr ptrdiff_t NotFound = -1;

  // Which column a row is read from, for the report.
  inline const char* ThisPlatformName()
  {
#if defined(ON_RUNTIME_APPLE_IOS)
    return "ios";
#elif defined(ON_RUNTIME_APPLE)
    return "mac";
#elif defined(ON_RUNTIME_ANDROID)
    return "android";
#elif defined(ON_RUNTIME_LINUX)
    return "linux";
#else
    return "windows";
#endif
  }

  inline const char* ThisFlavorName()
  {
#if defined(OPENNURBS_PLUS)
    return "ON+";
#else
    return "ON_public";
#endif
  }

  // One row, as data, so the rows that feed the static_asserts are the rows the
  // test suites walk. A row is written once and expanded into both.
  class Row
  {
  public:
    const char* m_class;
    const char* m_member;   // nullptr on a class size row
    ptrdiff_t m_actual;  // sizeof / offsetof here, or NotFound
    ptrdiff_t m_base;
    ptrdiff_t m_plus;
    ptrdiff_t m_windows;
    ptrdiff_t m_macos;
    ptrdiff_t m_ios;
    ptrdiff_t m_linux;
    ptrdiff_t m_android;

    constexpr bool IsSizeRow() const { return nullptr == m_member; }

    constexpr bool Found() const { return NotFound != m_actual; }

    // The delta column this build reads. A row reads its own columns: a
    // parameter for the Linux one cannot be called linux, because GCC defines
    // linux as 1 in the GNU dialect it compiles Rhino with.
    //
    // iOS is asked about before macOS, and Android before Linux, because each
    // of those builds defines both.
    constexpr ptrdiff_t PlatformColumn() const
    {
#if defined(ON_RUNTIME_APPLE_IOS)
      return m_ios;
#elif defined(ON_RUNTIME_APPLE)
      return m_macos;
#elif defined(ON_RUNTIME_ANDROID)
      return m_android;
#elif defined(ON_RUNTIME_LINUX)
      return m_linux;
#else
      return m_windows;
#endif
    }

    constexpr bool BelongsHere() const
    {
      return NotAvailable != PlatformColumn()
#if defined(OPENNURBS_PLUS)
        && NotAvailable != m_plus;
#else
        && NotAvailable != m_base;
#endif
    }

    constexpr ptrdiff_t Expected() const
    {
      return ((NotAvailable == m_base) ? 0 : m_base)
        + PlatformColumn()
#if defined(OPENNURBS_PLUS)
        + m_plus;
#else
        ;
#endif
    }

    // The three ways a row and a build can disagree, kept apart so each one
    // gets its own message.
    constexpr bool Missing() const { return BelongsHere() && !Found(); }
    constexpr bool Unexpected() const { return !BelongsHere() && Found(); }
    constexpr bool WrongValue() const
    {
      return BelongsHere() && Found() && m_actual != Expected();
    }
    constexpr bool Ok() const
    {
      return !Missing() && !Unexpected() && !WrongValue();
    }

    // What this build can be held to: everything, where it is the library the
    // table describes, and the values it measures where it is not.
    constexpr bool CheckedHere() const
    {
#if ON_CLASS_LAYOUT_LIBRARY
      return Ok();
#else
      return !WrongValue();
#endif
    }

    std::string Describe() const
    {
      std::string text(IsSizeRow() ? "sizeof(" : "offsetof(");
      text += m_class;
      if (!IsSizeRow())
      {
        text += ", ";
        text += m_member;
      }
      text += ")";
      return text;
    }

    std::string ReportText() const
    {
      if (Missing())
        return Describe() + " is missing from this build, but previous SDK versions expect it.";

      if (Unexpected())
        return Describe() + " exists in this build, but previous SDK versions only expect it in a different configuration.";

      if (WrongValue())
        return Describe() + " is " + std::to_string(m_actual) + ", but " + std::to_string(Expected()) + " was expected.";

      return Describe() + " matches the class layout table.";
    }
  };

  class Table
  {
  public:
    const char* m_name;
    const Row* m_rows;
    size_t m_count;

    template <size_t N>
    constexpr Table(const char* name, const Row (&rows)[N])
      : m_name(name), m_rows(rows), m_count(N) {}
  };

  constexpr char Lower(char c)
  {
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
  }

  // Exact comparison, one return statement, so it is constexpr from C++11 on.
  constexpr bool SameSpelling(const char* a, const char* b)
  {
    return (*a == *b) && (0 == *a || SameSpelling(a + 1, b + 1));
  }

  // Case-insensitive, so the table reads in the order a person would put it in,
  // and case-sensitive only to break a tie.
  ON_CLASS_LAYOUT_CONSTEXPR14 int CompareNames(const char* a, const char* b)
  {
    if (nullptr == a)
      return (nullptr == b) ? 0 : -1;
    if (nullptr == b)
      return 1;

    const char* p = a;
    const char* q = b;
    while (*p && Lower(*p) == Lower(*q)) { p++; q++; }
    const int fold = (int)(unsigned char)Lower(*p) - (int)(unsigned char)Lower(*q);
    if (0 != fold)
      return fold;

    while (*a && *a == *b) { a++; b++; }
    return (int)(unsigned char)(*a) - (int)(unsigned char)(*b);
  }

  ON_CLASS_LAYOUT_CONSTEXPR14 int CompareRows(const Row& a, const Row& b)
  {
    const int order = CompareNames(a.m_class, b.m_class);
    return (0 != order) ? order : CompareNames(a.m_member, b.m_member);
  }


  // The run-time half, used by the test suites. Where the static_asserts are
  // available they already guarantee the first of these passes, and what is
  // left is the report.
  template <class ReportFn>
  size_t CheckTable(const Table& table, ReportFn report)
  {
    size_t failures = 0;
    for (size_t i = 0; i < table.m_count; i++)
    {
      if (table.m_rows[i].CheckedHere())
        continue;
      report(table.m_rows[i]);
      failures++;
    }
    return failures;
  }

  // Rows are kept in alphabetical order of class name, then member name.
  // This way checking for a repeated row only has to check adjacent rows.
  template <class ReportFn>
  size_t CheckTableOrder(const Table& table, ReportFn report)
  {
    size_t problems = 0;
    for (size_t i = 1; i < table.m_count; i++)
    {
      if (CompareRows(table.m_rows[i - 1], table.m_rows[i]) < 0)
        continue;
      report(table.m_rows[i]);
      problems++;
    }
    return problems;
  }

  // Rows the table says do not belong in this configuration. They are still
  // checked - for absence - but they carry no size or offset here.
  inline size_t NotAvailableHereCount(const Table& table)
  {
    size_t count = 0;
    for (size_t i = 0; i < table.m_count; i++)
    {
      if (!table.m_rows[i].BelongsHere())
        count++;
    }
    return count;
  }

  // To measure a class that may not be here, use SFINAE to check that we can't
  // substitute the type with ON_NA as a template argument.
  template <class... T> struct Void { typedef void type; };
  class NotInThisBuild {};

  template <class T, class = void> struct Defined
  {
    typedef NotInThisBuild type;
    static constexpr ptrdiff_t Size = NotFound;
  };

  template <class T> struct Defined<T, typename Void<decltype(sizeof(T))>::type>
  {
    typedef T type;
    static constexpr ptrdiff_t Size = (ptrdiff_t)sizeof(T);
  };

  // The class a row names, or NotInThisBuild when this build only declares it.
  template <class T> using DefinedType = typename Defined<T>::type;
}

#define ON_NA ON_ClassLayout::NotAvailable

#endif  // ON_CLASS_LAYOUT_RUNTIME_CHECKS


#if ON_CLASS_LAYOUT_COMPILE_TIME_CHECKS

namespace ON_ClassLayout
{
  template <size_t N>
  consteval bool AllRowsOk(const Row (&rows)[N])
  {
    for (size_t i = 0; i < N; i++)
    {
      if (!rows[i].CheckedHere())
        return false;
    }
    return true;
  }

  template <size_t N>
  consteval bool RowsAreSortedAndUnique(const Row (&rows)[N])
  {
    for (size_t i = 1; i < N; i++)
    {
      if (CompareRows(rows[i - 1], rows[i]) >= 0)
        return false;
    }
    return true;
  }

  // These exist so that the numbers show up in the diagnostic. A static_assert
  // cannot print a value, but a compiler always names the template arguments of
  // the specialization it was instantiating.
  template<class cls, ptrdiff_t actual, ptrdiff_t expected> struct CheckSizeOf {
    static_assert(actual == expected,
      "Sizeof does not match the class layout table. "
      "The template arguments in the note above show "
      "CheckSizeOf<classname, actual_size, expected_size>.");
    static constexpr bool _cResult = (actual == expected);
  };

  template<class cls__member, ptrdiff_t actual, ptrdiff_t expected> struct CheckOffsetOf {
    static_assert(actual == expected,
      "Offsetof does not match the class layout table. "
      "The template arguments in the note above show "
      "CheckOffsetOf<class__member, actual_offset, expected_offset>.");
    static constexpr bool _cResult = (actual == expected);
  };

  // A row that says ON_NA where the build has the class after all
  template<class cls, ptrdiff_t measured> struct CheckAbsent {
    static_assert(NotFound == measured,
      "The class layout table says this is not part of this configuration. "
      "The template arguments in the note above show "
      "CheckAbsent<classname, the_value_this_build_measures>.");
    static constexpr bool _cResult = (NotFound == measured);
  };
}

#endif

////////////////////////////////////////////////////////////////
//
// Row expansions. The following macros and templates expand the size tables into:
//   - one named Row per entry
//   - assertions about each row (if the compiler supports it)
//   - an array for the test suites
// Both expansions read the same Row objects, so they cannot disagree.
//

// Many shipped classes are not standard-layout, and offsetof on one of those is
// conditionally supported rather than undefined. It is what the SDK's callers
// rely on, and what the pre-existing tests have always measured. A member can
// also be ON_DEPRECATED_MSG: measuring it is not using it.
#if defined(__clang__) || defined(__GNUC__)
#define ON_CLASS_LAYOUT_OFFSETOF_WARNING_PUSH                    \
  _Pragma("GCC diagnostic push")                                 \
  _Pragma("GCC diagnostic ignored \"-Winvalid-offsetof\"")       \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define ON_CLASS_LAYOUT_OFFSETOF_WARNING_POP \
  _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define ON_CLASS_LAYOUT_OFFSETOF_WARNING_PUSH \
  __pragma(warning(push))                     \
  __pragma(warning(disable : 4996))
#define ON_CLASS_LAYOUT_OFFSETOF_WARNING_POP \
  __pragma(warning(pop))
#else
#define ON_CLASS_LAYOUT_OFFSETOF_WARNING_PUSH
#define ON_CLASS_LAYOUT_OFFSETOF_WARNING_POP
#endif

// The offset of one member, or NotFound when this build has no such public member.
#define ON_CLASS_LAYOUT_OFFSET_MEASURE(detector, member)                         \
  namespace ON_ClassLayout { namespace Measured {                                \
    template <class T, class = void> struct detector                             \
    { static constexpr ptrdiff_t Value = NotFound; };                            \
    template <class T> struct detector<T,                                        \
      typename Void<decltype(&T::member)>::type>                                 \
    { static constexpr ptrdiff_t Value = (ptrdiff_t)offsetof(T, member); };      \
  } }

#if ON_CLASS_LAYOUT_COMPILE_TIME_CHECKS

// The three ways a row and a build can disagree, kept apart so that each one
// gets its own message naming the class, or the class and the member.
#if ON_CLASS_LAYOUT_LIBRARY
#define ON_CLASS_LAYOUT_SIZE_PRESENCE(cls, row, name)                            \
  static_assert(!ON_ClassLayout::Measured::row.Missing(),                        \
    name " is missing from this build, but previous SDK versions expect it.");   \
  static_assert(ON_ClassLayout::CheckAbsent<cls,                                 \
      ON_ClassLayout::Measured::row.Unexpected()                                 \
        ? ON_ClassLayout::Measured::row.m_actual : ON_ClassLayout::NotFound      \
    >::_cResult,                                                                 \
    name " exists in this build, but previous SDK versions only expect it in a different configuration.");
#define ON_CLASS_LAYOUT_OFFSET_PRESENCE(tag, row, name)                          \
  static_assert(!ON_ClassLayout::Measured::row.Missing(),                        \
    name " is missing, no longer public, or has no address of its own, but previous SDK versions expect it."); \
  static_assert(ON_ClassLayout::CheckAbsent<tag,                                 \
      ON_ClassLayout::Measured::row.Unexpected()                                 \
        ? ON_ClassLayout::Measured::row.m_actual : ON_ClassLayout::NotFound      \
    >::_cResult,                                                                 \
    name " is public in this build, but previous SDK versions only expect it in a different configuration.");
#else
#define ON_CLASS_LAYOUT_SIZE_PRESENCE(cls, row, name)
#define ON_CLASS_LAYOUT_OFFSET_PRESENCE(tag, row, name)
#endif

#define ON_CLASS_LAYOUT_SIZE_ASSERTS(cls, row, name)                             \
  ON_CLASS_LAYOUT_SIZE_PRESENCE(cls, row, name)                                  \
  static_assert(ON_ClassLayout::CheckSizeOf<cls,                                 \
      ON_ClassLayout::Measured::row.WrongValue()                          \
        ? ON_ClassLayout::Measured::row.m_actual : (ptrdiff_t)0,          \
      ON_ClassLayout::Measured::row.WrongValue()                          \
        ? ON_ClassLayout::Measured::row.Expected() : (ptrdiff_t)0         \
    >::_cResult,                                                                 \
    "sizeof(" name ") is not what the class layout table says. See opennurbs_plus_class_layout.h.");

#define ON_CLASS_LAYOUT_OFFSET_ASSERTS(cls, row, tag, name)                              \
  namespace ON_ClassLayout { namespace Measured {                                \
    namespace WrongOffset { struct tag; } } }                        \
  ON_CLASS_LAYOUT_OFFSET_PRESENCE(ON_ClassLayout::Measured::WrongOffset::tag, row, name) \
  static_assert(ON_ClassLayout::CheckOffsetOf<ON_ClassLayout::Measured::WrongOffset::tag, \
      ON_ClassLayout::Measured::row.WrongValue()             \
        ? ON_ClassLayout::Measured::row.m_actual : (ptrdiff_t)0, \
      ON_ClassLayout::Measured::row.WrongValue()             \
        ? ON_ClassLayout::Measured::row.Expected() : (ptrdiff_t)0 \
    >::_cResult,                                                                 \
    "offsetof(" name ") is not what the class layout table says. See opennurbs_plus_class_layout.h.");

#else

// Pre-C++20: don't build the static_asserts from the tables
#define ON_CLASS_LAYOUT_SIZE_ASSERTS(cls, row, name)
#define ON_CLASS_LAYOUT_OFFSET_ASSERTS(cls, row, tag, name)

#endif

#if ON_CLASS_LAYOUT_RUNTIME_CHECKS

// A class name can be a macro. RhinoSdkDialog.h has
//   #define CRhinoUiDialog CDialog
// for a translation unit without RHINO_SDK_MFC, and rhtests.h on Windows is
// one of those that still has MFC, so there the row would measure CDialog.
#define ON_CLASS_LAYOUT_SPELLING(cls) #cls
#if ON_CLASS_LAYOUT_LIBRARY
#define ON_CLASS_LAYOUT_NAMED_HERE(written, cls) true
#else
#define ON_CLASS_LAYOUT_NAMED_HERE(written, cls)                                 \
  ON_ClassLayout::SameSpelling(written, ON_CLASS_LAYOUT_SPELLING(cls))
#endif

// A size row forward-declares the class, so that a row may name a class this
// configuration does not have and the detector can say so.
#define ON_CLASS_LAYOUT_SIZE(kind, cls, base, plus, windows, macos, ios, linux, android) \
  kind cls;                                                                      \
  namespace ON_ClassLayout { namespace Measured {                                \
    constexpr Row cls##_size{ #cls, nullptr,                                     \
      ON_CLASS_LAYOUT_NAMED_HERE(#cls, cls) ? Defined<cls>::Size : NotFound,     \
      base, plus, windows, macos, ios, linux, android }; } }                     \
  ON_CLASS_LAYOUT_SIZE_ASSERTS(cls, cls##_size, #cls)

#define ON_CLASS_LAYOUT_OFFSET(cls, member, base, plus, windows, macos, ios, linux, android) \
  ON_CLASS_LAYOUT_OFFSETOF_WARNING_PUSH                                          \
  ON_CLASS_LAYOUT_OFFSET_MEASURE(Offset_##cls##_##member, member)                \
  namespace ON_ClassLayout { namespace Measured {                                \
    constexpr Row cls##_##member##_offset{ #cls, #member,                        \
      ON_CLASS_LAYOUT_NAMED_HERE(#cls, cls)                                      \
        ? Offset_##cls##_##member< DefinedType<cls> >::Value : NotFound,         \
      base, plus, windows, macos, ios, linux, android }; } }                     \
  ON_CLASS_LAYOUT_OFFSETOF_WARNING_POP                                           \
  ON_CLASS_LAYOUT_OFFSET_ASSERTS(cls, cls##_##member##_offset,                   \
                                 cls##__##member, #cls "::" #member)

#define ON_CLASS_LAYOUT_ROW_SIZE(kind, cls, base, plus, windows, macos, ios, linux, android) \
  ON_ClassLayout::Measured::cls##_size,

#define ON_CLASS_LAYOUT_ROW_OFFSET(cls, member, base, plus, windows, macos, ios, linux, android) \
  ON_ClassLayout::Measured::cls##_##member##_offset,

#else

// Older than C++11: the tables compile to nothing, so 3rd parties on old compilers can still build.
#define ON_CLASS_LAYOUT_SIZE(kind, cls, base, plus, windows, macos, ios, linux, android)
#define ON_CLASS_LAYOUT_OFFSET(cls, member, base, plus, windows, macos, ios, linux, android)
#define ON_CLASS_LAYOUT_ROW_SIZE(kind, cls, base, plus, windows, macos, ios, linux, android)
#define ON_CLASS_LAYOUT_ROW_OFFSET(cls, member, base, plus, windows, macos, ios, linux, android)

#endif

#endif
