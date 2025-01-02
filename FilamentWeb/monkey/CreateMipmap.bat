::Create mipmaps for base color
D:\Projects\Filament\FilamentBin\bin\mipgen.exe albedo.png albedo.ktx
D:\Projects\Filament\FilamentBin\bin\mipgen.exe --compression=uastc albedo.png albedo.ktx2

::Create mipmaps for the normal map and a compressed variant.
D:\Projects\Filament\FilamentBin\bin\mipgen.exe  --strip-alpha --kernel=NORMALS --linear normal.png normal.ktx
D:\Projects\Filament\FilamentBin\bin\mipgen.exe  --strip-alpha --kernel=NORMALS --linear --compression=uastc_normals normal.png normal.ktx2

::Create mipmaps for the single-component roughness map and a compressed variant.
D:\Projects\Filament\FilamentBin\bin\mipgen.exe  --grayscale --linear roughness.png roughness.ktx
D:\Projects\Filament\FilamentBin\bin\mipgen.exe  --grayscale --linear --compression=uastc roughness.png roughness.ktx2

::Create mipmaps for the single-component metallic map and a compressed variant.
D:\Projects\Filament\FilamentBin\bin\mipgen.exe  --grayscale --linear metallic.png metallic.ktx
D:\Projects\Filament\FilamentBin\bin\mipgen.exe  --grayscale --linear --compression=uastc metallic.png metallic.ktx2

::Create mipmaps for the single-component occlusion map and a compressed variant.
D:\Projects\Filament\FilamentBin\bin\mipgen.exe  --grayscale --linear ao.png ao.ktx
D:\Projects\Filament\FilamentBin\bin\mipgen.exe  --grayscale --linear --compression=uastc ao.png ao.ktx2