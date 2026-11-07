#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProceduralUniverseGenerator.generated.h"

class UWorld;
class AActor;
class UStaticMesh;
class UMaterialInterface;
class UCurveFloat;

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest,
    Village,
    Ruins,
    River,
    Mountain,
    Cave,
    Cliffs,
    Plains,
    Swamp,
    Volcanic,
    Frozen,
    Crystal
};

USTRUCT(BlueprintType)
struct FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Moisture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<AActor>> FoliageActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<AActor>> PropActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<AActor>> StructureActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* LandscapeMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UMaterialInterface*> FoliageMaterials;
};

USTRUCT(BlueprintType)
struct FLandmarkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName LandmarkID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType RequiredBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMissionCritical;
};

USTRUCT(BlueprintType)
struct FGenerationParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ChunkCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NoiseOctaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoisePersistence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseLacunarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TreeDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RockDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GrassDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LandmarkDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnemyDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateVillage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateRuins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateRiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateCaves;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerationProgress, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGenerationComplete);

UCLASS(BlueprintType, Blueprintable)
class MULTIVERSEHOPPER_API UProceduralUniverseGenerator : public UObject
{
    GENERATED_BODY()

public:
    UProceduralUniverseGenerator();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void Initialize(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateWorld(const FGenerationParams& Params);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateChunk(int32 ChunkX, int32 ChunkY);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void ClearWorld();

    UFUNCTION(BlueprintPure, Category = "Generation")
    float GetHeightAtLocation(FVector Location) const;

    UFUNCTION(BlueprintPure, Category = "Generation")
    EBiomeType GetBiomeAtLocation(FVector Location) const;

    UPROPERTY(BlueprintAssignable, Category = "Generation")
    FOnGenerationProgress OnGenerationProgress;

    UPROPERTY(BlueprintAssignable, Category = "Generation")
    FOnGenerationComplete OnGenerationComplete;

protected:
    UPROPERTY()
    UWorld* World;

    UPROPERTY()
    FGenerationParams CurrentParams;

    UPROPERTY()
    TMap<EBiomeType, FBiomeData> BiomeDatabase;

    UPROPERTY()
    TArray<FLandmarkData> LandmarkDatabase;

    UPROPERTY()
    TArray<AActor*> GeneratedActors;

    UPROPERTY()
    int32 CurrentChunkX;

    UPROPERTY()
    int32 CurrentChunkY;

    void LoadBiomeDatabase();
    void LoadLandmarkDatabase();
    void GenerateTerrain();
    void GenerateBiomes();
    void GenerateFoliage();
    void GenerateStructures();
    void GenerateLandmarks();
    void GenerateEnemies();
    void GenerateMissionCritical();
    FVector GetChunkCenter(int32 ChunkX, int32 ChunkY) const;
    float CalculateNoise(float X, float Y) const;
    EBiomeType DetermineBiome(float Elevation, float Moisture, float Temperature) const;
    void SpawnActorInBiome(TSubclassOf<AActor> ActorClass, FVector Location, FRotator Rotation, float Scale);
};