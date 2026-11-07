#include "Systems/ProceduralUniverseGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Enemy/BaseEnemy.h"
#include "Enemy/HeavyEnemy.h"

UProceduralUniverseGenerator::UProceduralUniverseGenerator()
{
    World = nullptr;
    CurrentChunkX = 0;
    CurrentChunkY = 0;

    CurrentParams.Seed = 12345;
    CurrentParams.WorldSize = 10000.0f;
    CurrentParams.ChunkCount = 16;
    CurrentParams.ChunkSize = 2000.0f;
    CurrentParams.NoiseScale = 0.001f;
    CurrentParams.NoiseOctaves = 6;
    CurrentParams.NoisePersistence = 0.5f;
    CurrentParams.NoiseLacunarity = 2.0f;
    CurrentParams.TreeDensity = 0.02f;
    CurrentParams.RockDensity = 0.005f;
    CurrentParams.GrassDensity = 0.1f;
    CurrentParams.LandmarkDensity = 0.001f;
    CurrentParams.EnemyDensity = 0.0005f;
    CurrentParams.bGenerateVillage = true;
    CurrentParams.bGenerateRuins = true;
    CurrentParams.bGenerateRiver = true;
    CurrentParams.bGenerateCaves = false;

    LoadBiomeDatabase();
    LoadLandmarkDatabase();
}

void UProceduralUniverseGenerator::Initialize(UWorld* InWorld)
{
    World = InWorld;
}

void UProceduralUniverseGenerator::LoadBiomeDatabase()
{
    FBiomeData Forest;
    Forest.BiomeType = EBiomeType::Forest;
    Forest.BiomeName = TEXT("Ancient Forest");
    Forest.Weight = 0.35f;
    Forest.MinElevation = 0.2f;
    Forest.MaxElevation = 0.7f;
    Forest.Moisture = 0.7f;
    Forest.Temperature = 0.6f;
    BiomeDatabase.Add(Forest.BiomeType, Forest);

    FBiomeData Village;
    Village.BiomeType = EBiomeType::Village;
    Village.BiomeName = TEXT("Wooden Village");
    Village.Weight = 0.05f;
    Village.MinElevation = 0.3f;
    Village.MaxElevation = 0.5f;
    Village.Moisture = 0.5f;
    Village.Temperature = 0.55f;
    BiomeDatabase.Add(Village.BiomeType, Village);

    FBiomeData Ruins;
    Ruins.BiomeType = EBiomeType::Ruins;
    Ruins.BiomeName = TEXT("Ancient Ruins");
    Ruins.Weight = 0.08f;
    Ruins.MinElevation = 0.4f;
    Ruins.MaxElevation = 0.8f;
    Ruins.Moisture = 0.3f;
    Ruins.Temperature = 0.5f;
    BiomeDatabase.Add(Ruins.BiomeType, Ruins);

    FBiomeData River;
    River.BiomeType = EBiomeType::River;
    River.BiomeName = TEXT("River Valley");
    River.Weight = 0.1f;
    River.MinElevation = 0.1f;
    River.MaxElevation = 0.3f;
    River.Moisture = 1.0f;
    River.Temperature = 0.6f;
    BiomeDatabase.Add(River.BiomeType, River);

    FBiomeData Mountain;
    Mountain.BiomeType = EBiomeType::Mountain;
    Mountain.BiomeName = TEXT("Mountain Peaks");
    Mountain.Weight = 0.12f;
    Mountain.MinElevation = 0.7f;
    Mountain.MaxElevation = 1.0f;
    Mountain.Moisture = 0.4f;
    Mountain.Temperature = 0.3f;
    BiomeDatabase.Add(Mountain.BiomeType, Mountain);

    FBiomeData Cliffs;
    Cliffs.BiomeType = EBiomeType::Cliffs;
    Cliffs.BiomeName = TEXT("Rocky Cliffs");
    Cliffs.Weight = 0.1f;
    Cliffs.MinElevation = 0.5f;
    Cliffs.MaxElevation = 0.9f;
    Cliffs.Moisture = 0.2f;
    Cliffs.Temperature = 0.45f;
    BiomeDatabase.Add(Cliffs.BiomeType, Cliffs);

    FBiomeData Plains;
    Plains.BiomeType = EBiomeType::Plains;
    Plains.BiomeName = TEXT("Grassy Plains");
    Plains.Weight = 0.2f;
    Plains.MinElevation = 0.25f;
    Plains.MaxElevation = 0.55f;
    Plains.Moisture = 0.5f;
    Plains.Temperature = 0.65f;
    BiomeDatabase.Add(Plains.BiomeType, Plains);
}

void UProceduralUniverseGenerator::LoadLandmarkDatabase()
{
    FLandmarkData VillageCenter;
    VillageCenter.LandmarkID = TEXT("Landmark_VillageCenter");
    VillageCenter.Location = FVector(-1500.0f, 200.0f, 100.0f);
    VillageCenter.Rotation = FRotator(0, 0, 0);
    VillageCenter.Scale = 1.0f;
    VillageCenter.RequiredBiome = EBiomeType::Village;
    VillageCenter.bIsMissionCritical = true;
    LandmarkDatabase.Add(VillageCenter);

    FLandmarkData RuinsEntrance;
    RuinsEntrance.LandmarkID = TEXT("Landmark_RuinsEntrance");
    RuinsEntrance.Location = FVector(2500.0f, 500.0f, 150.0f);
    RuinsEntrance.Rotation = FRotator(0, 90, 0);
    RuinsEntrance.Scale = 1.2f;
    RuinsEntrance.RequiredBiome = EBiomeType::Ruins;
    RuinsEntrance.bIsMissionCritical = true;
    LandmarkDatabase.Add(RuinsEntrance);

    FLandmarkData EnemyCamp;
    EnemyCamp.LandmarkID = TEXT("Landmark_EnemyCamp");
    EnemyCamp.Location = FVector(3500.0f, 0.0f, 150.0f);
    EnemyCamp.Rotation = FRotator(0, 180, 0);
    EnemyCamp.Scale = 1.0f;
    EnemyCamp.RequiredBiome = EBiomeType::Ruins;
    EnemyCamp.bIsMissionCritical = true;
    LandmarkDatabase.Add(EnemyCamp);
}

void UProceduralUniverseGenerator::GenerateWorld(const FGenerationParams& Params)
{
    CurrentParams = Params;
    ClearWorld();

    FMath::RandInit(Params.Seed);

    GenerateTerrain();
    GenerateBiomes();
    GenerateFoliage();
    GenerateStructures();
    GenerateLandmarks();
    GenerateEnemies();
    GenerateMissionCritical();

    OnGenerationComplete.Broadcast();
}

void UProceduralUniverseGenerator::GenerateTerrain()
{
    if (!World) return;

    for (int32 X = -CurrentParams.ChunkCount / 2; X < CurrentParams.ChunkCount / 2; ++X)
    {
        for (int32 Y = -CurrentParams.ChunkCount / 2; Y < CurrentParams.ChunkCount / 2; ++Y)
        {
            FVector ChunkCenter = GetChunkCenter(X, Y);
            float Height = CalculateNoise(ChunkCenter.X, ChunkCenter.Y) * 500.0f;

            // In a real implementation, this would create landscape components
            // For now, we'll use the flat terrain and place actors
        }
    }
}

void UProceduralUniverseGenerator::GenerateBiomes()
{
}

void UProceduralUniverseGenerator::GenerateFoliage()
{
    if (!World) return;

    for (int32 X = -CurrentParams.ChunkCount / 2; X < CurrentParams.ChunkCount / 2; ++X)
    {
        for (int32 Y = -CurrentParams.ChunkCount / 2; Y < CurrentParams.ChunkCount / 2; ++Y)
        {
            FVector ChunkCenter = GetChunkCenter(X, Y);
            float Noise = CalculateNoise(ChunkCenter.X + 10000, ChunkCenter.Y + 10000);

            int32 TreeCount = FMath::RoundToInt(CurrentParams.TreeDensity * CurrentParams.ChunkSize * CurrentParams.ChunkSize * 0.0001f);
            for (int32 i = 0; i < TreeCount; ++i)
            {
                FVector TreeLoc = ChunkCenter + FVector(
                    FMath::FRandRange(-CurrentParams.ChunkSize * 0.5f, CurrentParams.ChunkSize * 0.5f),
                    FMath::FRandRange(-CurrentParams.ChunkSize * 0.5f, CurrentParams.ChunkSize * 0.5f),
                    0
                );
                TreeLoc.Z = GetHeightAtLocation(TreeLoc);

                // Spawn tree placeholder (would use actual foliage system in production)
            }
        }
    }
}

void UProceduralUniverseGenerator::GenerateStructures()
{
    if (!World || !CurrentParams.bGenerateVillage || !CurrentParams.bGenerateRuins) return;

    // Village structures near village center
    FVector VillageLoc = FVector(-1500.0f, 200.0f, 100.0f);
    // Would spawn village buildings here

    // Ruins structures
    FVector RuinsLoc = FVector(2500.0f, 500.0f, 150.0f);
    // Would spawn ruin pieces here
}

void UProceduralUniverseGenerator::GenerateLandmarks()
{
    if (!World) return;

    for (const FLandmarkData& Landmark : LandmarkDatabase)
    {
        EBiomeType BiomeAtLoc = GetBiomeAtLocation(Landmark.Location);
        if (BiomeAtLoc == Landmark.RequiredBiome || Landmark.RequiredBiome == EBiomeType::Forest)
        {
            // Spawn landmark actor
            // In production, this would use the actual actor classes
        }
    }
}

void UProceduralUniverseGenerator::GenerateEnemies()
{
    if (!World) return;

    // Basic enemies near ruins
    TArray<FVector> EnemyPositions = {
        FVector(3000.0f, 200.0f, 150.0f),
        FVector(3100.0f, 150.0f, 150.0f),
        FVector(2900.0f, 250.0f, 150.0f),
        FVector(3200.0f, 100.0f, 150.0f),
        FVector(2800.0f, 300.0f, 150.0f)
    };

    for (const FVector& Pos : EnemyPositions)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ABaseEnemy* Enemy = World->SpawnActor<ABaseEnemy>(ABaseEnemy::StaticClass(), Pos, FRotator(0, FMath::FRandRange(0, 360), 0), SpawnParams);
        if (Enemy)
        {
            Enemy->EnemyType = EEnemyType::BasicMelee;
            Enemy->MaxHealth = 80.0f;
            Enemy->CurrentHealth = 80.0f;
            Enemy->PatrolSpeed = 200.0f;
            Enemy->ChaseSpeed = 400.0f;
            Enemy->AttackRange = 180.0f;
            Enemy->CombatRange = 600.0f;
            GeneratedActors.Add(Enemy);
        }
    }

    // Heavy enemy (boss) at enemy camp
    FVector BossPos = FVector(3500.0f, 0.0f, 150.0f);
    FActorSpawnParameters BossSpawnParams;
    BossSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AHeavyEnemy* Boss = World->SpawnActor<AHeavyEnemy>(AHeavyEnemy::StaticClass(), BossPos, FRotator(0, 180, 0), BossSpawnParams);
    if (Boss)
    {
        Boss->MaxHealth = 400.0f;
        Boss->CurrentHealth = 400.0f;
        Boss->PatrolSpeed = 150.0f;
        Boss->ChaseSpeed = 300.0f;
        Boss->AttackRange = 350.0f;
        Boss->CombatRange = 1000.0f;
        Boss->GroundSlamRadius = 450.0f;
        Boss->GroundSlamDamage = 60.0f;
        GeneratedActors.Add(Boss);
    }
}

void UProceduralUniverseGenerator::GenerateMissionCritical()
{
    // Player start
    if (World)
    {
        FVector PlayerStartLoc = FVector(-1500.0f, 200.0f, 150.0f);
        FActorSpawnParameters SpawnParams;
        World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), PlayerStartLoc, FRotator(0, 0, 0), SpawnParams);
    }
}

void UProceduralUniverseGenerator::GenerateChunk(int32 ChunkX, int32 ChunkY)
{
    CurrentChunkX = ChunkX;
    CurrentChunkY = ChunkY;
}

void UProceduralUniverseGenerator::ClearWorld()
{
    for (AActor* Actor : GeneratedActors)
    {
        if (Actor && Actor->IsValidLowLevel())
        {
            Actor->Destroy();
        }
    }
    GeneratedActors.Empty();
}

float UProceduralUniverseGenerator::GetHeightAtLocation(FVector Location) const
{
    return CalculateNoise(Location.X, Location.Y) * 500.0f;
}

EBiomeType UProceduralUniverseGenerator::GetBiomeAtLocation(FVector Location) const
{
    float Elevation = FMath::Abs(CalculateNoise(Location.X, Location.Y));
    float Moisture = CalculateNoise(Location.X + 5000, Location.Y + 5000);
    float Temperature = CalculateNoise(Location.X - 5000, Location.Y - 5000);

    Moisture = (Moisture + 1.0f) * 0.5f;
    Temperature = (Temperature + 1.0f) * 0.5f;
    Elevation = (Elevation + 1.0f) * 0.5f;

    return DetermineBiome(Elevation, Moisture, Temperature);
}

FVector UProceduralUniverseGenerator::GetChunkCenter(int32 ChunkX, int32 ChunkY) const
{
    return FVector(
        ChunkX * CurrentParams.ChunkSize,
        ChunkY * CurrentParams.ChunkSize,
        0.0f
    );
}

float UProceduralUniverseGenerator::CalculateNoise(float X, float Y) const
{
    float Noise = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = CurrentParams.NoiseScale;
    float MaxAmplitude = 0.0f;

    for (int32 i = 0; i < CurrentParams.NoiseOctaves; ++i)
    {
        float SampleX = X * Frequency;
        float SampleY = Y * Frequency;

        float Value = FMath::PerlinNoise2D(FVector2D(SampleX, SampleY));
        Noise += Value * Amplitude;

        MaxAmplitude += Amplitude;
        Amplitude *= CurrentParams.NoisePersistence;
        Frequency *= CurrentParams.NoiseLacunarity;
    }

    return MaxAmplitude > 0 ? Noise / MaxAmplitude : 0.0f;
}

EBiomeType UProceduralUniverseGenerator::DetermineBiome(float Elevation, float Moisture, float Temperature) const
{
    if (Elevation > 0.75f)
    {
        return EBiomeType::Mountain;
    }
    else if (Elevation > 0.6f && Moisture < 0.3f)
    {
        return EBiomeType::Cliffs;
    }
    else if (Moisture > 0.8f && Elevation < 0.35f)
    {
        return EBiomeType::River;
    }
    else if (Elevation > 0.35f && Elevation < 0.6f && Temperature < 0.5f)
    {
        return EBiomeType::Ruins;
    }
    else if (Elevation > 0.25f && Elevation < 0.5f && Temperature > 0.5f && Moisture > 0.4f && Moisture < 0.6f)
    {
        return EBiomeType::Village;
    }
    else if (Moisture > 0.6f && Temperature > 0.5f)
    {
        return EBiomeType::Forest;
    }
    else
    {
        return EBiomeType::Plains;
    }
}

void UProceduralUniverseGenerator::SpawnActorInBiome(TSubclassOf<AActor> ActorClass, FVector Location, FRotator Rotation, float Scale)
{
    if (!World || !ActorClass) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* Actor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
    if (Actor)
    {
        Actor->SetActorScale3D(FVector(Scale));
        GeneratedActors.Add(Actor);
    }
}