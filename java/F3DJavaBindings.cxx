#include <jni.h>

#include <engine.h>
#include <log.h>

#include <memory>

#define JAVA_API(function)                                                                   \
  JNIEXPORT void JNICALL Java_io_github_f3d_1app_f3d_NativeWrapper_##function

std::unique_ptr<f3d::engine> engine;

extern "C"
{
  JAVA_API(toggleOption)(JNIEnv* env, jobject, jstring option)
  {
    const char* str = env->GetStringUTFChars(option, nullptr);
    engine->getOptions().toggle(str);
    env->ReleaseStringUTFChars(option, str);
  }

  JAVA_API(addFile)(JNIEnv* env, jobject, jstring path)
  {
    const char* str = env->GetStringUTFChars(path, nullptr);
    engine->getLoader().addFile(str);
    env->ReleaseStringUTFChars(path, str);
  }

  JAVA_API(init)(JNIEnv*, jobject)
  {
    engine = std::make_unique<f3d::engine>(f3d::window::Type::NATIVE);
  }

  JAVA_API(resize)(JNIEnv*, jobject, jint width, jint height)
  {
    engine->getWindow().setSize(width, height);
  }

  JAVA_API(render)(JNIEnv*, jobject)
  {
    engine->getWindow().render();
  }

  JAVA_API(loadPrevious)(JNIEnv*, jobject)
  {
    engine->getLoader().loadFile(f3d::loader::LoadFileEnum::LOAD_PREVIOUS);
  }

  JAVA_API(loadNext)(JNIEnv*, jobject)
  {
    engine->getLoader().loadFile(f3d::loader::LoadFileEnum::LOAD_NEXT);
  }

  JAVA_API(loadCurrent)(JNIEnv*, jobject)
  {
    engine->getLoader().loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  }

  JAVA_API(rotate)(JNIEnv* env, jobject, jint dx, jint dy)
  {
    f3d::window& window = engine->getWindow();
    f3d::camera& camera = window.getCamera();

    double delta_elevation = 200.0 / window.getWidth();
    double delta_azimuth = -200.0 / window.getHeight();

    double rxf = dx * delta_azimuth;
    double ryf = dy * delta_elevation;

    camera.azimuth(rxf);
    camera.elevation(ryf);

    window.render();
  }

  JAVA_API(pan)(JNIEnv* env, jobject, jint dx, jint dy)
  {
    f3d::window& window = engine->getWindow();
    f3d::camera& camera = window.getCamera();

    f3d::point3_t focus = camera.getFocalPoint();
    f3d::point3_t focusDC = window.getDisplayFromWorld(focus);

    f3d::point3_t shiftDC = { focusDC[0] - dx, focusDC[1] + dy, focusDC[2] };
    f3d::point3_t shift = window.getWorldFromDisplay(shiftDC);

    f3d::vector3_t motion = { shift[0] - focus[0], shift[1] - focus[1], shift[2] - focus[2] };

    f3d::point3_t pos = camera.getPosition();

    camera.setFocalPoint({ motion[0] + focus[0], motion[1] + focus[1], motion[2] + focus[2] });
    camera.setPosition({ motion[0] + pos[0], motion[1] + pos[1], motion[2] + pos[2] });

    window.render();
  }

  JAVA_API(zoom)(JNIEnv* env, jobject, jdouble factor)
  {
    engine->getWindow().getCamera().dolly(factor);
    engine->getWindow().render();
  }
}
