#include <besu_fzz.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <jni.h>

extern const char* besu_classpath;

namespace {
struct JVM {
  JNIEnv* env = nullptr;
  JavaVM* jvm = nullptr;
  jclass fzz_class = nullptr;
  jmethodID validateEOF = nullptr;
  jbyteArray byteArr = nullptr;
  jsize byteArrLen = 1024;

  JVM() {
    JavaVMOption options[] = {
        {.optionString = const_cast<char*>(besu_classpath)},
        // {.optionString = "-Xcheck:jni"},

        /* This disables Java's signal handler,
         * so that aborting the fuzzer (ctrl+c) won't
         * cause a crash.
         */
        {
            .optionString = (char*)"-Xrs",
        },
        // {const_cast<char*>("-Xmx512m")}, // max heap size
    };
    JavaVMInitArgs vm_args{
        .version = JNI_VERSION_21,
        .nOptions = std::size(options),
        .options = options,
    };

    if (JNI_CreateJavaVM(&jvm, reinterpret_cast<void**>(&env), &vm_args) !=
        JNI_OK)
      std::abort();

    fzz_class = env->FindClass("BesuFzz");
    if (fzz_class == nullptr) {
      const auto ex = env->ExceptionOccurred();
      if (ex != nullptr) {
        env->ExceptionDescribe();
        env->ExceptionClear();
      }
      std::abort();
    }

    validateEOF = env->GetStaticMethodID(fzz_class, "validateEOF", "([BI)Z");

    byteArr = env->NewByteArray(byteArrLen);
  }

  ~JVM() { jvm->DestroyJavaVM(); }
};

JVM jvm;
} // namespace

bool fzz_besu_validate_eof(const uint8_t* data, size_t size) noexcept {
  // TODO: Besu recognizes legacy by prefix, so don't pass non-EOF code.
  if (size < 2 || data[0] != 0xEF || data[1] != 0x00)
    return false;

  const auto env = jvm.env;
  const auto length = static_cast<jsize>(size);

  if (jvm.byteArrLen < length) {
    // This actually does not improve performance and we can just allocate array
    // each time.
    env->DeleteLocalRef(jvm.byteArr);
    jvm.byteArr = env->NewByteArray(length);
    jvm.byteArrLen = length;
  }

  env->SetByteArrayRegion(jvm.byteArr, 0, length,
                          reinterpret_cast<const jbyte*>(data));
  const auto res = env->CallStaticBooleanMethod(jvm.fzz_class, jvm.validateEOF,
                                                jvm.byteArr, length);
  if (env->ExceptionOccurred() != nullptr) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    std::abort();
  }
  return res;
}
