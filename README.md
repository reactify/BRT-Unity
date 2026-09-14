# BRT Unity

BRT Unity is a Unity wrapper for the **[Binaural Rendering Toolbox](https://grupodiana.github.io/BRT-Documentation) (BRT)**.

It exposes BRT functionality to Unity through a standard Unity package interface, allowing BRT-based binaural rendering to be used from Unity projects.

## Adding the package to a Unity project

The package can be added to a Unity project through Unity’s Package Manager.

1. Open the Unity project.
2. Open **Window > Package Manager**.
3. Click the **+** button.
4. Select **Add package from git URL...**.
5. Enter the package URL:

   ```text
   https://github.com/reactify/BRT-Unity.git?path=/brt-unity-package
   ```

6. Click **Add**.

Unity will download the package and add it to the project.

## Sample

The package includes a sample scene demonstrating basic usage.

After adding the package to a Unity project, the sample can be imported from Package Manager:

1. Open **Window > Package Manager**.
2. Select **BRT Unity** from the installed packages list.
3. Open the **Samples** section.
4. Click **Import** next to the sample.

The imported sample can then be opened from the project’s `Assets/Samples` directory.

## Building from source

The repository contains both the Unity package and the native plugin source.

```text
native/
  Native plugin source and build setup.

brt-unity-package/
  Unity package contents.
```

Build the native plugin using the build setup provided in the `native` directory.

After building, the local package can be added to a Unity project through Package Manager:

1. Open **Window > Package Manager**.
2. Click the **+** button.
3. Select **Add package from disk...**.
4. Select the package manifest:

   ```text
   brt-unity-package/package.json
   ```

Unity will add the local package to the project.

## Requirements

- Unity with Package Manager support.
- A supported native build toolchain for the target platform.
- The platform-specific build tools required by the native plugin.

## Acknowledgements
This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement No. 101017743.
