#ifndef VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_BASE_HPP_
#define VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_BASE_HPP_

#include <glm/glm.hpp>

#include <QObject>

#include <QOpenGLFunctions_3_2_Core>

#include <array>
#include <memory>
#include <string>
#include <vector>

/// @defgroup colorscales Color Scales
/// @brief All color scale management classes.
///
/// The ColorScales group brings together every class related to color scale management in the program. However, do note
/// that most of the classes in this group have been defined and implemented, but not used yet.
///
/// They should undergo a round of testing before being used, and the visualization pipeline should be ready to accept
/// a color scale of this type into it (currently, the visualization pipeline handles its own custom color scales).

class Scene;	// fwd-decl

/// @brief The Color namespace holds the different classes that manage a color scale.
namespace Color {

	/// @brief Enum to keep track of the underlying scale's type when using std::dynamic_pointer_cast<>.
	enum ColorScale_t {
		Generic,	///< Generic scale, unusable in its current state.
		SimpleGradient,	   ///< 2-point gradient scale.
		Functor,	///< Defined by a lambda function
	};

	/// @ingroup colorscales
	/// @brief The ColorScaleBase class defines a function interface to create, manage & use color scales in the program
	/// @details It allows to create a color scale of arbitrary contents, and sample it at any point in the range [0,1].
	/// This is useful for not only passing the color scales to the visualization pipeline, but also to display them in
	/// a Qt widget ! Which is why it inherits from QObject, in order to have signals/slots.
	/// @note Not used yet in the visualization pipeline.
	class ColorScaleBase : public QObject {
		// Qt definitions :
		Q_OBJECT;
		Q_PROPERTY(std::string name_user READ getName WRITE setName NOTIFY nameChanged);
		Q_PROPERTY(std::size_t sample_count READ getSampleCount WRITE setSampleCount NOTIFY sampleCountChanged);

	public:
		/// @brief Handy typedef to a shared pointer of this color scale.
		typedef std::shared_ptr<ColorScaleBase> Ptr;

		/// @brief Default deletor for the color scale, removes all references.
		~ColorScaleBase(void);

		/// @brief Returns the internal sample count.
		std::size_t getSampleCount(void) const;

		/// @brief Sets a new internal sample count.
		void setSampleCount(std::size_t _new_sample_count);

		/// @brief Returns the user-defined name for this color scale
		std::string getName(void) const;

		/// @brief Allows to change the (user-defined) name of the color scale.
		/// @warning If the string is empty, nothing's changed.
		void setName(std::string _new_user_name);

		/// @brief Returns true if the data was changed on the host side, but not uploaded to the device side yet.
		bool needsUpdate(void) const;

		/// @brief Uploads the color scale to the GL.
		void uploadData(Scene* _func);

		/// @brief Returns the texture handle, useful for binding the texture to a sampler in GLSL.
		GLuint textureHandle(void) const;

		/// @brief Samples the color scale at a certain point between O (min) and 1 (max).
		virtual glm::vec3 sample(float f) const = 0;

		/// @brief Allows to sample the whole color scale, using the internal number of samples.
		virtual std::vector<glm::vec3> getColorScale() const = 0;

		/// @brief Allows to sample the whole color scale, using a user-defined number of samples.
		virtual std::vector<glm::vec3> getColorScale(std::size_t number_of_steps) const = 0;

		/// @brief Returns the color scale's type, defined by the enum values contained within.
		ColorScale_t getColorScaleType() const noexcept { return this->cs_type; }

	signals:
		/// @brief Signal fired when the user-defined name of the color scale changes.
		void nameChanged();

		/// @brief Changed the sample count of the data.
		void sampleCountChanged();

		/// @brief Signal fired when the data changes on the host side.
		void dataChanged();

		/// @brief Signal fired when the data is uploaded to the GL.
		void dataUploaded();

	protected slots:
		/// @brief Slot fired by derived classes when they change a variable, updating the time of host updates.
		void internal_data_update(void);
		/// @brief Slot fired after the data has been uploaded to the GL.
		void after_update(void);

	protected:
		/// @brief Default ctor for the color scale. Defined as
		ColorScaleBase(std::size_t target_sample_count = 256);

		/// The type of the color scale. Useful for tracking the real underlying type of a pointer.
		ColorScale_t cs_type;

		/// @brief The texture handle associated with the texture.
		GLuint texture_handle;

		/// @brief The color scale's user-defined name.
		std::string name_user;

		/// @brief The number of samples to generate when uploading the color scale to
		std::size_t sample_count;

		/// @brief The last time the data was updated, on the host side.
		std::chrono::steady_clock::time_point last_update_host;

		/// @brief The time the data was uploaded to the device.
		std::chrono::steady_clock::time_point last_update_device;
	};

}	 // namespace Color

#endif	  // VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_BASE_HPP_
