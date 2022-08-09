#pragma once

#include <pages/page.h>
#include <thunder_auto.h>
#include <imgui_internal.h>

// Field dimensions (meters).
#define FIELD_X 15.5702 // 54' 1"
#define FIELD_Y 8.1026 // 26' 7"

struct Project;

class PathEditorPage : public ProjectPage {
public:
  static PathEditorPage* get() {
    return &instance;
  }
  
  PathEditorPage(PathEditorPage const&) = delete;
  PathEditorPage& operator=(PathEditorPage const&) = delete;
  
  void present(bool* running) override;

  /**
   * @brief Sets the working project.
   *
   * @param project The project to work with.
   */
  void set_project(Project* project) override;

  /**
   * @brief Exports the current path to a CSV file.
   */
  void export_path();

  /**
   * @brief Represents a user-defined point on a path.
   */
  struct CurvePoint {
    // The point on the path.
    float px;
    float py;
    // The heading at the point.
    float h0;
    float h1;
    // The weigths of the two heading constraints.
    float w0;
    float w1;
    // The rotation of the robot at the point.
    float rotation;
    // Whether the robot should stop at this point.
    bool stop;
    // Whether this is the starting point of the path.
    bool begin;
    // Whether this is the ending point of the path.
    bool end;

    /**
     * @brief Returns the coordinates of a tangent control point.
     *
     * @param first Whether it return the first or second control point.
     * @return The coordinates of the tangent control point, or std::nullopt if there is no control point.
     */
    std::optional<ImVec2> get_tangent_pt(bool first) const;

    /**
     * @brief Sets the coordinates of a tangent control point.
     *
     * @param first Whether to set the first or second control point.
     * @param x The x coordinate of the control point.
     * @param y The y coordinate of the control point.
     */
    void set_tangent_pt(bool first, float x, float y);

    /**
     * @brief Returns the coordinates of the rotation control point.
     *
     * @return The coordinates of the rotation control point.
     */
    ImVec2 get_rot_pt(bool reverse = false) const;

    /**
     * @brief Returns the coordinates of a corner of the robot rotation widget.
     *
     * @param index The index of the corner to return (0 = FL, 1 = FR, 2 = BL, 3 = BR).
     * @return The coordinates of the corner.
     */
    ImVec2 get_rot_corner_pt(int index) const;

    /**
     * @brief Sets the coordinates of the rotation control point.
     *
     * @param x The x coordinate of the control point.
      * @param y The y coordinate of the control point.
      */
    void set_rot_pt(float x, float y);

    /**
     * @brief Translates the point by a specified amount.
     *
     * @param dx The amount to translate in the x direction.
     * @param dy The amount to translate in the y direction.
     */
    void translate(float dx, float dy);
  };

  using CurvePointTable = std::vector<CurvePoint>;

  /**
   * @brief Returns the currently selected point.
   *
   * @return The currently selected point, or std::nullopt if no point is selected.
   */
  std::optional<CurvePointTable::iterator> get_selected_point();

  /**
   * @brief Resets the selection.
   */
  void reset_selected_point();

  /**
   * @brief Deletes the currently selected point.
   */
  void delete_point();

  /**
   * @brief Tells the path editor to update the path.
   */
  constexpr void update() { updated = true; }

  /**
   * @brief Sets whether the path editor shows the tangent control points.
   *
   * @param show Whether to show the tangent control points.
   */
  constexpr void set_show_tangents(bool show) { show_tangents = show; }

  /**
   * @brief Sets whether the path editor shows the rotation control points.
   *
   * @param show Whether to show the rotation control points.
   */
  constexpr void set_show_rotation(bool show) { show_rotation = show; }

  enum class CurveStyle : std::size_t {
    VELOCITY = 0,
    CURVATURE = 1,
  };

  /**
   * @brief Sets the style of the curve to provide different feedback to the user.
   *
   * @param style The style of the curve.
   */
  constexpr void set_curve_style(CurveStyle style) { curve_style = style; }
  
private:
  PathEditorPage();
  ~PathEditorPage();

  /**
   * @brief Presents the path editor widget.
   */
  void present_curve_editor();

  /**
   * @brief Calculates the points of the curve.
   *
   * @return The points of the curve.
   */
  std::vector<ImVec2> calc_curve_points() const;

  /**
   * @brief Calculates a point on the curve.
   *
   * @param pt_it The iterator to the curve part to calculate the point on.
   * @param t The parameter to the curve function (0-1).
   * @return The point on the curve.
   */
  ImVec2 calc_curve_point(CurvePointTable::const_iterator pt_it, float t) const;

  /**
   * @brief Calculates the length of the path.
   *
   * @return A list of lengths of the parts of the path.
   */
  std::vector<float> calc_curve_lengths() const;

  /**
   * @brief Calculates the length of a part of the path.
   *
   * @param pt_it The iterator to the curve part to calculate the length of.
   * @return The length of the curve part.
   */
  float calc_curve_part_length(CurvePointTable::const_iterator pt_it) const;

  /**
   * @brief Calculates the curvature of the path.
   *
   * @return The curvature of all the points of the path.
   */
  std::vector<float> calc_curvature() const;

  /**
   * @brief Calculates the velocity and time of each point of the path.
   *
   * @return A list of velocities and times for each point of the path.
   */
  std::tuple<std::vector<float>, std::vector<float>, std::vector<float>> calc_velocity_time() const;

  std::pair<CurvePointTable::const_iterator, float> find_curve_part_point(float x, float y) const;

  std::vector<ImVec2>::const_iterator find_curve_point(float x, float y) const;

  /**
   * @brief Represents a point exported to the CSV file.
   */
  struct ExportCurvePoint {
    float time;
    float x;
    float y;
    float heading;
    float velocity;
  };

  // The current working project.
  inline static Project* project = nullptr;

  // Cached values for the path editor.
  std::vector<float> cached_curve_lengths;
  std::vector<ImVec2> cached_curve_points;
  std::vector<float> cached_curvatures;
  std::vector<float> cached_velocities;
  std::vector<float> cached_times;
  std::vector<float> cached_rotations;

  /**
   * @brief Calculates and caches values for the path editor.
   */
  void cache_values();

  // The current point selected.
  CurvePointTable::iterator selected_pt;

  // The style of the curve.
  CurveStyle curve_style = CurveStyle::VELOCITY;

  // Whether the path editor should update its values on the next frame.
  bool updated = true;

  bool show_tangents = true;
  bool show_rotation = true;

  // The aspect ratio of the field image.
  float field_aspect_ratio;
  // The OpenGL texture ID of the field image.
  unsigned int field_tex;

  ImVec2 field_offset;
  float field_scale = 1.0f;

  /**
   * @brief Converts a screen coordinate to a field coordinate.
   */
  ImVec2 to_field_coord(ImVec2 pt, bool apply_offset = true) const;

  /**
   * @brief Converts a field coordinate to a screen coordinate.
   */
  ImVec2 to_draw_coord(ImVec2 pt, bool apply_offset = true) const;

  /**
   * @brief Adjusts a field coordinate to fit the defined field bounds.
   */
  ImVec2 adjust_field_coord(ImVec2 pt) const;

  /**
   * @brief Unadjusts a field coordinate from the defined field bounds to the texture bounds.
   */
  ImVec2 un_adjust_field_coord(ImVec2 pt) const;
  
  // The widget's bounding box.
  ImRect bb;
  
  static PathEditorPage instance;
};
