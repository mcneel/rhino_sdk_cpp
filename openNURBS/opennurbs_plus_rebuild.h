#pragma once
//
// Copyright (c) 1993-2025 Robert McNeel & Associates. All rights reserved.
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

#if !defined(OPENNURBS_PLUS) || defined(OPENNURBS_PUBLIC)
#error This file should not be distributed with the public opennurbs source code toolkit.
#endif

/// <summary>
/// The NURBS curve fit tool creates a NURBS curve that approximates
/// an existing curve. The ON_NurbsCurveFitParameters class
/// contains the paramters that determine NURBS curve properties
/// (degree, point count, ...) and other fitting contstraints.
/// </summary>
class ON_CLASS ON_NurbsCurveFitParameters
{
public:
  ON_NurbsCurveFitParameters() = default;
  ~ON_NurbsCurveFitParameters() = default;
  ON_NurbsCurveFitParameters(const ON_NurbsCurveFitParameters&) = default;
  ON_NurbsCurveFitParameters& operator=(const ON_NurbsCurveFitParameters&) = default;

public:
  enum : int
  {
    MinimumDegree = 1,
    MaximumDegree = 11,
    DefaultDegree = 3,

    MinimumClampedPointCount = 2,
    MinimumClosedPointCount = 3,
    MaximumPointCount = 3000,

    DefaultSampleCount = ON_PointPairing::DefaultPointCount,

    /// <summary>
    /// Minimum number of points to sample from input curves and
    /// pass to the fitter that creates the output NURBS curve.
    /// </summary>
    MinimumSampleCount = (ON_NurbsCurveFitParameters::DefaultSampleCount / 10),

    /// <summary>
    /// Maximum number of points to sample from input curves and
    /// pass to the fitter that creates the output NURBS curve.
    /// </summary>
    MaximumSampleCount = (10 * ON_NurbsCurveFitParameters::DefaultSampleCount),
  };

  /// <summary>
  /// Give a degree and periodic setting, determine the minimum point count.
  /// </summary>
  /// <param name="degree">
  /// Degree of output NURBS curve.
  /// </param>
  /// <param name="bClosed">
  /// True if the output NURBS curve will be closed.
  /// </param>
  /// <returns>
  /// Minimum permitted point count.
  /// </returns>
  static int MinimumPointCountForDegree(int degree, bool bClosed, bool bSubDFreiendly);

  /// <summary>
  /// Given a point count and periodic setting, determine the maximum degree.
  /// </summary>
  /// <param name="point_count">
  /// Number of indpendent control points in the output NURBS curve.
  /// </param>
  /// <param name="bClosed">
  /// True if the output NURBS curve will be periodic.
  /// </param>
  /// <returns>
  /// Maximum permitted degree.
  /// </returns>
  static int MaximumDegreeForPointCount(int point_count, bool bClosed, bool bSubDFreiendly);

  static bool ValidInput(
    size_t sample_point_count,
    unsigned degree,
    unsigned control_point_count,
    bool bClosed,
    ON_Interval curve_domain
  );

  /// <summary>
  /// Returns a hash of the options that control the output curve geometry.
  /// These options are:
  /// degree (degree of the output curves),
  /// point_count (number of user control points in the output curves),
  /// sample_point_count (number of points sampled from the input curves).
  /// smoothing
  /// When bShowTangentOptions is true, end tangent matching settings are hashed.
  /// When bShowSplitAtKinksOption is true, kink parameters are hashed.
  /// This hash is useful for determing when previously calculated output curves
  /// need to be updated by comparing the hash current options and the hash
  /// of options used to calculate the output curves.
  /// </summary>
  /// <param name="bHashTangentMatching">
  /// If the set of input curves contain at least one that is open,
  /// then pass true to include end tangent matching settings in the hash.
  /// </param>
  /// <param name="bHashKinkSplitting">
  /// If the set of input curves have kinks, then pass true to include 
  /// kink splitting settings in the hash.
  /// </param>
  /// <returns>
  /// A hash of the options that control the output curve geometry.
  /// </returns>
  const ON_SHA1_Hash Hash(
    bool bHashTangentMatching,
    bool bHashKinkSplitting
  ) const;

  /// <returns>
  /// True if every curve created with these options will be periodic.
  /// </returns>
  bool Closed() const;

  /// <summary>
  /// Generally, it is best to call SetClosed() once before setting
  /// any other options and to leave it unchanged while the other options
  /// are adjusted. This is because the relationship betweem 
  /// the point count and degree options are different 
  /// for periodic and non-periodic NURBS curves.
  /// If every curve created with these options will be closed, 
  /// then call SetClosed(true). Otherwise call SetClosed(false).
  /// </summary>
  /// <param name="bClosed">
  /// Pass true if every curve created with these options will be closed.
  /// Otherwise, pass false.
  /// </param>
  void SetClosed(bool bClosed);

  /// <summary>
  /// If Closed() is true,
  /// the number of constrained points is 
  /// (TangentMatchStart() || TangentMatchEnd() ? 1 : 0) 
  /// + (TangentMatchStart()?1:0)
  /// + (TangentMatchEnd()?1:0).
  /// If Closed() is false,
  /// the number of constrained points is 
  /// 2 
  /// + (SubDFriendly() ? 2 : 0) 
  /// + (TangentMatchStart()?1:0)
  /// + (TangentMatchEnd()?1:0).
  /// </summary>
  /// <returns>
  /// Number of constrained points.
  /// </returns>
  int ConstrainedPointCount() const;

  /// <summary>
  /// Number of distinct control points in the NURBS curve fit.
  /// If a range of point counts is possible, this is the minimum
  /// number.
  /// </summary>
  /// <returns>
  /// Number of distinct control points in the NURBS curve fit.
  /// </returns>
  int PointCount() const;

  /// <returns>
  /// Minimum number of distinct control points in the NURBS curve fit.
  /// </returns>
  int PointCountRangeMinimum() const;


  /// <returns>
  /// Maximum number of distinct control points in the NURBS curve fit.
  /// </returns>
  int PointCountRangeMaximum() const;

  /// <summary>
  /// Get the point count range as an ON_2dex.
  /// </summary>
  /// <returns>
  /// PointCountRange().i = PointCountRangeMinimum()
  /// PointCountRange().j = PointCountRangeMinimum()
  /// </returns>
  const ON_2dex PointCountRange() const;


  /// <summary>
  /// When a range of point counts is permitted 
  /// and PointCountRangeTolerance() &gt; 0,
  /// then searching for the NURBS curve fit terminates if 
  /// the separation is &lt= PointCountRangeTolerance().
  /// </summary>
  /// <returns>
  /// The desired or target separation value used when a ranged of point counts
  /// is permitted.
  /// </returns>
  double PointCountRangeTolerance() const;

  /// <summary>
  /// Sets the point count and, if necessary, reduces the degree so that degree that 
  /// degree &lt;= ON_NurbsCurveFitParameters::MaximumDegree(point_count, Closed()).
  /// </summary>
  /// <param name="point_count"></param>
  void SetPointCount(int point_count);

  /// <returns>
  /// Returns true if the best fit in a range of point counts
  /// should be returned.
  /// </returns>
  bool VariablePointCount() const;

  /// <summary>
  /// Use SetPointCountRange() if you want the NURBS curve fit with the smallest separation
  /// having the point count in the specified range.
  /// </summary>
  /// <param name="minimum_point_count"></param>
  /// <param name="maximum_point_count"></param>
  void SetPointCountRange(int minimum_point_count, int maximum_point_count);

  /// <summary>
  /// Use SetPointCountRange() if you want the NURBS curve fit with the smallest separation
  /// having the point count in the specified range. This version of SetPointCountRange() lets
  /// you also specify a tolerance.
  /// </summary>
  /// <param name="minimum_point_count"></param>
  /// <param name="maximum_point_count"></param>
  /// <param name="tolerance">
  /// tolerance &gt;= 0.
  /// If you are willing to accept any separation below a certain
  /// value, pass that value as tolerance. Otherwise pass 0.0.
  /// </param>
  void SetPointCountRange(int minimum_point_count, int maximum_point_count, double tolerance);

  void GetPointCountRange(int& minimum_point_count, int& maximum_point_count, double& tolerance) const;

  int Degree() const;

  void SetDegree(int degree);

  // Number of control points if the NURBS curve will be clamped.
  // Number of total control points if the NURBS curve will be periodic.
  // This number is &gt;= PointCount() Includes SubDFriendly constrained points
  // and the duplicated point for a clamped closed curve.
  int ClampedControlPointCount() const;

  // Number of total control points if the NURBS curve will be periodic.
  // This number is &gt;= PointCount() and includes periodic duplicates.
  int PeriodicControlPointCount() const;

  bool OptimizeCurve() const;

  void SetOptimizeCurve(bool bOptimizeCurve);

  /// <summary>
  /// The TangentMatch enum is used to constrain the tangents of the rebuilt
  /// curve to match those of the target curve.
  /// </summary>
  enum class TangentMatch : unsigned char
  {
    /// <summary>
    /// Ignore target curve start and end tangent directions.
    /// </summary>
    None = 0,

    /// <summary>
    /// Match the target curve start tangent direction.
    /// </summary>
    AtStart = 1,

    /// <summary>
    /// Match the target curve end tangent direction.
    /// </summary>
    AtEnd = 2,

    /// <summary>
    /// Match the target curve start and end tangent direction.
    /// </summary>
    AtStartAndEnd = 3,
  };

  /// <summary>
  /// Convert an integer value to the
  /// ON_NurbsCurveFitParameters::TangentMatch enum
  /// with the same value.
  /// </summary>
  /// <param name="tangent_match_as_int"></param>
  /// <param name="invalid_input_result">
  /// If tangent_match_as_int does not match an TangentMatch enum value,
  /// then this value is returned.
  /// </param>
  /// <returns></returns>
  static ON_NurbsCurveFitParameters::TangentMatch TangentMatchFromInt(
    int tangent_match_as_int,
    ON_NurbsCurveFitParameters::TangentMatch invalid_input_result
  );

  ON_NurbsCurveFitParameters::TangentMatch TangentMatching() const;

  void SetTangentMatching(ON_NurbsCurveFitParameters::TangentMatch tangent_matching);

private:
  // Returns true if TangentMatch() is AtStart or AtStartAndEnd.
  bool Internal_TangentMatchStart() const;

  // Returns true if TangentMatch() is AtEnd or AtStartAndEnd.
  bool Internal_TangentMatchEnd() const;

public:

  /// <summary>
  /// The KinkSplit enum is used to determine how target curve kinks 
  /// (abrupt changes in target or curvature) are handled during rebuilding.
  /// </summary>
  enum class KinkSplit : unsigned char
  {
    /// <summary>
    /// All target curve kinks are ignored and rebuilt curves will be G2.
    /// </summary>
    None = 0,

    /// <summary>
    /// The rebuilt curve is permitted have a tangent change at
    /// locations where the target curve has tangent changes. 
    /// Curvature changes in the target curve are ignored.
    /// </summary>
    AtG1Changes = 1,

    /// <summary>
    /// The rebuilt curve is permitted have a tangent or curvature changes
    /// at locations where the target curve has a tangent 
    /// or large curvature changes. 
    /// </summary>
    AtLargeG2Changes = 2,

    /// <summary>
    /// The rebuilt curve is permitted have a tangent or curvature changes
    /// at locations where the target curve has a tangent 
    /// or medium curvature changes.
    /// </summary>
    AtMediumG2Changes = 3,

    /// <summary>
    /// The rebuilt curve is permitted have a tangent or curvature changes
    /// at locations where the target curve has a tangent 
    /// or small curvature changes. This option is the most sensitive to splitting
    /// at any tangent or curvature kink.
    /// </summary>
    AtSmallG2Changes = 4,
  };

  /// <summary>
  /// Convert an integer value to the
  /// ON_NurbsCurveFitParameters::KinkSplit enum
  /// with the same value.
  /// </summary>
  /// <param name="kink_split_as_int"></param>
  /// <param name="invalid_input_result">
  /// If kink_split_as_int does not match an KinkSplit enum value,
  /// then this value is returned.
  /// </param>
  /// <returns></returns>
  static ON_NurbsCurveFitParameters::KinkSplit KinkSplitFromInt(
    int kink_split_as_int,
    ON_NurbsCurveFitParameters::KinkSplit invalid_input_result
  );

  /// <summary>
  /// 
  /// </summary>
  /// <returns>
  /// Types of target curve kinks that get separate rebuilding on either side
  /// of the kink.
  /// </returns>
  ON_NurbsCurveFitParameters::KinkSplit KinkSplitting() const;

  /// <summary>
  /// 
  /// </summary>
  /// <param name="kink_splitting">
  /// Specifies what type of target curve kinks get separate rebuilding on
  /// either side of the kink.
  /// </param>
  void SetKinkSplitting(
    ON_NurbsCurveFitParameters::KinkSplit kink_splitting
  );

  double KinkAngleDegrees() const;

  double KinkAngleRadians() const;

  void SetKinkAngleDegrees(double kink_angle_degrees);

  void SetKinkAngleRadians(double kink_angle_radians);

  /// <summary>
  /// When kink splitting is enabled, the kink definition
  /// determines what type of target curve tangent of curvature change
  /// is considered to be a kink.
  /// </summary>
  /// <returns>
  /// Current kink definition.
  /// </returns>
  const ON_CurveKinkDefinition KinkDefinition() const;

  /// <summary>
  /// Control when tangent matching is applied at kinks. 
  /// Note that this setting does somthing only when some
  /// tangent matching option is true and the input curve has kinks.
  /// </summary>
  /// <returns>
  /// If true, the TangentMatchStart() and  TangentMatchEnd() settings are applied at kinks. 
  /// Otherwise, the tangent matching applies only to the ends of curves.
  /// </returns>
  bool ApplyTangentMatchingAtKinks() const;

  /// <summary>
  /// Control when tangent matching is applied at kinks. 
  /// Note that this setting does somthing only when some
  /// tangent matching option is true and the input curve has kinks.
  /// </summary>
  /// <param name="bApplyTangentMatchAtKinks">
  /// If true, the TangentMatchStart() and  TangentMatchEnd() settings are applied at kinks. 
  /// Otherwise, the tangent matching applies only to the ends of curves.
  /// </param>
  void SetApplyTangentMatchingAtKinks(bool bApplyTangentMatchingAtKinks);

  bool SubDFriendly() const;

  void SetSubDFriendly(bool bSubDFriendly);

  /// <summary>
  /// The Intensity enum is used to select a predefined or a custom value
  /// for the smoothing, uniformity, and curvature bias coefficients.
  /// </summary>
  enum class Intensity : unsigned char
  {
    None = 0,
    Low = 1,
    Moderate = 2,
    Medium = 3,
    High = 4,
    Extreme = 5,
    Custom = 6
  };

  static ON_NurbsCurveFitParameters::Intensity IntensityFromInt(
    int intensity_as_int,
    ON_NurbsCurveFitParameters::Intensity invalid_input_result
  );

  /// <summary>
  /// Smoothing coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Low.
  /// </summary>
  static constexpr double LowSmoothingCoefficient = 0.001;

  /// <summary>
  /// Smoothing coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Moderate.
  /// </summary>
  static constexpr double ModerateSmoothingCoefficient = 0.01;

  /// <summary>
  /// Smoothing coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Medium.
  /// </summary>
  static constexpr double MediumSmoothingCoefficient = 0.1;

  /// <summary>
  /// Smoothing coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::High.
  /// </summary>
  static constexpr double HighSmoothingCoefficient = 1.0;

  /// <summary>
  /// Smoothing coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Extreme.
  /// </summary>
  static constexpr double ExtremeSmoothingCoefficient = 10.0;

  /// <summary>
  /// Maximum permitted smoothing coefficient.
  /// </summary>
  static constexpr double MaximumSmoothingCoefficient = 10.0;

  /// <summary>
  /// The smoothing penalty in the objective function penalizes triplets of
  /// contiguous control points that are not colinear and equally spaced.
  /// Use SmoothingCoefficient() to get the numerical coefficient used in 
  /// the objective function. Use SetSmoothingCoefficient() to set a custom 
  /// smoothing coefficient.
  /// </summary>
  /// <returns>
  /// The intensity of the smoothing penalty in the objective function.
  /// </returns>
  ON_NurbsCurveFitParameters::Intensity SmoothingIntensity() const;

  /// <summary>
  /// Use one of the one of ON_NurbsCurveFitParameters::Intensity enum values
  /// to specify a smoothing coefficient. This method is provided to 
  /// make it easier to provide an interface that lets a user
  /// select reasonable values for the smoothing penalty without
  /// having to understand the objective fuction calculation.
  /// </summary>
  /// <param name="smoothing_intensity">
  /// One of ON_NurbsCurveFitParameters::Intensity enum values.
  /// If ON_NurbsCurveFitParameters::Intensity::Custom is passed in
  /// and a preexisting custom smoothing penalty has not been specified
  /// using SetSmoothingCoefficient(), then SmothingIntensity() is set to none.
  /// If you want to set a custom smoothing coefficient, use SetSmoothingCoefficient(smoothing_coefficient).
  /// </param>
  void SetSmoothingIntensity(ON_NurbsCurveFitParameters::Intensity smoothing_intensity);

  /// <summary>
  /// Specify a smoothing coefficient.
  /// </summary>
  /// <param name="smoothing_coefficient">
  /// Valid values are in the range 0 &lt;= smoothing_coefficient &lt;= ON_NurbsCurveFitParameters::MaximumCustomSmoothingCoefficient.
  /// If smoothing_coefficient is not valid, then smoothing intensity is set to ON_NurbsCurveFitParameters::Smoothing::None.
  /// </param>
  void SetSmoothingCoefficient(double smoothing_coefficient);

  /// <returns>
  /// The value of the smoothing penalty coefficient used in the objective function.
  /// 0 indicates no smoothing penalty. 
  /// Values &gt; 0 and &lt;= ON_NurbsCurveFitParameters::MaximumCustomSmoothingCoefficient
  /// indicate a smoothing penalty will be applied.
  /// NaN indicates invalid settings and no smoothing penalty will be applied.
  /// Note that a smoothing penalty only applies when ther are 3 or more control points
  /// in the NURBS curve fit.
  /// </returns>
  double SmoothingCoefficient() const;


  /// <summary<coefficient>
  /// Uniformity coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Low.
  /// </summary>
  static constexpr double LowUniformityCoefficient = 0.001;

  /// <summary<coefficient>
  /// Uniformity coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Moderate.
  /// </summary>
  static constexpr double ModerateUniformityCoefficient = 0.01;

  /// <summary>
  /// Uniformity coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Medium.
  /// </summary>
  static constexpr double MediumUniformityCoefficient = 0.1;

  /// <summary>
  /// Uniformity coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::High.
  /// </summary>
  static constexpr double HighUniformityCoefficient = 1.0;

  /// <summary>
  /// Uniformity coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::High.
  /// </summary>
  static constexpr double ExtremeUniformityCoefficient = 10.0;

  /// <summary>
  /// Maximum permitted uniformity coefficient.
  /// </summary>
  static constexpr double MaximumUniformityCoefficient = 10.0;

  /// <summary>
  /// The uniformity penalty in the objective function penalizes variation 
  /// in the distances between control points.
  /// Use UniformityCoefficient() to get the numerical coefficient used in 
  /// the objective function. Use SetUniformityCoefficient() to set a custom 
  /// uniformity penalty coefficient.
  /// </summary>
  /// <returns>
  /// The intensity of the uniformity penalty in the objective function.
  /// </returns>
  ON_NurbsCurveFitParameters::Intensity UniformityIntensity() const;

  /// <summary>
  /// Use one of ON_NurbsCurveFitParameters::Intensity enum values
  /// to specify a uniformity coefficient. This method is provided to 
  /// make it easier to provide an interface that lets a user
  /// select reasonable values for the uniformity coefficient without
  /// having to understand the optimization calculation.
  /// Positive uniformity coefficients make the distances between control points
  /// more uniform. 
  /// In general, use the smallest value that gives the desired result.
  /// </summary>
  /// <param name="uniformity_intensity">
  /// One of ON_NurbsCurveFitParameters::Intensity enum values.
  /// If ON_NurbsCurveFitParameters::Intensity::Custom is passed in
  /// and a preexisting custom uniformity penalty has not been specified
  /// using SetUniformityCoefficient(), then UniformityIntensity() is set to none.
  /// If you want to set a custom uniformity coefficient, use SetUniformityCoefficient(x).
  /// </param>
  void SetUniformityIntensity(ON_NurbsCurveFitParameters::Intensity uniformity_intensity);

  /// <summary>
  /// Specify a uniformity coefficient.
  /// Positive uniformity coefficients make the distances between control points
  /// more uniform. 
  /// In general, use the smallest value that gives the desired result.
  /// </summary>
  /// <param name="uniformity_coefficient">
  /// Valid values are in the range 0 &lt;= uniformity_coefficient &lt;= ON_NurbsCurveFitParameters::MaximumUniformityCoefficient.
  /// If uniformity_coefficient is not valid, then uniformity is set to ON_NurbsCurveFitParameters::Uniformity::None.
  /// </param>
  void SetUniformityCoefficient(double uniformity_coefficient);

  /// <returns>
  /// The value of the uniformity penalty coefficient used in the objective function.
  /// 0 indicates no uniformity penalty. 
  /// Values &gt; 0 and &lt;= ON_NurbsCurveFitParameters::MaximumCustomUniformityCoefficient
  /// indicate uniformity penalty will be applied.
  /// NaN indicates invalid settings and no uniformity penalty will be applied.
  /// Note that uniformity penalty only applies when there are 3 or more control points
  /// in the NURBS curve fit.
  /// </returns>
  double UniformityCoefficient() const;

  /// <summary>
  /// Curvature bias coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Low.
  /// </summary>
  static constexpr double LowCurvatureBiasCoefficient = 0.001;

  /// <summary>
  /// Curvature bias coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Moderate.
  /// </summary>
  static constexpr double ModerateCurvatureBiasCoefficient = 0.01;

  /// <summary>
  /// Curvature bias coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::Medium.
  /// </summary>
  static constexpr double MediumCurvatureBiasCoefficient = 0.1;

  /// <summary>
  /// Curvature bias coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::High.
  /// </summary>
  static constexpr double HighCurvatureBiasCoefficient = 1.0;

  /// <summary>
  /// Curvature bias coefficient that corresponds to ON_NurbsCurveFitParameters::Intensity::High.
  /// </summary>
  static constexpr double ExtremeCurvatureBiasCoefficient = 10.0;

  /// <summary>
  /// Maximum permitted curvature bias coefficient.
  /// </summary>
  static constexpr double MaximumCurvatureBiasCoefficient = 10.0;

  /// <summary>
  /// Use one of ON_NurbsCurveFitParameters::Intensity enum values
  /// to specify a curvature bias. This method is provided to 
  /// make it easier to provide an interface that lets a user
  /// select reasonable values for the curvature bias coefficient without
  /// having to understand the objective function.
  /// Positive curvature bias coefficients make the fit curve closer 
  /// to regions in the target curve with relatively higher curvature.
  /// In general, use the smallest value that gives the desired result.
  /// </summary>
  /// <param name="curvature_bias_intensity">
  /// One of ON_NurbsCurveFitParameters::Intensity enum values.
  /// If ON_NurbsCurveFitParameters::Intensity::Custom is passed in
  /// and a preexisting custom curvature bias has not been specified
  /// using SetCurvatureBiasCoefficient(), then CurvatureBiasIntensity() is set to none.
  /// If you want to set a custom uniformity coefficient, use SetCurvatureBiasCoefficient(x).
  /// </param>
  void SetCurvatureBiasIntensity(ON_NurbsCurveFitParameters::Intensity curvature_bias_intensity);

  /// <summary>
  /// Curvature bias adds a bias to fit target points with relatively higher
  /// curvatures more closely.
  /// Use CurvatureBiasCoefficient() to get the numerical coefficient used in 
  /// the objective function. Use SetCurvatureBiasCoefficient() to set a custom 
  /// curvature bias coefficient.
  /// </summary>
  /// <returns>
  /// The intensity of the curvature bias in the objective function.
  /// </returns>
  ON_NurbsCurveFitParameters::Intensity CurvatureBiasIntensity() const;

  /// <summary>
  /// Specify a curvature bias coefficient.
  /// Positive curvature bias coefficients make the fit curve relatively closer
  /// to the target curve in regions where the target curve has relatively higher curvature.
  /// In general, use the smallest value that gives the desired result.
  /// </summary>
  /// <param name="uniformity_coefficient">
  /// Valid values are in the range 0 &lt;= uniformity_coefficient &lt;= ON_NurbsCurveFitParameters::MaximumCurvatuerBiasCoefficient.
  /// If curvature_bias_coefficient is not valid, then curvature bias is set to ON_NurbsCurveFitParameters::Uniformity::None.
  /// </param>
  void SetCurvatureBiasCoefficient(double curvature_bias_coefficient);

  double CurvatureBiasCoefficient() const;

  int SampleCount() const;

  void SetSampleCount(int sample_count);

  void SetProgressReporter(class ON_ProgressReporter* progress_reporter);

  class ON_ProgressReporter* ProgressReporter() const;

  void SetTerminator(class ON_Terminator* terminator);

  class ON_Terminator* Terminator() const;

  bool TerminationRequested() const;

private:
  // This is the number of points the NURBS curve is fit through.
  // Is it common for these points to be sampled from an input curve.
  int m_sample_count = ON_NurbsCurveFitParameters::DefaultSampleCount;

  // Degree of the NURBS curve fit.
  // If m_bSubDFriendly is true, m_degree must be 3.
  int m_degree = ON_NurbsCurveFitParameters::DefaultDegree;

  // Number of indpendent control points in the NURBS curve fit.
  // If m_point_count1 > m_point_count0 
  // and m_target_maximum_separation >= 0
  // then the smallest point count in the range
  // m_point_count0 <= point_count <= m_point_count1
  // that results in a maximum separation <= m_target_maximum_separation
  // or minimal separation will be used.
  // Otherwise m_point_count0 is used.
  int m_point_count0 = (ON_NurbsCurveFitParameters::DefaultDegree + 1);

  // If m_point_count1 > m_point_count0 
  // and m_target_maximum_separation >= 0
  // then the smallest point count in the range
  // m_point_count0 <= point_count <= m_point_count1
  // that results in a maximum separation <= m_target_maximum_separation
  int m_point_count1 = 0;
  double m_target_maximum_separation = ON_DBL_QNAN;

  /// <summary>
  /// True if the NURBS curve fit will be closed (start = end).
  /// </summary>
  bool m_bClosed = false;

  // If m_bOptimizeCurve = false, then options for Curvature Bias, Kink Splitting, and Smoothing are ignored
  // and the Greville interpolant is returned. The Rhino 1 - Rhino 8 Rebuild command created a result similar
  // what is created when OptimizeCurve = false.
  bool m_bOptimizeCurve = true;

  /// <summary>
  /// Controls matching of target curve tangents.
  /// </summary>
  ON_NurbsCurveFitParameters::TangentMatch m_tangent_matching = ON_NurbsCurveFitParameters::TangentMatch::None;


  /// <summary>
  /// Determines when a target curve kink get independent rebuilds on either side.
  /// </summary>
  ON_NurbsCurveFitParameters::KinkSplit m_kink_splitting = ON_NurbsCurveFitParameters::KinkSplit::None;

  /// <summary>
  /// If true, the m_tangent_match setting is applied at kinks. 
  /// Otherwise, m_tangent_match applies only to the ends of curves.
  /// </summary>
  bool m_bApplyTangentMatchingAtKinks = true;

  /// <summary>
  /// If SubD friendly is true, then degree = 3
  /// Clampled endse constraints:
  ///   start point = fixed
  ///   end point = fixed
  ///   point_count >= 2.
  ///   The triple of control points at the ends satisfies P1 = (2*P0 + P2)/3
  ///   where P0 = end, P1 = first/penultimate P2 = second/second before last.
  ///   In the optimpzation, the coordinates of P2 are the variables and P1
  ///   is expressed in termps of P0 and P2. P0 is the fixed start/end.
  ///   Total control point count = m_point_count + 2;
  /// Periodic constraints:
  ///   degree = 3;
  ///   m_point_count >= 3;
  ///   Total control point count = m_point_count + 3;
  /// </summary>
  bool m_bSubDFriendly = false;


  /// <summary>
  /// Smoothing penalty used to make triples of consecutive control polygon points colinear.
  /// Use SmoothingCoefficient() to convert this enum to the the value passed to the optimizer.
  /// </summary>
  ON_NurbsCurveFitParameters::Intensity m_smoothing_intensity = ON_NurbsCurveFitParameters::Intensity::None;

  /// <summary>
  /// Uniformity penalty used to make the distance between control points more equal.
  /// Use UniformityCoefficient() to convert this enum to the the value passed to the optimizer.
  /// </summary>
  ON_NurbsCurveFitParameters::Intensity m_uniformity_intensity = ON_NurbsCurveFitParameters::Intensity::None;

  /// <summary>
  /// Curvature bias is used to make the fit curve stay close to regions of relatively high curvature.
  /// Use CurvatureBiasCoefficient() to convert this enum to the the value passed to the optimizer.
  /// </summary>
  ON_NurbsCurveFitParameters::Intensity m_curvature_bias_intensity = ON_NurbsCurveFitParameters::Intensity::None;

  ON__UINT16 m_reserved1 = 0;
  ON__UINT32 m_reserved2 = 0;
  ON__UINT64 m_reserved3 = 0;

  /// <summary>
  /// When m_smoothing_intensity = ON_NurbsCurveFitParameters::Intensity::Custom,
  /// SmoothingCoefficient() returns m_custom_smoothing_coefficient.
  /// </summary>
  double m_custom_smoothing_coefficient = ON_DBL_QNAN;

  /// <summary>
  /// When m_uniformity_intensity = ON_NurbsCurveFitParameters::Intensity::Custom,
  /// UniformityCoefficient() returns m_custom_uniformity_coefficient.
  /// </summary>
  double m_custom_uniformity_coefficient = ON_DBL_QNAN;

  /// <summary>
  /// When m_curvature_bias = ON_NurbsCurveFitParameters::Intensity::Custom,
  /// CurvatureBiasCoefficient() returns m_custom_curvature_bias.
  /// </summary>
  double m_custom_curvature_bias_coefficient = ON_DBL_QNAN;

  ON_CurveKinkDefinition m_kink_definition = ON_CurveKinkDefinition::DefaultTangentKink;

  class ON_ProgressReporter* m_progress_reporter = nullptr;
  class ON_Terminator* m_terminator = nullptr;
};

/// <summary>
/// ON_TemplatedCurveRebuilder rebuilds a curve (the "source" curve) so that it
/// takes on the degree, knot vector, and control point count of a "template"
/// NURBS curve.
///
/// The template curve is analyzed once with SetTemplateCurve(); the rebuilder
/// can then be applied to many source curves with RebuildCurve().
/// </summary>
class ON_CLASS ON_TemplatedCurveRebuilder
{
public:
  ON_TemplatedCurveRebuilder() = default;
  ~ON_TemplatedCurveRebuilder() = default;
  ON_TemplatedCurveRebuilder(const ON_TemplatedCurveRebuilder&) = default;
  ON_TemplatedCurveRebuilder& operator=(const ON_TemplatedCurveRebuilder&) = default;

  /// <summary>
  /// Analyze the template curve. Computes and stores the template's NURBS form
  /// (clamped for open/simple-closed templates), its Greville abscissae, its
  /// closed/periodic state, and the control point range to interpolate.
  /// </summary>
  /// <param name="template_curve">
  /// The curve whose degree, knots, and control point count the rebuilt source
  /// curves will match.
  /// </param>
  /// <returns>
  /// True if the template curve is usable. If false is returned, the rebuilder
  /// is left unset.
  /// </returns>
  bool SetTemplateCurve(const ON_Curve& template_curve);

  /// <returns>
  /// True if SetTemplateCurve() has succeeded and the rebuilder is ready to
  /// use.
  /// </returns>
  bool IsSet() const;

  /// <returns>
  /// The template curve's stored NURBS form, or an empty curve when the
  /// rebuilder is not set.
  /// </returns>
  const ON_NurbsCurve& TemplateCurve() const;

  /// <summary>
  /// Rebuild one source curve so it matches the template curve's degree, knot
  /// vector, and control point count.
  /// </summary>
  /// <param name="source_curve">
  /// The curve to rebuild.
  /// </param>
  /// <param name="bFlipSourceDirection">
  /// If true and the source curve is open, its direction is flipped (reversed)
  /// before sampling. Set this when the source runs opposite the template.
  /// </param>
  /// <param name="bPreserveEndTangents">
  /// If true and the rebuilt curve is open with at least 4 control points, the
  /// interior end control points are adjusted so the rebuilt curve's end
  /// tangents match the source's.
  /// </param>
  /// <param name="bMakeSubDFriendly">
  /// If true, the rebuilt curve is converted to a SubD friendly curve.
  /// </param>
  /// <param name="destination">
  /// The rebuilt curve is returned here.
  /// </param>
  /// <returns>
  /// True if a valid curve was created.
  /// </returns>
  bool RebuildCurve(
    const ON_Curve& source_curve,
    bool bFlipSourceDirection,
    bool bPreserveEndTangents,
    bool bMakeSubDFriendly,
    ON_NurbsCurve& destination
  ) const;

private:
  bool m_bIsSet = false;

  // 0 = open, 1 = closed but not periodic, 2 = periodic
  int m_template_curve_is_closed = 0;

  // Control point interpolation range [m_template_curve_cv0,
  // m_template_curve_cv1] (inclusive) into the template's knot/Greville
  // structure.
  int m_template_curve_cv0 = 0;
  int m_template_curve_cv1 = 0;

  ON_NurbsCurve m_template_nurbs_curve;
  ON_SimpleArray<double> m_template_greville_abcissa;
};

#ifdef OPENNURBS_IN_RHINO
/// <summary>
/// The NURBS curve fit tool creates a NURBS curve that approximates
/// an existing curve. The aproximation is calculated as a minimization
/// of an objective function that measures the deviation between 
/// the original curve and the NURBS approximation.
/// The ON_NurbsCurveFitObjectiveValue stores the results of the
/// minimization calculation. This is nerdy information that will
/// be interesting to a few nerds and everybody else can safely ignore
/// it and get on with their lives.
/// </summary>
class ON_CLASS ON_NurbsCurveFitObjectiveValue
{
public:
  static const ON_NurbsCurveFitObjectiveValue Nan;
  static const ON_NurbsCurveFitObjectiveValue Zero;

public:
  ON_NurbsCurveFitObjectiveValue() = default;
  ~ON_NurbsCurveFitObjectiveValue() = default;
  ON_NurbsCurveFitObjectiveValue(const ON_NurbsCurveFitObjectiveValue&) = default;
  ON_NurbsCurveFitObjectiveValue& operator=(const ON_NurbsCurveFitObjectiveValue&) = default;

public:
  ON_NurbsCurveFitObjectiveValue(
    unsigned evaluaton_count,
    unsigned optimal_evaluation,
    double initial_value,
    double optimal_value,
    double deviation,
    double smoothness,
    double uniformity
  );

  bool IsSet() const;

public:
  /// <summary>
  /// Number of objective function evaluations during optimization
  /// </summary>
  unsigned m_evaluation_count = 0u;

  /// <summary>
  /// If m_optimal_evaluation > 0, it is value of m_evaluation_count
  /// for the evaluation that yielded m_optimal_value.
  /// </summary>
  unsigned m_optimal_evaluation = 0u;

  /// <summary>
  /// Objective function value of the seed curve.
  /// </summary>
  double m_initial_value = ON_DBL_QNAN;


  /// <summary>
  /// Objective function value of the optimized  curve.
  /// Optimization sets the control points to minimize the objective value.
  /// </summary>
  double m_optimal_value = ON_DBL_QNAN;

  /// <summary>
  /// deviation component of m_optimal_value
  /// </summary>
  double m_deviation = ON_DBL_QNAN;

  /// <summary>
  /// smoothness component of m_optimal_value
  /// If m_minimum_smoothness &gt;= 0.0, then the objective function
  /// had a smoothness factor. 
  /// Otherwise, smoothness was not included of the objective function.
  /// </summary>
  double m_smoothness = ON_DBL_QNAN;

  /// <summary>
  /// uniformity component of m_optimal_value
  /// If m_uniformity &gt;= 0.0, then the objective function
  /// had a uniformity factor. 
  /// Otherwise, uniformity was not included of the objective function.
  /// </summary>
  double m_uniformity = ON_DBL_QNAN;

  double AddEvaluation(
    double deviation,
    double smoothness,
    double uniformity
    );

  void AddSegment(ON_NurbsCurveFitObjectiveValue& segment_value);

  const ON_wString ToString(bool bVerbose) const;

  /// <summary>
  /// The Rhino document runtime serial number of is typically used for debugging purposes.
  /// The calculations performed in opennurbs do not use this information.
  /// </summary>
  /// <param name="rhino_doc_runtime_serial_number"></param>
  void SetRhinoDocSerialNumber(unsigned rhino_doc_runtime_serial_number);

  /// <summary>
  /// The Rhino document runtime serial number of is typically used for debugging purposes.
  /// The calculations performed in opennurbs do not use this information.
  /// </summary>
  /// <returns>
  /// The Rhino document runtime serial number.
  /// </returns>
  unsigned RhinoDocSerialNumber() const;

private:
  unsigned m_rhino_doc_runtime_serial_number = 0;
  unsigned m_reserved = 0;
};

/// <summary>
/// The NURBS curve fit tool creates a NURBS curve that approximates
/// an existing curve. When the approximation preserves kinks in the
/// input curve, the ON_NurbsCurveFitSegment class is used to save
/// the smooth approximations between kinks.
/// </summary>
class ON_CLASS ON_NurbsCurveFitSegment
{
public:
  ON_NurbsCurveFitSegment() = default;
  ~ON_NurbsCurveFitSegment() = default;
  ON_NurbsCurveFitSegment(const ON_NurbsCurveFitSegment&) = default;
  ON_NurbsCurveFitSegment& operator=(const ON_NurbsCurveFitSegment&) = default;

public:
  /// <summary>
  /// 
  /// </summary>
  /// <param name="domain">
  /// Domain of the parent curve the segment is part of.
  /// </param>
  /// <param name="degree">
  /// Degree of the NURBS curve fit.
  /// </param>
  /// <returns>
  /// True if the segment domain and degree make sense.
  /// </returns>
  bool IsSet(const ON_Interval domain, const int degree) const;

  // domain of the segment
  ON_Interval m_domain = ON_Interval::Nan;

  // m_continuity[] specifies the continuity with respect to the
  // ON_CurveKinkDefinition settings used to create the segments.
  // m_continuity[0] is the continuity between the previous segment and this segment.
  // m_continuity[1] is the continuity between this segment and the next segment.
  // This setting is used to determine if interior segment end tangets should
  // be matched. If the continuity at an interior segment end is G1,
  // then end tangents will be matched beause the "split" was a result
  // of a G2 discontinuity.
  ON::continuity m_continuity[2] = {};

  /// <summary>
  /// 0.0 &lt= m_relative_length &lt; 1.0 is the relative length of the segment (or an estimate) with respect to the entire curve
  /// and is used to determine how many control points are assigned to the segment.
  /// </summary>
  double m_relative_length = 0.0;

  // degree of the segment
  int m_degree = 0;

  // number of control points assigned to the segment.
  int m_control_point_count = 0;
};

#if defined(ON_DLL_TEMPLATE)
ON_DLL_TEMPLATE template class ON_CLASS ON_SimpleArray<ON_NurbsCurveFitSegment>;
#endif

/// <summary>
/// The NURBS curve fit tool creates a NURBS curve that approximates
/// an existing curve. The ON_NurbsCurveFitBuilder is used to collect
/// the information needed to calulcate this approximation and to
/// calculate the approximation. This class is designed to partition 
/// the steps of the calcaulation into steps that need to be done
/// one time for each input curve and steps that need to be done every
/// time a value changes in ON_NurbsCurveFitParameters.
/// </summary>
class ON_CLASS ON_NurbsCurveFitBuilder
{
public:

  /// <summary>
  /// A hash of NURBS curve properties aside from control point locations.
  /// </summary>
  /// <param name="dim">
  /// Eucleadin dimension (For rational nurbs, do not include the weight)
  /// Pass 0 if dimension should be ignored in the current context.
  /// </param>
  /// <param name="degree"></param>
  /// <param name="cv_count">
  /// Number of control points.
  /// </param>
  /// <param name="weights_stride">
  /// Pass 0 if weights should be ignored in the current context.
  /// If the curve is rational and the weights are fixed in the context being considered,
  /// then the weights are weights[i*weights_stride] where 0 &lt;= i &lt; cv_count.
  /// </param>
  /// <param name="weights">
  /// Pass nullptr if weights should be ignored in the current context.
  /// If the curve is rational and the weights are fixed in the context being considered,
  /// then the weights are weights[i*weights_stride] where 0 &lt;= i &lt; cv_count.
  /// Otherwise pass nullptr for weights.
  /// </param>
  /// <param name="knots">
  /// Pass nullptr if knots should be ignored in the current context.
  /// The knot vector.
  /// </param>
  /// <returns>
  /// A hash of the nurbs properties.
  /// </returns>
  static const ON_SHA1_Hash NurbsPropertiesHash(
    int dim,
    int degree,
    int cv_count,
    size_t weights_stride,
    const double* weights,
    const double* knots
  );

  /// <summary>
  /// A hash of NURBS curve properties aside from control point locations.
  /// The knot values are always included in the hash.
  /// </summary>
  /// <param name="bIncludeWeights">
  /// If nurbs_curve is rational and bIncludeWeights is true, then the weights
  /// are included in the hash.
  /// </param>
  /// <returns>
  /// A hash of the nurbs properties.
  /// </returns>
  static const ON_SHA1_Hash NurbsPropertiesHash(const ON_NurbsCurve& nurbs_curve, bool bIncludeWeights);

  /// <summary>
  /// A hash of NURBS curve properties aside from control point locations.
  /// The knot values are always included in the hash.
  /// If nurbs_curve is rational, the weights are included in the hash.
  /// </summary>
  /// <returns>
  /// A hash of the nurbs properties.
  /// </returns>
  static const ON_SHA1_Hash NurbsPropertiesHash(const ON_NurbsCurve& nurbs_curve);

  static const ON_NurbsCurveFitBuilder Unset;

public:
  ON_NurbsCurveFitBuilder() = default;
  ~ON_NurbsCurveFitBuilder() = default;
  ON_NurbsCurveFitBuilder(const ON_NurbsCurveFitBuilder&) = default;
  ON_NurbsCurveFitBuilder& operator=(const ON_NurbsCurveFitBuilder&) = default;

public:
  bool InitializeFromObjectRef(ON_ObjRef oref, const ON_Curve* oref_curve);

  bool InitializeFromInputCurve(const ON_Curve* input_curve, ON_Interval domain, bool bCheckForKinks);

private:
  static const ON_SHA1_Hash Internal_InputCurveNurbFormHash(const ON_Curve* input_curve, const ON_Interval domain, ON_BoundingBox* nurbs_curve_bbox);

public:

  void ClearKinkSegments();

  unsigned SetKinkSegmentsIntervals(const ON_CurveKinkDefinition kink_definition);

  void ClearKinkSegmentsPointCounts();

  int SetKinkSegmentsPointCounts(int degree, int desired_point_count);

  /// <summary>
  /// CalculateNurbsCurveFit() calculates the NURBS curve fit and stores 
  /// the result in m_nurbs_curve_fit.
  /// </summary>
  /// <param name="fit_parameters">
  /// Parameters that control the form of the NURBS curve fit.
  /// </param>
  /// <param name="bUseNurbFormWhenPossible">
  /// If thre is an input curve, bUseNurbFormWhenPossible is true,
  /// and the NURBS form of the input curve has the NURBS form 
  /// specified in fit_parameters, then the NURBS curve fit will be the
  /// NURBS form of the input curve.This option is use when the expectation
  /// of the caller is that curves with matching NURBS form will not
  /// be changed.
  /// </param>
  /// <param name="debug_rhino_doc_sn">
  /// Typically 0.
  /// If you are debugging this calculation in a Rhino debug build,
  /// pass the serial number of the Rhino document. 
  /// Otherwise pass 0.
  /// </param>
  /// <returns>
  /// True if the NURBS curve fit was successfully calculated.
  /// </returns>
  bool CalculateNurbsCurveFit(
    const class ON_NurbsCurveFitParameters& fit_parameters,
    bool bUseNurbFormWhenPossible,
    unsigned debug_rhino_doc_sn
  );

private:
  bool Internal_CalculateNurbsCurveFit(
    const class ON_NurbsCurveFitParameters& fit_parameters,
    bool bUseNurbFormWhenPossible,
    unsigned debug_rhino_doc_sn
  );

public:


  /// <param name="input_parameter">
  /// If the NURBS curve fit was created by sampling points from an input curve
  /// and the input curve parameter of the maximum separation point
  /// can be calculated, it is returned in input_parameter.
  /// Otherwise ON_DBL_QNAN is returned in input_parameter.
  /// </param>
  /// <param name="nurbs_curve_fit_parameter">
  /// If the NURBS curve fit parameter of the maximum separation point
  /// can be calculated, it is returned in nurbs_curve_fit_parameter.
  /// Otherwise ON_DBL_QNAN is returned in nurbs_curve_fit_parameter.
  /// </param>
  /// <returns>
  /// Returns the maximum separation between the input curve or points and the NURBS curve fit.
  /// The ON_Line.from point is on the input 
  /// and the ON_Line.to point is on the NURBS curve fit.
  /// </returns>
  const ON_Line MaximumSeparation(
    double& input_parameter,
    double& nurbs_curve_fit_parameter
  ) const;

  /// <returns>
  /// Returns the maximum separation between the input curve or points and the NURBS curve fit.
  /// The ON_Line.from point is on the input 
  /// and the ON_Line.to point is on the NURBS curve fit.
  /// </returns>
  const ON_Line MaximumSeparation() const;


  /// <summary>
  /// Clears the rebuld curve and associated information. The kink segments and
  /// input points are not cleared because they depend only on the input. 
  /// Typically, ClearNurbsCurveFit() is called when the fitting paramters
  /// change and the NURBS curve fit needs to be recalculated.
  /// </summary>
  void ClearNurbsCurveFit();

public:
  /// <summary>
  /// The Rhino object used to select the input curve. 
  /// This object can be a curve object or something like a 
  /// brep, subd, or extrusion that has the curve as a component.
  /// </summary>
  ON_ObjRef m_input_oref;

  const ON_Curve* m_input_curve = nullptr;

  /// <summary>
  /// Domain of the portion of input_curve to fit. 
  /// m_domain is always inlcuded in m_input_curve->Domain().
  /// The NURBS curve fit will have this domain as well and 
  /// this is critical to make Rhino history work as expected.
  /// </summary>
  ON_Interval m_domain = ON_Interval::Nan;

  /// <summary>
  /// True if the input curve is closed and not G1 at the start/end.
  /// The curve may have additional tangent discontinuities on the interior of the domain.
  /// </summary>
  bool m_bClosedG0 = false;

  /// <summary>
  /// True if the input curve is closed and G1 at the start/end.
  /// The curve may have tangent discontinuities on the interior of the domain.
  /// </summary>
  bool m_bClosedG1 = false;

  /// <returns>
  /// True if m_bClosedG0 or m_bClosedG1 is true.
  /// </returns>
  bool IsClosed() const;

  /// <summary>
  /// True if the input curve is open
  /// </summary>
  bool m_bOpen = false;

  /// <summary>
  /// Set to true if it is possible that some setting of the "SplitAtKinks" option
  /// would find a kink on the interior of the input curve.
  /// NOTE WELL: 
  /// This bool indicates that if the user sets the strictest definition of a "kink,"
  /// then the curve will have a kink. If the sure uses larger angle tolerances or doesn't
  /// care about curvature changes, then the kink that caused m_InputCurveConatainsKink
  /// to be true may not be severe enought to quality as a kink in the NURBS curve fit.
  /// </summary>
  bool m_SplitAtKinksCandidate = false;

  /// <returns>
  /// True if m_bOpen or m_bClosedG0 is true.
  /// </returns>
  bool TangentMatchCandidate() const;


private:
  ON__UINT32 m_reserved1 = 0;

public:
  /// <summary>
  /// The hash in m_input_nurb_form_hash is from NurbsPropertiesHash(NURBS form of input curve). 
  /// When NurbsPropertiesHash() has the same values
  /// for the NURBS form and for fitting parameter, 
  /// then CalculateNurbsCurveFit can use the NURBS form instead of using
  /// optimization to calculate a NURBS curve fit.
  /// </summary>
  ON_SHA1_Hash m_input_nurb_form_hash = ON_SHA1_Hash::EmptyContentHash;

  ON_BoundingBox m_input_curve_bbox = ON_BoundingBox::NanBoundingBox;

private:
  int m_reserved = 0;

private:
  // If the input curve has reflection or rotation symmetries,
  // they are saved here so the rebuilt curve can have the same
  // symmetries when that is possible.
  int m_symmetry_mirror_plane_count = 0;
  ON_PlaneEquation m_symmetry_mirror_planes[2] = { ON_PlaneEquation::NanPlaneEquation , ON_PlaneEquation::NanPlaneEquation };
  ON_Plane m_symmetry_rotation_axis = ON_Plane::NanPlane;
  double m_symmetry_rotation_radius = ON_DBL_QNAN;
  double m_symmetry_snap_tolerance = ON_DBL_QNAN;

public:
  /// <summary>
  /// When m_kink_segments[] is not empty, it is a contiguous list of segments
  /// that contain no kinks and are separated by a kink.
  /// 
  /// Use SetKinkSegmentsIntervals() to set 
  /// m_kink_segments[].m_domain and m_kink_segments[].m_relative_length segments.
  /// 
  /// Use SetKinkSegmentsPointCounts() to set m_kink_segments[].m_degree and m_kink_segments[].m_control_point_count.
  /// </summary>
  ON_SimpleArray<ON_NurbsCurveFitSegment> m_kink_segments;

  /// <summary>
  /// Use SetKinkSegmentsIntervals() to set 
  /// m_kink_segments[].m_domain and m_kink_segments[].m_relative_length segments.
  /// 
  /// m_kink_parameters_hash is a SHA1 has of the paramters used to calculate 
  /// the m_kink_segments[].m_domain and m_kink_segments[].m_relative_length
  /// values. If m_kinks_parameters_hash = ON_SHA1_Hash::EmptyContentHash, then
  /// kinks in the input curve are ignored and m_kink_segments[] should be empty. 
  /// Otherwise m_kink_segments[] is either empty (no kinks) or is 
  /// a contiguous array of segments that contain no kinks in their interiors.
  /// </summary>
  ON_SHA1_Hash m_kink_definition_hash = ON_SHA1_Hash::EmptyContentHash;

  /// <summary>
  /// The degree assigned to each kink segment.
  /// Use SetKinkSegmentsPointCounts() to set this value.
  /// </summary>
  int m_kink_segments_degree = 0;

  /// <summary>
  /// The number of points that will be in a kinky elmo curve.
  /// Use SetKinkSegmentsPointCount() to set this value.
  /// </summary>
  int m_kink_segments_point_count = 0;


  int KinkSegmentsTotalPointCount() const;

  /// <summary>
  /// m_point_pairing.Points(A) are points no the input curve 
  /// and  m_point_pairing.Points(B) are pionts on the NURBS curve fit.
  /// </summary>
  ON_PointPairing m_point_pairing;

  /// <summary>
  /// Value of the objective function when the NURBS curve fit was optimized.
  /// </summary>
  ON_NurbsCurveFitObjectiveValue m_objective_value = ON_NurbsCurveFitObjectiveValue::Nan;

  /// <summary>
  /// A NURBS curve fit to the input curve.
  /// </summary>
  ON_NurbsCurve m_nurbs_curve_fit;

  ON_BoundingBox m_nurbs_curve_fit_bbox = ON_BoundingBox::NanBoundingBox;

  // The point count used for m_nurbs_curve_fit. This is useful when 
  // a variable point count is used to determine which point count 
  // generated the NURBS curve fit returned in m_nurbs_curve_fit and when
  // kink splitting requires increased point counts.
  int m_nurbs_curve_fit_point_count = 0;

  // Value of ON_NurbsCurveFitParameters::Hash for the ON_NurbsCurveFitParameters
  // used to calculate m_nurbs_curve_fit.
  ON_SHA1_Hash m_fit_parameters_hash = ON_SHA1_Hash::EmptyContentHash;

  // The results of calculating MaximumSeparation are cached here.
  // If m_input_curve exists, m_maximum_separation.from is on m_input_curve;
  // otherwise it is a point in m_point_pairing.ListA().
  // m_maximum_separation.to is on m_nurbs_curve_fit.
  mutable ON_Line m_maximum_separation = ON_Line::NanLine;

  // m_maximum_separation_parameters[0] is an m_input_curve parameter
  // when m_input_curve exists.
  // m_maximum_separation_parameters[1] is an m_nurbs_curve_fit parameter.
  mutable double m_maximum_separation_parameters[2] = { ON_DBL_QNAN, ON_DBL_QNAN };
};
#endif // OPENNURBS_IN_RHINO
