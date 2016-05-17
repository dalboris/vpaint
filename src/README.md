
# Source code structure

The source code is organized as follow:

app/
libs/
  Lib1/
  Lib2/
third/
  Lib3/
  Lib4/

app/ is as minimalist as possible. Its only source file is main.cpp, allocating an `Application` object implemented in libs/VPaint/Application.{h|cpp}. In most cases (e.g.: bug fixes, new features), you should not modify files in app/.

libs/ contains the libraries developed for the project, and is where most of the development should take place. Each library is compiled statically and can be used by other libraries. Cyclic dependencies between libraries are not allowed. For now, most of the code is in a huge library called VPaint (i.e., most of the code is in the src/libs/VPaint/ folder), but the long-term objective is to refactor it into smaller and more independent libraries.

third/ contains the third-party libraries used by the project and shipped with the distribution. They are also compiled statically. The only third-party libraries that the project depends on, but are not shiped with the distribution, are OpenGL and Qt, which are linked dynamically.


# Object lifetime management

By default, raw pointers are assumed to be non-null and non-owning. In other words, they point to a valid observed object whose lifetime is guaranteed by client code to exceed the lifetime of the observer.

One exception to this rule are QWidgets and QLayouts. It is okay for QWidget subclasses to store raw pointers of their child widgets and layouts, even though they are "owning". The reason is that these QObjects are automatically parented and deleted by Qt. Code should document which widgets/layouts are children, and which are observed.

However, all other QObjects (i.e., those who do not inherit from QWidget or QLayout) should be made parent-less, and should be managed with standard C++11 memory management techniques (e.g., std::unique_ptr), instead of the Qt object ownership model. The rationale is that this makes pointer semantics self-documented:

 - std::unique_ptr for non-null, owning pointers (to QObjects or other).
 - raw pointers for non-null, non-owning pointers (to QObjects or other).
 - QPointer for possibly-null, non-owning pointers to QObjects (discouraged but okay-ish).

Another rationale is that with std::unique_ptr (instead of having a parent QObject), then owned objects are deleted in ~MyQObjectSubclass() in reversed declared order, instead of being deleted in ~QObject() in a non-specified order, which can be problematic if one owned object should outlive a sibling.

In other words, in this code base, QObjects are primarily used to take advantage of the signal/slot mechanism, but not to take advantage of the Qt memory management mechanism (which is convenient but has some disadvantages, and makes ownership intent less readable). 

Any other usage/semantics of raw pointers should be clearly documented, and avoided whenever possible.
