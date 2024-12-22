#include "FeelingBotNPCCharacter.h"
#include "Json.h"
#include "Kismet/GameplayStatics.h"

AFeelingBotNPCCharacter::AFeelingBotNPCCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 메시 컴포넌트 생성 및 설정
    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
    CharacterMesh->SetupAttachment(RootComponent);

    // 스켈레탈 메시 에셋 설정
    // 스켈레탈 메시 경로 수정
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/MC_Sample/Demo/Characters/MCUE5/Meshes/SKM_MCUE5"));
    if (MeshAsset.Succeeded())
    {
        UE_LOG(LogTemp, Warning, TEXT("Skeletal Mesh found successfully"));
        CharacterMesh->SetSkeletalMesh(MeshAsset.Object);
        CharacterMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        CharacterMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find Skeletal Mesh"));
    }

    // 애니메이션 블루프린트 설정
    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(TEXT("/Game/feelbot/ABP_FeelingBotNPC.ABP_FeelingBotNPC_C"));
    if (AnimBP.Succeeded())
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Blueprint found successfully"));
        CharacterMesh->SetAnimInstanceClass(AnimBP.Class);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find Animation Blueprint"));
    }

    ApiUrl = "LLM";
    ApiKey = "*********************************";

}

void AFeelingBotNPCCharacter::BeginPlay()
{
    Super::BeginPlay();
    CreateAndShowChatWidget();
}

void AFeelingBotNPCCharacter::CreateAndShowChatWidget()
{
    if (ChatWidgetClass)
    {
        ChatWidget = CreateWidget<UChatWidget>(GetWorld(), ChatWidgetClass);
        if (ChatWidget)
        {
            ChatWidget->AddToViewport();

            // 델리게이트 연결 전 로그
            UE_LOG(LogTemp, Warning, TEXT("Attempting to bind OnMessageSent delegate"));

            ChatWidget->OnMessageSent.AddDynamic(this, &AFeelingBotNPCCharacter::OnMessageReceived);

            // 델리게이트 연결 후 로그
            UE_LOG(LogTemp, Warning, TEXT("OnMessageSent delegate bound"));

            UE_LOG(LogTemp, Warning, TEXT("Chat widget created and added to viewport"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create chat widget"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ChatWidgetClass is not set"));
    }
}

void AFeelingBotNPCCharacter::OnMessageReceived(const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("FeelingBotNPCCharacter::OnMessageReceived called with message: %s"), *Message);
    SendMessageToGeminiAPI(Message);
}

void AFeelingBotNPCCharacter::SendMessageToGeminiAPI(const FString& UserMessage)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &AFeelingBotNPCCharacter::OnResponseReceived);
    Request->SetURL(ApiUrl + "?key=" + ApiKey);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");

    FString Prompt = TEXT(
        "You are an AI assistant with emotions. "
        "Analyze the following message and select one of these emotions: happy, sad, angry, fear, neutral. "
        "Please respond in Korean language while maintaining emotional context.\n\n"
        "Response format:\n"
        "Emotion: [selected emotion]\n"
        "Response: [Write an empathetic message in Korean]\n\n"
        "User message: ") + UserMessage;

    // JSON 페이로드 생성
    TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
    TArray<TSharedPtr<FJsonValue>> Contents;

    // 프롬프트를 사용자 메시지로 전송
    TSharedPtr<FJsonObject> UserContent = MakeShareable(new FJsonObject);
    UserContent->SetStringField("role", "user");

    TSharedPtr<FJsonObject> UserParts = MakeShareable(new FJsonObject);
    UserParts->SetStringField("text", Prompt);

    TArray<TSharedPtr<FJsonValue>> UserPartsArray;
    UserPartsArray.Add(MakeShareable(new FJsonValueObject(UserParts)));

    UserContent->SetArrayField("parts", UserPartsArray);
    Contents.Add(MakeShareable(new FJsonValueObject(UserContent)));

    JsonPayload->SetArrayField("contents", Contents);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

    UE_LOG(LogTemp, Warning, TEXT("Request JSON: %s"), *RequestBody);

    Request->SetContentAsString(RequestBody);
    Request->ProcessRequest();
}

void AFeelingBotNPCCharacter::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("Response received. Success: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

    if (bWasSuccessful && Response.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Response content: %s"), *Response->GetContentAsString());

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            TArray<TSharedPtr<FJsonValue>> Candidates = JsonObject->GetArrayField("candidates");
            if (Candidates.Num() > 0)
            {
                TSharedPtr<FJsonObject> FirstCandidate = Candidates[0]->AsObject();
                TSharedPtr<FJsonObject> Content = FirstCandidate->GetObjectField("content");
                TArray<TSharedPtr<FJsonValue>> Parts = Content->GetArrayField("parts");
                if (Parts.Num() > 0)
                {
                    FString AIResponse = Parts[0]->AsObject()->GetStringField("text");
                    UE_LOG(LogTemp, Warning, TEXT("AI Response: %s"), *AIResponse);

                    // 응답에서 감정과 메시지 분리
                    FString Emotion, Message;
                    if (AIResponse.Split(TEXT("\nResponse: "), &Emotion, &Message))
                    {
                        Emotion.ReplaceInline(TEXT("Emotion: "), TEXT(""));
                        UE_LOG(LogTemp, Warning, TEXT("Detected Emotion: %s"), *Emotion);

                        // 감정에 따른 상태 설정
                        if (Emotion.Contains(TEXT("happy")))
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Setting emotion to Happy"));
                            SetEmotionState(EEmotionType::Happy);
                        }
                        else if (Emotion.Contains(TEXT("sad")))
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Setting emotion to Sad"));
                            SetEmotionState(EEmotionType::Sad);
                        }
                        else if (Emotion.Contains(TEXT("angry")))
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Setting emotion to Angry"));
                            SetEmotionState(EEmotionType::Angry);
                        }
                        else if (Emotion.Contains(TEXT("fear")))
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Setting emotion to Fear"));
                            SetEmotionState(EEmotionType::Fear);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Setting emotion to Neutral"));
                            SetEmotionState(EEmotionType::Neutral);
                        }

                        // 채팅 위젯에 메시지 표시
                        if (ChatWidget)
                        {
                            ChatWidget->AddMessage("AI", Message);
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("Failed to parse emotion and message from response"));
                        if (ChatWidget)
                        {
                            ChatWidget->AddMessage("AI", AIResponse);
                        }
                    }
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to receive response from Gemini API"));
        if (ChatWidget)
        {
            ChatWidget->AddMessage("AI", "Sorry, I couldn't process your request.");
        }
    }
}

void AFeelingBotNPCCharacter::SetEmotionState(EEmotionType NewEmotion)
{
    if (!AnimInstance)
    {
        AnimInstance = CharacterMesh->GetAnimInstance();
        UE_LOG(LogTemp, Warning, TEXT("Creating new AnimInstance"));
    }
    if (AnimInstance)
    {
        UAnimMontage* MontageToPlay = nullptr;
        // 감정에 따른 몽타주 선택
        switch (NewEmotion)
        {
        case EEmotionType::Happy:
            MontageToPlay = HappyMontage;
            UE_LOG(LogTemp, Warning, TEXT("Selected Happy Montage: %s, Address: %p"),
                MontageToPlay ? TEXT("Valid") : TEXT("Invalid"), MontageToPlay);
            break;
        case EEmotionType::Sad:
            MontageToPlay = SadMontage;
            UE_LOG(LogTemp, Warning, TEXT("Selected Sad Montage: %s, Address: %p"),
                MontageToPlay ? TEXT("Valid") : TEXT("Invalid"), MontageToPlay);
            break;
        case EEmotionType::Angry:
            MontageToPlay = AngryMontage;
            UE_LOG(LogTemp, Warning, TEXT("Selected Angry Montage: %s, Address: %p"),
                MontageToPlay ? TEXT("Valid") : TEXT("Invalid"), MontageToPlay);
            break;
        case EEmotionType::Fear:
            MontageToPlay = FearMontage;
            UE_LOG(LogTemp, Warning, TEXT("Selected Fear Montage: %s, Address: %p"),
                MontageToPlay ? TEXT("Valid") : TEXT("Invalid"), MontageToPlay);
            break;
        case EEmotionType::Neutral:
            MontageToPlay = NeutralMontage;
            UE_LOG(LogTemp, Warning, TEXT("Selected Neutral Montage: %s, Address: %p"),
                MontageToPlay ? TEXT("Valid") : TEXT("Invalid"), MontageToPlay);
            break;
        }

        if (MontageToPlay)
        {
            // 현재 재생 중인 몽타주 확인
            UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
            if (CurrentMontage)
            {
                UE_LOG(LogTemp, Warning, TEXT("Currently playing montage: %s"), *CurrentMontage->GetName());
                // 필요한 경우 현재 재생 중인 몽타주 중지
                AnimInstance->Montage_Stop(0.25f, CurrentMontage);
            }
            // 몽타주 재생
            float Duration = AnimInstance->Montage_Play(MontageToPlay, 1.0f);
            UE_LOG(LogTemp, Warning, TEXT("Attempting to play montage: %s, Duration: %f"),
                *MontageToPlay->GetName(), Duration);
            if (Duration > 0.0f)
            {
                FOnMontageEnded EndDelegate;
                EndDelegate.BindUObject(this, &AFeelingBotNPCCharacter::OnMontageEnded);
                AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to play montage, Duration is 0"));
            }
        }
    }
}

void AFeelingBotNPCCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Warning, TEXT("Montage Ended, returning to Idle"));
    // 여기서 필요한 경우 Idle 상태로 돌아가는 로직 추가
    SetEmotionState(EEmotionType::Idle);
}
