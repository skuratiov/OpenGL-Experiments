# OpenGL Experiments

A collection of small demo projects and experiments using **OpenGL with
C/C++**, developed primarily in **Microsoft Visual Studio**.

This repository is used as a playground for testing graphics techniques,
shaders, textures, and rendering effects.

------------------------------------------------------------------------

## 📌 Overview

This repository contains standalone demo projects that demonstrate:

-   OpenGL context creation and basic rendering
-   GLSL shader compilation and usage
-   Texture handling and bump/normal mapping
-   Text rendering using FreeType
-   Low-level graphics pipeline experiments

Each project focuses on a specific graphics feature or rendering
technique.

------------------------------------------------------------------------

## 🗂️ Project Structure

Current subprojects include:

    GLExp_Boilerplate/   — Base OpenGL Visual Studio project template  
    GLExp_BumpMapping/   — Bump mapping / normal mapping demo  
    GLExp_Freetype/      — Text rendering using FreeType  

> Note: Folder names may change as new experiments are added.

------------------------------------------------------------------------

## 🛠️ Requirements

To build and run the projects, you will need:

-   Windows OS
-   Microsoft Visual Studio (2019 / 2022 recommended)
-   C++ Desktop Development workload
-   OpenGL 4.5+ compatible GPU and drivers
-   GLEW / GLM (included or configured per project)
-   FreeType (for text rendering demo)

Basic knowledge of OpenGL and GLSL is recommended.

------------------------------------------------------------------------

## 🔧 Build Instructions (Visual Studio)

1.  Clone the repository:

``` bash
git clone https://github.com/skuratiov/OpenGL-Experiments.git
```

2.  Open the desired project folder.

3.  Open the `.sln` file in Visual Studio.

4.  Make sure the platform is set to `x64` or `Win32`.

5.  Select **Debug** or **Release** configuration.

6.  Press **F5** or click **Run**.

Visual Studio will automatically build and launch the demo.

------------------------------------------------------------------------

## ▶️ Running the Demos

Each subproject contains its own Visual Studio solution.

After opening the solution:

-   Set the startup project (if needed)
-   Build the solution
-   Run using **F5**

The executable will be generated in:

 {SolutionFolder}/bin/

inside the project folder.

------------------------------------------------------------------------

## 🧠 Learning Goals

These experiments are designed to:

-   Explore modern OpenGL rendering pipelines
-   Improve shader programming skills
-   Practice shader programming (GLSL)
-   Understand low-level GPU interaction
-   Learn graphics performance optimization

------------------------------------------------------------------------

## ✨ Features

Depending on the demo, features may include:

-   Visual Studio project templates
-   Bump / normal mapping
-   Dynamic lighting
-   Font rendering via FreeType
-   Minimal external dependencies
-   Clean and modular code structure

------------------------------------------------------------------------

## 📌 Project Status

This repository contains experimental and prototype-level projects.

Some demos may be incomplete or subject to refactoring.

------------------------------------------------------------------------

## 📜 License

This project is licensed under the **MIT License**, unless stated
otherwise in individual folders.

------------------------------------------------------------------------

## 👤 Author

**Sergei Kuratiov**

GitHub: https://github.com/skuratiov

Linkedin: https://www.linkedin.com/in/sergei-kuratiov-32212010a/

------------------------------------------------------------------------

## 🤝 Contributions

Suggestions, bug reports, and pull requests are welcome.

Feel free to fork and experiment.
