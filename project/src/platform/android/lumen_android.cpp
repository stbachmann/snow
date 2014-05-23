#ifdef ANDROID


#include <jni.h>
#include <android/log.h>

#include <map>
#include <string>

#include "lumen_core.h"
#include "common/ByteArray.h"
#include "platform/android/lumen_android.h"
#include "lumen_io.h"
#include "libs/sdl/SDL.h"

#define LOG(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "/ lumen /", __VA_ARGS__))


static JavaVM* java_vm;

#ifdef LUMEN_USE_OPENAL
    extern "C" {
        void alcandroid_OnLoad( JavaVM *vm );
        void alcandroid_OnUnload( JavaVM *vm );
        void alcandroid_Resume();
        void alcandroid_Suspend();
    }
#endif //LUMEN_USE_OPENAL

  //This is called after SDL gets inited and passes
#ifdef LUMEN_USE_SDL

    int SDL_main(int argc, char *argv[]) {
        LOG("/ lumen / android post init SDL_main");
        return 0;
    }

#endif //LUMEN_USE_SDL

namespace lumen {

    std::map<std::string, jclass> jClassCache;

    void init_core_platform() {

        lumen::log("/ lumen / android core platform init");
        jClassCache = std::map<std::string, jclass>();

        JNIEnv *env = GetEnv();
        env->GetJavaVM( &java_vm );

        #ifdef LUMEN_USE_OPENAL
            alcandroid_OnLoad( java_vm );
        #endif

    } //init_core_platform

    void shutdown_core_platform() {

        #ifdef LUMEN_USE_OPENAL
            alcandroid_OnUnload( java_vm );
        #endif

    } //shutdown_core_platform

    void update_core_platform() {

    } //update_core_platform

    void on_system_event_platform( const SystemEvent &event ) {

        switch( event.type ) {

            case se_app_didenterbackground: {
                alcandroid_Suspend();
                break;
            }

            case se_app_didenterforeground: {
                alcandroid_Resume();
                break;
            }

            default :{
                return;
            }
        }

    } //on_system_event

    JNIEnv *GetEnv() {

        return (JNIEnv*)SDL_AndroidGetJNIEnv();

    } //JNIEnv

    jclass FindClass(const char *className,bool inQuiet) {

        std::string cppClassName(className);
        jclass ret;

        if(jClassCache[cppClassName]!=NULL) {

            ret = jClassCache[cppClassName];

        } else {

            JNIEnv *env = GetEnv();
            jclass tmp = env->FindClass(className);

            if (!tmp) {
                if (inQuiet) {
                    jthrowable exc = env->ExceptionOccurred();
                    if (exc) {
                        env->ExceptionClear();
                    }
                } else {
                    CheckException(env);
                }

                return 0;
            }

            ret = (jclass)env->NewGlobalRef(tmp);
            jClassCache[cppClassName] = ret;
            env->DeleteLocalRef(tmp);
        }

        return ret;

    } //findClass

    void CheckException(JNIEnv *env, bool inThrow) {

        jthrowable exc = env->ExceptionOccurred();
        if (exc) {
            env->ExceptionDescribe();
            env->ExceptionClear();

            if (inThrow) {
                val_throw(alloc_string("JNI Exception"));
            }
        }

    } //CheckException

} //namespace lumen

#endif