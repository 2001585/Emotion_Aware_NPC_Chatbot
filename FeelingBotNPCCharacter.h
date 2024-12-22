#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "ChatWidget.h"
#include "Animation/AnimInstance.h"  // 추가됨
#include "UObject/UnrealType.h"     // 추가됨
#include "FeelingBotNPCCharacter.generated.h"

// 감정 타입 열거형 추가 - 올바른 위치
UENUM(BlueprintType)
enum class EEmotionType : uint8
{
    Idle     UMETA(DisplayName = "Idle"),
    Happy    UMETA(DisplayName = "Happy"),
    Sad      UMETA(DisplayName = "Sad"),
    Angry    UMETA(DisplayName = "Angry"),
    Fear     UMETA(DisplayName = "Fear"),
    Neutral  UMETA(DisplayName = "Neutral")
};

// 감정 응답 구조체 추가 - 올바른 위치
USTRUCT(BlueprintType)
struct FEmotionResponse
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionType EmotionType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResponseText;
};

UCLASS()
class FEELINGBOT_API AFeelingBotNPCCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AFeelingBotNPCCharacter();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CreateAndShowChatWidget();

    UFUNCTION()
    void OnMessageReceived(const FString& Message);

    UPROPERTY(EditDefaultsOnly, Category = "Mesh")
    class USkeletalMeshComponent* CharacterMesh;

    // 감정 상태 변경 함수
    UFUNCTION(BlueprintCallable, Category = "Emotion")
    void SetEmotionState(EEmotionType NewEmotion);

protected:
    virtual void BeginPlay() override;

private:
    void SendMessageToGeminiAPI(const FString& UserMessage);
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    FString ApiUrl;
    FString ApiKey;

    UPROPERTY()
    UChatWidget* ChatWidget;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UChatWidget> ChatWidgetClass;

    // 감정 애니메이션 몽타주 변수들
    UPROPERTY(EditDefaultsOnly, Category = "Emotions|Animations")
    class UAnimMontage* AngryMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Emotions|Animations")
    class UAnimMontage* HappyMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Emotions|Animations")
    class UAnimMontage* FearMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Emotions|Animations")
    class UAnimMontage* SadMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Emotions|Animations")
    class UAnimMontage* NeutralMontage;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // 몽타주 종료 시 호출될 함수
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
