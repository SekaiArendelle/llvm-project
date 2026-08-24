// RUN: %clang_analyze_cc1 -std=c++23 -analyzer-checker=cplusplus.Move \
// RUN:   -analyzer-output=text -verify %s

namespace std {
template <class T> struct remove_reference { using type = T; };
template <class T> struct remove_reference<T &> { using type = T; };
template <class T> struct remove_reference<T &&> { using type = T; };

template <class T>
typename remove_reference<T>::type &&move(T &&value) {
  return static_cast<typename remove_reference<T>::type &&>(value);
}
} // namespace std

struct Owner {
  Owner() = default;
  Owner(Owner &&) {}

  Owner &operator=(this Owner &self, Owner &&other) {
    return self;
  }

  void use() const {}
};

void moveAssignmentMarksTheSource() {
  Owner target;
  Owner source;
  target = std::move(source); // expected-note {{Object 'source' is moved}}
  target.use();
  source.use(); // expected-warning {{Method called on moved-from object 'source'}}
                // expected-note@-1 {{Method called on moved-from object 'source'}}
}

void moveAssignmentResetsTheTarget() {
  Owner movedFrom;
  Owner target = std::move(movedFrom);
  Owner source;
  target = std::move(source);
  target.use();
}

void movingFromTheSourceTwiceWarns() {
  Owner firstTarget;
  Owner secondTarget;
  Owner source;
  firstTarget = std::move(source); // expected-note {{Object 'source' is moved}}
  secondTarget = std::move(source); // expected-warning {{Moved-from object 'source' is moved}}
                                    // expected-note@-1 {{Moved-from object 'source' is moved}}
}

void selfMoveAssignmentDoesNotMarkTheObject() {
  Owner object;
  object = std::move(object);
  object.use();
}
