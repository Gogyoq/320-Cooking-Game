
# CookingMamaClone

## Requirements
- Windows
- Visual Studio 2022 with **"Desktop development with C++"** workload

## How to Run

1. Open `CookingMamaClone.sln` in Visual Studio 2022.
2. At the top, set:
   - Configuration: **Debug**
   - Platform: **x64**
3. Build and run the project (Ctrl + F5).

On the first run, Visual Studio creates the x64/Debug folder, so the build may fail.
This is normal.
Simply run it a second time and it will succeed.

### If you get DLL / library errors

If the compiler or the program complains about missing SDL libraries (e.g. `SDL3.dll`, `SDL_ttf.dll`, `SDL_image.dll`):

1. Copy these files from:
   - `frameworks/lib/SDL3.dll`
   - `frameworks/lib/SDL_ttf.dll`
   - `frameworks/lib/SDL_image.dll`
2. Paste them into your build output:
   - `x64/Debug/`
   (same folder where the `.exe` is)

Then run the project again.
