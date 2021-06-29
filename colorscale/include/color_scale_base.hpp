#ifndef VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_BASE_HPP_
#define VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_BASE_HPP_

#include <glm/glm.hpp>

#include <QObject>

#include <QOpenGLFunctions_3_2_Core>

#include <array>
#include <vector>
#include <string>
#include <memory>

class Scene; // fwd-decl

namespace Color {

	/// @b Enum to keep track of the underlying scale's type when using std::dynamic_pointer_cast<>.
	enum ColorScale_t {
		Generic,			///< Generic scale, unusable in its current state.
		SimpleGradient,		///< 2-point gradient scale.
		Functor,			///< Defined by a lambda function
	};

	class ColorScaleBase : public QObject {
		// Qt definitions :
			Q_OBJECT;
			Q_PROPERTY(std::string name_user READ getName WRITE setName NOTIFY nameChanged);
			Q_PROPERTY(std::size_t sample_count READ getSampleCount WRITE setSampleCount NOTIFY sampleCountChanged);

		public:
			/// @b Handy typedef to a shared pointer of this color scale.
			typedef std::shared_ptr<ColorScaleBase> Ptr;

			/// @b Default deletor for the color scale, removes all references.
			~ColorScaleBase(void);

			/// @b Returns the internal sample count.
			std::size_t getSampleCount(void) const;

			/// @b Sets a new internal sample count.
			void setSampleCount(std::size_t _new_sample_count);

			/// @b Returns the user-defined name for this color scale
			std::string getName(void) const;

			/// @b Allows to change the (user-defined) name of the color scale.
			/// @warning If the string is empty, nothing's changed.
			void setName(std::string _new_user_name);

			/// @b Returns true if the data was changed on the host side, but not uploaded to the device side yet.
			bool needsUpdate(void) const;

			/// @b Uploads the color scale to the GL.
			void uploadData(Scene* _func);

			/// @b Returns the texture handle, useful for binding the texture to a sampler in GLSL.
			GLuint textureHandle(void) const;

			/// @b Samples the color scale at a certain point between O (min) and 1 (max).
			virtual glm::vec3 sample(float f) const = 0;

			/// @b Allows to sample the whole color scale, using the internal number of samples.
			virtual std::vector<glm::vec3> getColorScale() const = 0;

			/// @b Allows to sample the whole color scale, using a user-defined number of samples.
			virtual std::vector<glm::vec3> getColorScale(std::size_t number_of_steps) const = 0;

			/// @b Returns the color scale's type, defined by the enum values contained within.
			ColorScale_t getColorScaleType() const noexcept { return this->cs_type; }

		signals:
			/// @b Signal fired when the user-defined name of the color scale changes.
			void nameChanged();

			/// @b Changed the sample count of the data.
			void sampleCountChanged();

			/// @b Signal fired when the data changes on the host side.
			void dataChanged();

			/// @b Signal fired when the data is uploaded to the GL.
			void dataUploaded();

		protected slots:
			/// @b Slot fired by derived classes when they change a variable, updating the time of host updates.
			void internal_data_update(void);
			/// @b Slot fired after the data has been uploaded to the GL.
			void after_update(void);

		protected:
			/// @b Default ctor for the color scale. Defined as
			ColorScaleBase(std::size_t target_sample_count = 256);

			/// The type of the color scale. Useful for tracking the real underlying type of a pointer.
			ColorScale_t cs_type;

			/// @b The texture handle associated with the texture.
			GLuint texture_handle;

			/// @b The color scale's user-defined name.
			std::string name_user;

			/// @b The number of samples to generate when uploading the color scale to
			std::size_t sample_count;

			/// @b The last time the data was updated, on the host side.
			std::chrono::steady_clock::time_point last_update_host;

			/// @b The time the data was uploaded to the device.
			std::chrono::steady_clock::time_point last_update_device;
	};

}

#endif // VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_BASE_HPP_
