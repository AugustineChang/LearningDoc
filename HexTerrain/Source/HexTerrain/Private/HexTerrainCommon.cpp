#include "HexTerrainCommon.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

FHexTerrainNoiser::FHexTerrainNoiser()
	:bIsValid(false)
{}

void FHexTerrainNoiser::InitNoiser(const FString& InNoiseTexturePath)
{
	TArray<uint8> TextureBinData;
	FFileHelper::LoadFileToArray(TextureBinData, *(FPaths::ProjectDir() / InNoiseTexturePath));
	CreateTextureFromData(NoiseTexture, TextureBinData, EImageFormat::PNG);

	GenerateRandomCache();

	bIsValid = true;
}

void FHexTerrainNoiser::GenerateRandomCache()
{
	static int32 RandomCacheSize = 512;
	RandomCache.Empty(RandomCacheSize);
	RandomCache.AddDefaulted(RandomCacheSize);
	for (int32 Y = 0; Y < RandomCacheSize; ++Y)
	{
		RandomCache[Y].AddDefaulted(RandomCacheSize);
		for (int32 X = 0; X < RandomCacheSize; ++X)
		{
			FVector4& OneRand = RandomCache[Y][X];
			OneRand.X = FMath::FRand();
			OneRand.Y = FMath::FRand();
			OneRand.Z = FMath::FRand();
			OneRand.W = FMath::FRand();
		}
	}
}

FVector4 FHexTerrainNoiser::GetRandomValueByPosition(const FVector& InVertex) const
{
	if (RandomCache.IsEmpty())
		return FVector::ZeroVector;

	int32 X = FMath::RoundToInt(InVertex.X * 0.1);
	int32 Y = FMath::RoundToInt(InVertex.Y * 0.1);

	int32 CacheSizeX = RandomCache[0].Num();
	int32 CacheSizeY = RandomCache.Num();

	int32 CacheTillingStartX = FMath::FloorToInt(float(X) / CacheSizeX) * CacheSizeX;
	int32 CacheTillingStartY = FMath::FloorToInt(float(Y) / CacheSizeY) * CacheSizeY;

	X = X - CacheTillingStartX;
	Y = Y - CacheTillingStartY;

	return RandomCache[Y][X];
}

FLinearColor FHexTerrainNoiser::SampleTextureBilinear(const FVector& SamplePos, bool bRatio) const
{
	int32 SizeY = NoiseTexture.Num();
	int32 SizeX = NoiseTexture[0].Num();

	float SamplePosX = bRatio ? SamplePos.X * SizeX : SamplePos.X;
	float SamplePosY = bRatio ? SamplePos.Y * SizeY : SamplePos.Y;
	int32 SampleX = FMath::FloorToInt(SamplePosX);
	int32 SampleY = FMath::FloorToInt(SamplePosY);
	float RatioX = SamplePosX - SampleX;
	float RatioY = SamplePosY - SampleY;

	if (SampleX < 0)
		SampleX += SizeX * (1 - SampleX / SizeX);
	if (SampleY < 0)
		SampleY += SizeY * (1 - SampleY / SizeY);

	SampleX = SampleX % SizeX;
	SampleY = SampleY % SizeY;
	int32 NextSampleX = (SampleX + 1) % SizeX;
	int32 NextSampleY = (SampleY + 1) % SizeY;

	const FColor& LTColor = NoiseTexture[SampleY][SampleX];
	const FColor& RTColor = NoiseTexture[SampleY][NextSampleX];
	const FColor& LDColor = NoiseTexture[NextSampleY][SampleX];
	const FColor& RDColor = NoiseTexture[NextSampleY][NextSampleX];

	FLinearColor TColor = FMath::Lerp(LTColor.ReinterpretAsLinear(), RTColor.ReinterpretAsLinear(), RatioX);
	FLinearColor DColor = FMath::Lerp(LDColor.ReinterpretAsLinear(), RDColor.ReinterpretAsLinear(), RatioX);

	return FMath::Lerp(TColor, DColor, RatioY);
}

FLinearColor FHexTerrainNoiser::SampleTextureNearest(int32 SampleX, int32 SampleY) const
{
	int32 SizeY = NoiseTexture.Num();
	int32 SizeX = NoiseTexture[0].Num();

	if (SampleX < 0)
		SampleX += SizeX * (1 - SampleX / SizeX);
	if (SampleY < 0)
		SampleY += SizeY * (1 - SampleY / SizeY);

	SampleX = SampleX % SizeX;
	SampleY = SampleY % SizeY;

	return NoiseTexture[SampleY][SampleX].ReinterpretAsLinear();
}

void FHexTerrainNoiser::CreateTextureFromData(TArray<TArray<FColor>>& OutTexture, const TArray<uint8>& InBineryData, EImageFormat InFormat)
{
	if (InBineryData.IsEmpty())
	{
		return;
	}

	static IImageWrapperModule* ImageWrapperModule = FModuleManager::LoadModulePtr<IImageWrapperModule>("ImageWrapper");
	if (ImageWrapperModule == nullptr)
	{
		return;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule->CreateImageWrapper(InFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(InBineryData.GetData(), InBineryData.Num()))
	{
		return;
	}

	TArray<uint8> Uncompressed;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, Uncompressed))
	{
		return;
	}

	OutTexture.Empty(ImageWrapper->GetHeight());
	OutTexture.AddDefaulted(ImageWrapper->GetHeight());
	for (int32 Y = 0; Y < ImageWrapper->GetHeight(); ++Y)
	{
		OutTexture[Y].AddUninitialized(ImageWrapper->GetWidth());
		for (int32 X = 0; X < ImageWrapper->GetWidth(); ++X)
		{
			int32 Index = X + Y * ImageWrapper->GetWidth();

			OutTexture[Y][X].B = Uncompressed[Index * 4];
			OutTexture[Y][X].G = Uncompressed[Index * 4 + 1];
			OutTexture[Y][X].R = Uncompressed[Index * 4 + 2];
			OutTexture[Y][X].A = Uncompressed[Index * 4 + 3];
		}
	}
}