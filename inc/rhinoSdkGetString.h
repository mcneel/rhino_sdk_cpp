//
// Copyright (c) 1993-2017 Robert McNeel & Associates. All rights reserved.
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

#include "rhinoSdkGet.h"

class RHINO_SDK_CLASS CRhinoGetString : public CRhinoGet
{
public:
  CRhinoGetString();
  ~CRhinoGetString();
  CRhinoGetString(const CRhinoGet&); // prompt and options inherited
  CRhinoGetString& operator=(const CRhinoGetString&); // prompt and options inherited

  //////////////////////////////////////////////////////////////////
  //
  // STEP 1: Use CRhinoGet member functions to specify command prompt
  //         and command options.
  //

  //////////////////////////////////////////////////////////////////
  //
  // STEP 2: Use CRhinoGet member functions to specify what types of
  //         alternate input can be accepted.
  //

  //////////////////////////////////////////////////////////////////
  //
  // STEP 3: Call GetString() or GetLiteralString() to get a string.
  //         GetString() is the standard and you should almost always
  //         use this.  GetLiteralString() is used to get the entire
  //         line (up to an Enter key press or a new-line in a script
  //         file.).  For example, GetLiteralString() is used to get
  //         input for lisp, python, etc. scripts. 
  //
  //
  CRhinoGet::result GetString();

  CRhinoGet::result GetLiteralString();

  //////////////////////////////////////////////////////////////////
  //
  // STEP 4: Use result code returned by GetString() to determine what
  //         CRhinoGet function to call to get the input.
  //
  //         result               function
  //           CRhinoGet::cancel    user pressed ESCAPE
  //           CRhinoGet::nothing   user pressed ENTER
  //           CRhinoGet::undo      user wants to UNDO
  //           CRhinoGet::option    CRhinoGet::Option()
  //           CRhinoGet::number    CRhinoGet::Number()
  //           CRhinoGet::string    CRhinoGet::String()
  //
protected:
  // used by CRhinoGetColor, CRhinoGetNumber, and CRhinoGetOption
  CRhinoGetString(TYPE);
  CRhinoGetString(TYPE,const CRhinoGet&);
};

class RHINO_SDK_CLASS CRhinoGetOption : public CRhinoGetString
{
public:
  CRhinoGetOption();
  ~CRhinoGetOption();
  CRhinoGetOption(const CRhinoGet&);                  // prompt and options inherited
  CRhinoGetOption& operator=(const CRhinoGetOption&); // prompt and options inherited

  //////////////////////////////////////////////////////////////////
  //
  // STEP 1: Use CRhinoGet member functions to specify command prompt
  //         and command options.
  //

  //////////////////////////////////////////////////////////////////
  //
  // STEP 2: Use CRhinoGet member functions to specify what types of
  //         alternate input can be accepted.
  //

  //////////////////////////////////////////////////////////////////
  //
  // STEP 3: Call GetOption() to get a option.
  //
  // A return value of CRhinoGet::option means the user selected a
  // valid option.  Use CRhinoGet::Option() to determine which option
  // was selected.
  CRhinoGet::result GetOption();

  //////////////////////////////////////////////////////////////////
  //
  // STEP 4: Use result code returned by GetOption() to determine what
  //         CRhinoGet function to call to get the input.
  //
  //         result               function
  //           CRhinoGet::cancel    user pressed ESCAPE
  //           CRhinoGet::nothing   user pressed ENTER
  //           CRhinoGet::undo      user wants to UNDO
  //           CRhinoGet::option    CRhinoGet::Option()
  //           CRhinoGet::number    CRhinoGet::Number()
  //           CRhinoGet::string    CRhinoGet::String()
  //

private:
  // If you want to explicitly get string input, then 
  // use CRhinoGetString with options.
  // If you want to only get options, then use CRhinoGetOption,
  CRhinoGet::result GetString();
};


//////////////////////////////////////////////////////////////////////////
//
// CRhinoGetNumber is used to get floating point numbers
//
class RHINO_SDK_CLASS CRhinoGetNumber : public CRhinoGetString
{
public:
  CRhinoGetNumber();
  ~CRhinoGetNumber();
  CRhinoGetNumber(const CRhinoGet&);                  // prompt and options inherited
  CRhinoGetNumber(const CRhinoGetNumber&);
  CRhinoGetNumber& operator=(const CRhinoGetNumber&);


  //////////////////////////////////////////////////////////////////
  //
  // STEP 1: Use CRhinoGet member functions to specify command prompt
  //

  //////////////////////////////////////////////////////////////////
  //
  // STEP 2: Use CRhinoGet member functions to specify what types of
  //         alternate input can be accepted.
  //

  /*
  Description:
    Set a lower limit on the number that can be returned.  
    By default there is no lower limit.
  Parameters:
    lower_limit - [in] smallest acceptable number.  Pass ON_UNSET_VALUE
        if you want to disable a previously set lower_limit.
    bStrictlyGreaterThan - [in] (default=FALSE) If TRUE, then
        the returned number will be > lower_limit.
  See Also:
    CRhinoGetNumber::SetUpperLimit
  */
  void SetLowerLimit( 
    double lower_limit,
    BOOL32 bStrictlyGreaterThan = FALSE
    );


  /*
  Description:
    Set a upper limit on the number that can be returned.
    By default there is no upper limit.
  Parameters:
    upper_limit - [in] largest acceptable number.  Pass ON_UNSET_VALUE
        if you want to disable a previously set upper_limit.
    bStrictlyLessThan - [in] (default=FALSE) If TRUE, then
        the returned number will be < upper_limit.
  See Also:
    CRhinoGetNumber::SetLowerLimit
  */
  void SetUpperLimit( 
    double upper_limit,
    BOOL32 bStrictlyLessThan = FALSE
    );

  //////////////////////////////////////////////////////////////////
  //
  // STEP 4: Call GetNumber() to get an integer
  //
  //
  CRhinoGet::result GetNumber();

  //////////////////////////////////////////////////////////////////
  //
  // STEP 5: Use result code returned by GetNumber() to determine what
  //         CRhinoGet function to call to get the input.
  //
  //         result               function
  //           CRhinoGet::cancel    user pressed ESCAPE
  //           CRhinoGet::nothing   user pressed ENTER
  //           CRhinoGet::undo      user wants to UNDO
  //           CRhinoGet::option    CRhinoGet::Option()
  //           CRhinoGet::number    CRhinoGet::Number()
  //           CRhinoGet::string    CRhinoGet::String()
  //

protected:
  int m_lower_limit_flag; // 0 = none, 1 = m_lower_limit <= result, 2 = m_lower_limit <= result;
  int m_upper_limit_flag; // 0 = none, 1 = result <= m_upper_limit, 2 = result < m_upper_limit
  double m_lower_limit; // default is ON_UNSET_VALUE
  double m_upper_limit; // default is ON_UNSET_VALUE

private:
  // If you want to get a string, then use CRhinoGetString.
  // If you want to get a number, then use CRhinoGetNumber.
  CRhinoGet::result GetString();
};


//////////////////////////////////////////////////////////////////////////
//
// CRhinoGetInteger is used to get integer numbers
//
class RHINO_SDK_CLASS CRhinoGetInteger : public CRhinoGetNumber
{
public:
  CRhinoGetInteger();
  ~CRhinoGetInteger();
  CRhinoGetInteger(const CRhinoGet&);                  // prompt and options inherited
  CRhinoGetInteger& operator=(const CRhinoGetInteger&); // prompt and options inherited


  //////////////////////////////////////////////////////////////////
  //
  // STEP 1: Use CRhinoGet member functions to specify command prompt
  //

  //////////////////////////////////////////////////////////////////
  //
  // STEP 2: Use CRhinoGet member functions to specify what types of
  //         alternate input can be accepted.
  //

  //////////////////////////////////////////////////////////////////
  //
  // STEP 3: Call GetNumber() to get an integer
  //
  //
  CRhinoGet::result GetInteger();

  //////////////////////////////////////////////////////////////////
  //
  // STEP 5: Use result code returned by GetString() to determine what
  //         CRhinoGet function to call to get the input.
  //
  //         result               function
  //           CRhinoGet::cancel    user pressed ESCAPE
  //           CRhinoGet::nothing   user pressed ENTER
  //           CRhinoGet::undo      user wants to UNDO
  //           CRhinoGet::option    CRhinoGet::Option()
  //           CRhinoGet::number    CRhinoGet::Number()
  //           CRhinoGet::string    CRhinoGet::String()
  //
  int Number();
};

class RHINO_SDK_CLASS CRhinoGetColor : public CRhinoGetString
{
public:
  CRhinoGetColor();
  ~CRhinoGetColor();
  CRhinoGetColor(const CRhinoGet&);
  CRhinoGetColor& operator=(const CRhinoGetColor&);

  CRhinoGet::result GetColor();
  CRhinoGet::result GetColor(bool bUseAlpha);

private:
  // If you want to get a string, then use CRhinoGetString.
  // If you want to get a color, then use CRhinoGetColor.
  CRhinoGet::result GetString();
};

class RHINO_SDK_CLASS CRhinoRebuildCurveOptions
{
public:
  CRhinoRebuildCurveOptions() = default;
  ~CRhinoRebuildCurveOptions() = default;
  CRhinoRebuildCurveOptions(const CRhinoRebuildCurveOptions&) = default;
  CRhinoRebuildCurveOptions& operator=(const CRhinoRebuildCurveOptions&) = default;


public:
  static const CRhinoRebuildCurveOptions Defaults;

public:
  // Interface to use to get and update options used in curve rebuilding.

  /// <summary>
  /// Specify the context where the rebuild calculation is occuring. This is done
  /// one time by the code or command performing that is managing the calculation.
  /// User interface code should never call this function.
  /// </summary>
  /// <param name="default_options">
  /// These are the default values to begin and are typically the values from the
  /// most recent calculation in the same context. If you don't have context specific
  /// defaults, then pass CRhinoRebuildCurveOptions::Defaults;
  /// </param>
  /// <param name="rhino_doc_sn">
  /// If the rebuild calculation is running in a Rhino command,
  /// pass the serial number of the Rhino document.
  /// </param>
  /// <param name="context_distance_tolerance">
  /// 0 &lt; context_distance_tolerance
  /// </param>
  /// <param name="context_distance_tolerance">
  /// 0 &lt; context_distance_tolerance
  /// </param>
  /// <param name="input_curve_count">
  /// If existing curves are being rebuilt, 
  /// then pass the number of input curves.
  /// Otherwise pass 0.
  /// </param>
  /// <param name="input_degree_range">
  /// If existing curves are being rebuilt, 
  /// then pass (minimum input degree, maximum input degree).
  /// Otherwise, pass(0,0).
  /// </param>
  /// <param name="input_point_count_range">
  /// If existing curves are being rebuilt, 
  /// then pass (minimum input point count, maximum input point count).
  /// Otherwise, pass(0,0).
  /// </param>
  /// <param name="input_span_count_range">
  /// If existing curves are being rebuilt, 
  /// then pass (minimum input span count, maximum input span count).
  /// Otherwise, pass(0,0).
  /// </param>
  /// <param name="context_angle_tolerance_radians">
  /// 0 &lt;= context_angle_tolerance_radians &lt;= pi.
  /// </param>
  /// <param name="bOpenTargetCurves">
  /// Pass true if at least one of the target curves is open (different start and end points).
  /// </param>
  /// <param name="bClosedG0TargetCurves">
  /// Pass true if at least one of the target curves is closed (equal start and end points)
  /// and has distinct tangents at the start and end.
  /// </param>
  /// <param name="bClosedG1TargetCurves">
  /// Pass true if at least one of the target curves is closed (equal start and end points)
  /// and has equal tangents at the start and end.
  /// </param>
  /// <param name="bTangentMatchingPermitted">
  /// Pass true if the user interface should offer start/end tangent matching options 
  /// when they are appropriate.
  /// </param>
  /// <param name="bKinkSplittingPermitted">
  /// Pass true if at least one of the target curves has a kink of some sort
  /// and the user interface is permitted to offer kink splitting options.
  /// </param>
  /// <param name="bPointCountRangesPermitted">
  /// Pass true if the user interface is permitted to offer a range of point counts
  /// and the rebuild will automatically choose the one with the smalles deviation.
  /// </param>
  void SetContext(
    const CRhinoRebuildCurveOptions& default_options,
    unsigned int rhino_doc_sn,
    double context_distance_tolerance,
    double context_angle_tolerance_radians,
    int input_curve_count,
    ON_2dex input_degree_range,
    ON_2dex input_point_count_range,
    ON_2dex input_span_count_range,
    bool bOpenTargetCurves,
    bool bClosedG0TargetCurves,
    bool bClosedG1TargetCurves,
    bool bTangentMatchingPermitted,
    bool bKinkSplittingPermitted,
    bool bPointCountRangesPermitted
  );

  /// <returns>
  /// When the rebuild is happening in a Rhino command, this is
  /// the serial number of the Rhino document where the command
  /// is running.
  /// </returns>
  unsigned RhinoDocumentSerialNumber() const;

  /// <summary>
  /// When input curves are being rebuilt, InputCurveCount()
  /// reports the number of input curves.
  /// </summary>
  /// <returns>
  /// Number of curves being rebuilt or 0 if there are no
  /// input curves in the current context.
  /// </returns>
  int InputCurveCount() const;

  /// <summary>
  /// When input curves are being rebuilt, InputDegreeRange()
  /// reports the range of input curve degrees. 
  /// </summary>
  /// <returns>
  /// If input curves are being rebuilt, then
  /// InputDegreeRange() = (minimum input degree, maximum input degree).
  /// Otherwise InputDegreeRange() = (0,0).
  /// </returns>
  const ON_2dex InputDegreeRange() const;

  /// <summary>
  /// When input curves are being rebuilt, InputPointCountRange()
  /// reports the range of input curve point counts. 
  /// </summary>
  /// <returns>
  /// If input curves are being rebuilt, then
  /// InputPointCountRange() = (minimum input point count, maximum input point count).
  /// Otherwise InputPointCountRange() = (0,0).
  /// </returns>
  const ON_2dex InputPointCountRange() const;

  /// <summary>
  /// When input curves are being rebuilt, InputSpanCountRange()
  /// reports the range of input curve span counts. 
  /// </summary>
  /// <returns>
  /// If input curves are being rebuilt, then
  /// InputSpanCountRange() = (minimum input span count, maximum input span count).
  /// Otherwise InputSpanCountRange() = (0,0).
  /// </returns>
  const ON_2dex InputSpanCountRange() const;

  const ON_wString InputSummary() const;

  /// <summary>
  /// If the output rebuilt curves have been calculated, the
  /// code performing the calculation can call SetCurrentOutputProperties()
  /// to update the values of 
  /// CurrentOutputSpanCount(),
  /// CurrentOutputSpanCount(),
  /// and CurrentOutputMaximumDeviation().
  /// User interface code that is getting and setting values, 
  /// should never call this function.
  /// </summary>
  /// <param name="current_output_point_count_range"></param>
  /// <param name="current_output_span_count_range"></param>
  /// <param name="current_output_maximum_deviation"></param>
  void SetCurrentOutputProperties(
    ON_2dex current_output_point_count_range,
    ON_2dex current_output_span_count_range,
    double current_output_maximum_deviation
  ) const;

  void ClearCurrentOutputProperties() const;

  const ON_wString CurrentOutputSummary() const;

  /// <returns>
  /// If the input has been used to calculate rebuilt curves, 
  /// this is the range of point counts in the rebuilt curves.
  /// Otherwise, ON_2dex(0,0) is returned.
  /// </returns>
  const ON_2dex CurrentOutputPointCountRange() const;

  /// <returns>
  /// If the input has been used to calculate rebuilt curves, 
  /// this is the range of span counts in the rebuilt curves.
  /// Otherwise, ON_2dex(0,0) is returned.
  /// </returns>
  const ON_2dex CurrentOutputSpanCountRange() const;

  /// <returns>
  /// If the input has been used to calculate rebuilt curves, 
  /// this is the maximum deviation between an input target curve
  /// and the corresponding output rebuilt curve.
  /// Otherwise, ON_DBL_QNAN is returned.
  /// </returns>
  double CurrentOutputMaximumDeviation() const;

  /// <summary>
  /// After calling SetContext, call GetOptions() to interactively get curve rebuilding options.
  /// </summary>
  /// <param name="ui_style">
  /// Command UI style
  /// </param>
  /// <param name="callback_context">
  /// First parameter passed to the Callback() function.
  /// </param>
  /// <param name="Callback">
  /// The option getting code calls Callback() everytime and option value changes.
  /// Typically Callback() handles calculating the rebuilt curves and dynamically displaying them.
  /// callback_context is the value passed to GetOptions().
  /// bRebuildOptionsChanged will be true if an option that controls the rebuilt curve geometry changes.
  /// bDisplayOptionsChanged will be true if an option that could effect dynamic display changes.
  /// options is the updated option values.
  /// </param>
  /// <returns></returns>
  CRhinoCommand::result GetOptions(
    CRhinoCommandContext::COMMAND_STYLE ui_style,
    ON__UINT_PTR callback_context,
    void (*Callback)(
      ON__UINT_PTR callback_context,
      bool bRebuildOptionsChanged,
      bool bDisplayOptionsChanged,
      const CRhinoRebuildCurveOptions& options)
  );

  /// <summary>
  /// The value of RebuildOptionsHash() changes whenever any option that
  /// controls the geometry of the rebuilt curves changes. These options 
  /// include things like point count and degree. The value of 
  /// RebuldOptionsHash()
  /// does not change when document management options change.
  /// </summary>
  /// <returns>
  /// A SHA1 has of all the options that control the geometry of the
  /// rebuilt curves.
  /// </returns>
  const ON_SHA1_Hash RebuildOptionsHash() const;


  /// <summary>
  /// The value of DocumentOptionsHash() chages whenever any option that
  /// controls the document management like which layer the rebuilt
  /// curves will be added to and if the input curves should be deleted.
  /// The value of DocumentOptionsHash() does not change when options
  /// that control the geometry of the rebuilt curves change.
  /// </summary>
  /// <returns>
  /// A SHA1 has of all the options that control the geometry of the
  /// rebuilt curves.
  /// </returns>
  const ON_SHA1_Hash DocumentOptionsHash() const;

private:
  CRhinoCommand::result Internal_CommandLineGetOptionsHelper();
  CRhinoCommand::result Internal_DialogGetOptionsHelper();

  static int Internal_IntensityToListDexHelper(ON_NurbsCurveFitParameters::Intensity intensity);
  static ON_NurbsCurveFitParameters::Intensity Internal_ListDexToIntensityHelper(int listdex);


public:

  /// <returns>Returns the current options used to create the rebuilt curves.</returns>
  const ON_NurbsCurveFitParameters NurbsCurveFitParameters() const;

  /// <summary>
  /// User interface code should NEVER call this function.
  /// This is a low level function used by the code that calculates the fits
  /// when a fit parameter needs to be adjusted. One situation is where a user
  /// has specified kink splitting and a point count that is not large enough to
  /// accomodate splitting and the kinks. 
  /// </summary>
  /// <param name="fit_parameters"></param>
  void SetNurbsCurveFitParametersForExperts(const ON_NurbsCurveFitParameters& fit_parameters);

  /// <returns>
  /// If the context of the rebuild calculation has a distance tolerance, that value is returned.
  /// Otherwise ON_DBL_QNAN is returned.
  /// </returns>
  double ContextDistanceTolerance() const;

  /// <returns>
  /// If the context of the rebuild calculation has an agnle tolerance, that value is returned in radians.
  /// Otherwise ON_DBL_QNAN is returned.
  /// </returns>
  double ContextAngleToleranceRadians() const;

  /// <returns>
  /// True if the user interface is permitted to offer a range of
  /// point counts via SetPointCountRange().
  /// Otherwise the user interface can only offer a single value 
  /// for point count. This value is set by SetContext() and is 
  /// never changed during any subsequent interactive phases.
  /// </returns>
  bool PointCountRangesPermitted() const;

  /// <returns>
  /// The mimimum value a user interface may specify as a degree value.
  /// </returns>
  const int MinimumDegreeLimit() const;

  /// <returns>
  /// The maximum value a user interface may specify as a degree value.
  /// </returns>
  const int MaximumDegreeLimit() const;

  /// <summary>
  /// The degree the rebuit curves will have.
  /// </summary>
  /// <returns>
  /// A value between ON_NurbsCurveFitParameters::MinimumDegree and 
  /// ON_NurbsCurveFitParameters::MaximumDegree. Display this value
  /// int the dialog interface.
  /// </returns>
  int Degree() const;

  /// <summary>
  /// Sets the degree used to rebuild curves.
  /// NOTE WELL:
  /// User interface code should not attempt to validate the value. 
  /// All validation and implied adjustments are handled in 
  /// one place in opennurbs. After changing any option value,
  /// the user interface should update all controls using the Degree(),
  /// MinimumPointCount(), MaximumPiontCount() etc., member functions.
  /// </summary>
  /// <param name="degree">
  /// Desired degree. This input will be ignored if it is outside the limits
  /// specified by MinimumDegreeLimit() and MaximumDegreeLimit().
  /// </param>
  void SetDegree(int degree);

  /// <returns>
  /// The mimimum value a user interface may specify as a point count value.
  /// </returns>
  const int MinimumPointCountLimit() const;

  /// <returns>
  /// The maximum value a user interface may specify as a point count value.
  /// </returns>
  const int MaximumPointCountLimit() const;

  /// <summary>
  /// The minimum and maximum values a user interface may specify
  /// as a point count value.
  /// </summary>
  /// <returns>
  /// ON_2dex( MinimumPointCountLimit(), MaximumPointCountLimit() )
  /// </returns>
  const ON_2dex PointCountLimits() const;

  /// <summary>
  /// The minimum and maximum values a user interface may specify
  /// as a point count value when degree is fixed. If the degree can
  /// be changed, use PointCountLimits() instead.
  /// </summary>
  /// <param name="degree">
  /// The degree in the current context.
  /// </param>
  /// <returns>
  /// ON_2dex( MinimumPointCountLimit(), MaximumPointCountLimit() )
  /// </returns>
  const ON_2dex PointCountLimitsForDegree(int degree, bool bSubDFriendly) const;

  /// <summary>
  /// If the minimum and maximum point counts are equal, that value is returned.
  /// Otherwise 0 is returned.
  /// NOTE WELL:
  /// The dialog code should use this value only when it is &gt; 0.
  /// </summary>
  /// <returns>
  /// If the minimum and maximum point counts are equal, that value is returned.
  /// Otherwise 0 is returned.
  ///</returns>
  int PointCount() const;

  /// <summary>
  /// Set the minimum and maximum point count to the value of the point_count parameter.
  /// NOTE WELL:
  /// User interface code should not attempt to validate the value. 
  /// All validation and implied adjustments are handled in 
  /// one place in opennurbs. After changing any option value,
  /// the user interface should update all controls using the Degree(),
  /// PointCount(), etc., member functions. on this class.

  /// </summary>
  /// <param name="point_count">
  /// 
  /// </param>
  void SetPointCount(int point_count);

  /// <returns>
  /// The minimum point count.
  ///</returns>
  int MinimumPointCount() const;

  /// <returns>
  /// The maximum point count.
  ///</returns>
  int MaximumPointCount() const;

  const ON_2dex PointCountRange() const;

  /// <summary>
  /// When a range of point counts is permitted, 
  /// the rebuild caclulation will stop searching for better matches 
  /// using more points when the deviation between the target curve and 
  /// current rebuild candidate is &lt= PointCountRangeTolerance().
  /// </summary>
  /// <returns></returns>
  double PointCountRangeTolerance() const;

  /// <summary>
  /// Use variable point counts to get a rebuilt curve with the fewest possible points
  /// that has a deviation &lt= tolerance.
  /// </summary>
  /// <param name="minimum_point_count">
  /// Maximum number of points permitted in a rebuilt curve.
  /// </param>
  /// <param name="maximum_point_count">
  /// Maximum number of points permitted in a rebuilt curve.
  /// </param>
  /// <param name="tolerance">
  /// The rebuild caclulation will stop searching for better matches 
  /// using more points when the deviation between the target curve and 
  /// current rebuild candidate is &lt= tolerance.
  /// To get the best possible fit in the range of point counts,
  /// pass 0.
  /// </param>
  void SetPointCountRange(
    int minimum_point_count, 
    int maximum_point_count,
    double tolerance
  );

  /// <summary>
  /// 
  /// </summary>
  /// <returns>
  /// True if the current options permit variable point counts.
  /// </returns>
  bool PointCountRangeIsVariable() const;

  /// <returns>
  /// True if the user interface is permitted to offer tangent matching options.
  /// </returns>
  bool TangentMatchingPermitted() const;

  /// <returns>
  /// Determines if the tangents of the rebuilt curve match the tangents of the target curve.
  /// </returns>
  ON_NurbsCurveFitParameters::TangentMatch TangentMatching() const;

  /// <summary>
  /// Specifies if the tangents of the rebuilt curve match the tangents of the target curve.
  /// </summary>
  /// <param name="tangent_matching">
  /// </param>
  void SetTangentMatching(ON_NurbsCurveFitParameters::TangentMatch tangent_matching);

  /// <returns>
  /// True if at least one curve in the current calculation or command 
  /// has some type of kink and the user interface is permitted to
  /// offer kink splitting options.
  /// </returns>
  bool KinkSplittingPermitted() const;

  /// <returns>
  /// True if rebuilt curves will be split at target curve kinks
  /// where a kink is specified by KinkDefinition(). 
  /// </returns>
  ON_NurbsCurveFitParameters::KinkSplit KinkSplitting() const;

  void SetKinkSplitting(ON_NurbsCurveFitParameters::KinkSplit kink_splitting);

  double KinkAngleDegrees() const;

  double KinkAngleRadians() const;

  void SetKinkAngleDegrees(double kink_angle_degrees);

  void SetKinkAngleRadians(double kink_angle_radians);

  void ClearKinkAngle();

  /// <summary>
  /// If KinkSplitting() is on, then, then KinkDefinition() specifies 
  /// how target curve kinks are identified. The default value is
  /// ON_CurveKinkDefinition::DefaultTangentKink.
  /// </summary>
  /// <returns>
  /// The values used to detect kinks in the target curve.
  /// </returns>
  const ON_CurveKinkDefinition KinkDefinition() const;

  bool SubDFriendly() const;

  void SetSubDFriendly(bool bCreateSubDFriendlyCurves);

  bool SmoothingPermitted() const;
  ON_NurbsCurveFitParameters::Intensity SmoothingIntensity() const;
  void SetSmoothingIntensity(ON_NurbsCurveFitParameters::Intensity smoothing_intensity);
  void SetSmoothingCoefficient(double smoothing_coefficient);
  double SmoothingCoefficient() const;

  bool UniformityPermitted() const;
  ON_NurbsCurveFitParameters::Intensity UniformityIntensity() const;
  void SetUniformityIntensity(ON_NurbsCurveFitParameters::Intensity uniformity_intensity);
  void SetUniformityCoefficient(double uniformity_coefficient);
  double UniformityCoefficient() const;

  /// <summary>
  /// NOTE WELL: Ordinary user interfaces should not expose this setting.
  /// This is an advanced debugging option that reports the number of points
  /// sampled from the target curve. 
  /// </summary>
  /// <returns>
  /// Advanced debugging option that users should never see.
  /// </returns>
  int TargetSampleCount() const;

  /// <summary>
  /// NOTE WELL: Ordinary user interfaces should not expose this setting.
  /// This is an advanced debugging option that sets the number of points 
  /// sampled from the target curve. 
  /// </summary>
  /// <param name="target_sample_count">
  /// Advanced debugging option that users should never see.
  /// </param>
  void SetTargetSampleCount(int target_sample_count);

  bool DeleteInputCurves() const;

  void SetDeleteInputCurves(bool bDeleteInputCurves);

  bool OutputToCurrentLayer() const;

  void SetOutputToCurrentLayer(bool bOutputToCurrentLayer);

  enum DebugOptions
  {
    DebugOff = 0,
    DebugDraw = 1,
    DebugDrawAndBake = 2
  };

  enum CRhinoRebuildCurveOptions::DebugOptions DebugOption() const;

  bool DebugDisplayDraw() const;

  bool DebugDisplayBake() const;

  void SetDebugOption(enum CRhinoRebuildCurveOptions::DebugOptions debug_option);

private:
  CRhinoCommand::result Internal_GetFitPointCountParameters(
    const double doc_absolute_tolerance,
    const int min_point_count
  );


  /// <summary>
  /// Typically something like CRhinoDoc's absolute tolerance.
  /// This value is set by SetContext and exists so the GetOptions() callback can use
  /// it if needed. It plays no role in the calculation of the rebuilt curves.
  /// NOTE WELL: 
  /// This value is NOT the variable point count rebuild tolerance.
  /// This value is used for internal calculations of user interface feedback
  /// information like maximum deviation.
  /// </summary>
  double m_context_distance_tolerance = ON_DBL_QNAN;

  /// <summary>
  /// Typically something like CRhinoDoc's angle tolerance.
  /// This value is set by SetContext and exists so the GetOptions() callback can use
  /// it if needed. It plays no role in the calculation of the rebuilt curves.
  /// </summary>
  double m_context_angle_tolerance_radians = ON_DBL_QNAN;

  int m_input_curve_count = 0;
  int m_reserved = 0;
  ON_2dex m_context_input_degree_range = ON_2dex::Zero;
  ON_2dex m_context_input_point_count_range = ON_2dex::Zero;
  ON_2dex m_context_input_span_count_range = ON_2dex::Zero;

  bool m_bOpenTargetCurves = false;
  bool m_bClosedG0TargetCurves = false;
  bool m_bClosedG1TargetCurves = false;
  bool m_bTangentMatchingPermitted = false;
  bool m_bKinkSplittingPermitted = false;
  bool m_bPointCountRangesPermitted = false;
  bool Internal_ClosedTargetCurves() const;

  ON_2dex m_before_kink_splitting_point_count_range = ON_2dex::Zero;
  ON_2dex m_after_kink_splitting_point_count_range = ON_2dex::Zero;

  // Parameters that control the rebuilding process
  ON_NurbsCurveFitParameters m_fit_parameters;

  void Internal_OptionsChanged();
  void Internal_KinkSplittingOptionsChanged();
  ON_SHA1_Hash m_rebuild_options_hash = ON_SHA1_Hash::EmptyContentHash;
  ON_SHA1_Hash m_document_management_options_hash = ON_SHA1_Hash::EmptyContentHash;
  unsigned int m_rhino_doc_sn = 0;

  ON__UINT_PTR m_callback_context = 0;
  void (*m_Callback)(
    ON__UINT_PTR callback_context,
    bool bRebuildOptionsChanged,
    bool bDisplayOptionsChanged,
    const CRhinoRebuildCurveOptions& options) = nullptr;

  // Parameters that control object managment
  bool m_bDeleteInput = true;
  bool m_bOuputToCurrentLayer = true;

  // properties of the current rebuilt curves
  mutable ON_2dex m_current_output_point_count_range = ON_2dex::Zero;
  mutable ON_2dex m_current_output_span_count_range = ON_2dex::Zero;
  mutable double m_current_output_maximum_deviation = ON_DBL_QNAN;

  /// <summary>
  /// When m_bDebugDisplay is true, the final pairing is displayed during dynamic preview.
  /// </summary>
  enum DebugOptions m_debug_option = DebugOptions::DebugOff;


  ON__UINT64 m_reserved1 = 0;
  ON__UINT64 m_reserved2 = 0;
};
