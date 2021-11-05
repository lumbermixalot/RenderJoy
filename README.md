# Welcome To RenderJoy

RenderJoy is a clone of ShaderToy(https://www.shadertoy.com/) for O3DE.

*But, I don't know what's ShaderToy?* It's a very convenient sandbox to write multi-pass pixel shaders.

## Why?

1. Shaders in *ShaderToy* are written in *GLSL* (The WebGL variant). In O3DE shaders are written in **AZSL** (A superset of *HLSL*).  
**RenderJoy** provides the same conveniences as *ShaderToy*, but from the comfort of writing the shaders in **AZSL**.  
This means you can prototype or practice **AZSL** shader development without having to write C++ code. You can design really complex
multi-pass pipelines at runtime, only limited by your imagination.

2. The main renderer of O3DE is ATOM. ATOM is modular and data-driven, allowing developers to create custom render pipelines with ease.  
**RenderJoy** is an opportunity to showcase such flexibility.

---

## Installation & Setup

After cloning this repository, you'll find two subdirectories:  
*DemoProject/*  
*Gem/*  
  
At its core **RenderJoy** is a Gem, so at a minimum you'll only need to register the Gem, and enable it in your other game projects.  
  
*Optionally*, you can use the sample game project, **RenderJoyDemoProject**,  provided inside the *DemoProject/* folder. This game project shows some examples ported from *ShaderToy*, like being able to play *PacMan* (https://www.shadertoy.com/view/Ms3XWN) :grin:  
  
  
For the sake of simplicity, the following instructions assume you have installed *O3DE* at *"D:\GIT\o3de\"*,  
and **RenderJoy** was cloned in *"E:\Other\RenderJoy\"*.

1. Register the Gem.  
`cd D:\GIT\o3de`  (all the commands listed below assume this is your working directory).  
`.\scripts\o3de.bat register -gp E:\Other\RenderJoy\Gem`  
  
2. (Optional) Register the provided demo project, **RenderJoyDemoProject**.  
`.\scripts\o3de.bat register -pp E:\Other\RenderJoy\DemoProject`  
  
3. Enable the **RenderJoy** Gem.  
Assuming you want to use the provided demo project, you can use either one of these two commands.  
`.\scripts\o3de.bat enable-gem -pp E:\Other\RenderJoy\DemoProject -gp E:\Other\RenderJoy\Gem`  
OR  
`.\scripts\o3de.bat enable-gem -pn RenderJoyDemoProject -gn RenderJoy`  
Assuming you **don't** want to use the provided demo project, and your particular game project is located at:  
*"D:\MyProjects\SecretProject\"*:  
`.\scripts\o3de.bat enable-gem -pp D:\MyProjects\SecretProject -gp E:\Other\RenderJoy\Gem`  
OR  
`.\scripts\o3de.bat enable-gem -pp D:\MyProjects\SecretProject -gn RenderJoy`  
  
4. Configure the game project.  
`cd E:\Other\RenderJoy\DemoProject`  
or, if you are using your own game project  
`cd D:\MyProjects\SecretProject`  
The following *cmake* configuration command assumes your 3rdParty folder is located at: *"D:\LY3RDPARTY\lyengine\3rdParty\"*  
`cmake -B build -S . -G "Visual Studio 16 2019" -DLY_3RDPARTY_PATH=D:\LY3RDPARTY\lyengine\3rdParty`  
OR  
If you use the default 3rdParty folder, then this command should suffice:  
`cmake -B build -S . -G "Visual Studio 16 2019"`  
  
5. Build the game project.  
Compile the *Editor* target using the generated solution at *'.\build\RenderJoyDemoProject.sln'*, or equivalent if you chose another game project.  


