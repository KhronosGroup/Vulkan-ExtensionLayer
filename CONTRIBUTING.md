# How to Contribute to Vulkan Source Repositories

## **The Repository**

The source code for The Vulkan-ExtensionLayer components is sponsored by Khronos.
* [Khronos Vulkan-ExtensionLayer](https://github.com/KhronosGroup/Vulkan-ExtensionLayer)

### **Coding Conventions and Formatting**
* Use the **[Google style guide](https://google.github.io/styleguide/cppguide.html)** for source code with the following exceptions:
    * The column limit is 132 (as opposed to the default value 80). The clang-format tool will handle this. See below.
    * The indent is 4 spaces instead of the default 2 spaces. Access modifier (e.g. `public:`) is indented 2 spaces instead of the
      default 1 space. Again, the clang-format tool will handle this.
    * The C++ file extension is `*.cpp` instead of the default `*.cc`.
    * If you can justify a reason for violating a rule in the guidelines, then you are free to do so. Be prepared to defend your
decision during code review. This should be used responsibly. An example of a bad reason is "I don't like that rule." An example of
a good reason is "This violates the style guide, but it improves type safety."

* Run **clang-format** on your changes to maintain consistent formatting
    * There are `.clang-format` files present in the repository to define clang-format settings
      which are found and used automatically by clang-format.
    * **clang-format** binaries are available from the LLVM orginization, here: [LLVM](https://clang.llvm.org/). Our CI system (Travis-CI)
       currently uses clang-format version 7.0.0 to check that the lines of code you have changed are formatted properly. It is
    recommended that you use the same version to format your code prior to submission.
    * A sample git workflow may look like:

>        # Make changes to the source.
>        $ git add -u .
>        $ git clang-format --style=file
>        # Check to see if clang-format made any changes and if they are OK.
>        $ git add -u .
>        $ git commit

### **Contributor License Agreement (CLA)**

You will be prompted with a one-time "click-through" CLA dialog as part of submitting your pull request
or other contribution to GitHub.

### **License and Copyrights**

All contributions made to the Vulkan-ExtensionLayer repository are Khronos branded and as such,
any new files need to have the Khronos license (Apache 2.0 style) and copyright included.
Please see an existing file in this repository for an example.

You can include your individual copyright after any existing copyrights.
