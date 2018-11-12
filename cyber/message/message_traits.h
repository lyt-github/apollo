/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef CYBER_MESSAGE_MESSAGE_TRAITS_H_
#define CYBER_MESSAGE_MESSAGE_TRAITS_H_

#include <string>

#include "cyber/message/intra_message_traits.h"
#include "cyber/message/protobuf_traits.h"
#include "cyber/message/py_message_traits.h"
#include "cyber/message/raw_message_traits.h"

namespace apollo {
namespace cyber {
namespace message {

#define DEFINE_TYPE_TRAIT(name, func)                            \
  template <typename T>                                          \
  class name {                                                   \
   private:                                                      \
    template <typename Class>                                    \
    static char Test(decltype(&Class::func)*);                   \
    template <typename>                                          \
    static int Test(...);                                        \
                                                                 \
   public:                                                       \
    static constexpr bool value = sizeof(Test<T>(nullptr)) == 1; \
  };                                                             \
                                                                 \
  template <typename T>                                          \
  constexpr bool name<T>::value;

DEFINE_TYPE_TRAIT(HasType, TypeName)
DEFINE_TYPE_TRAIT(HasDescriptor, GetDescriptorString)

template <typename T>
class HasSerializer {
 private:
  template <typename Class>
  static char Test(decltype(&Class::SerializeToString)*,
                   decltype(&Class::ParseFromString)*);
  template <typename>
  static int Test(...);

 public:
  static constexpr bool value = sizeof(Test<T>(nullptr, nullptr)) == 1;
};

// avoid potential ODR violation
template <typename T>
constexpr bool HasSerializer<T>::value;

template <typename T>
typename std::enable_if<HasSerializer<T>::value, bool>::type ParseFromString(
    const std::string& str, T* message) {
  return message->ParseFromString(str);
}

template <typename T>
typename std::enable_if<!HasSerializer<T>::value, bool>::type ParseFromString(
    const std::string& str, T* message) {
  return false;
}

template <typename T>
typename std::enable_if<HasSerializer<T>::value, bool>::type SerializeToString(
    const T& message, std::string* str) {
  return message.SerializeToString(str);
}

template <typename T>
typename std::enable_if<!HasSerializer<T>::value, bool>::type SerializeToString(
    const T& message, std::string* str) {
  return false;
}

template <typename T,
          typename std::enable_if<HasType<T>::value, bool>::type = 0>
std::string MessageType(const T& message) {
  return T::TypeName();
}

template <typename T,
          typename std::enable_if<HasType<T>::value, bool>::type = 0>
std::string MessageType() {
  return T::TypeName();
}

template <typename T,
          typename std::enable_if<
              !HasType<T>::value &&
                  !std::is_base_of<google::protobuf::Message, T>::value,
              bool>::type = 0>
std::string MessageType() {
  return typeid(T).name();
}

template <typename T,
          typename std::enable_if<HasDescriptor<T>::value, bool>::type = 0>
void GetDescriptorString(const std::string& type, std::string* desc_str) {
  T::GetDescriptorString(type, desc_str);
}

template <typename T,
          typename std::enable_if<
              !HasDescriptor<T>::value &&
                  !std::is_base_of<google::protobuf::Message, T>::value,
              bool>::type = 0>
void GetDescriptorString(const std::string& type, std::string* desc_str) {}

template <typename MessageT,
          typename std::enable_if<
              !std::is_base_of<google::protobuf::Message, MessageT>::value,
              int>::type = 0>
void GetDescriptorString(const MessageT& message, std::string* desc_str) {}

}  // namespace message
}  // namespace cyber
}  // namespace apollo

#endif  // CYBER_MESSAGE_MESSAGE_TRAITS_H_
