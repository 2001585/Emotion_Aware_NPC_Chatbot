#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "ChatWidget.generated.h"

UCLASS()
class FEELINGBOT_API UChatWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Chat")
    class UScrollBox* ScrollBox_ChatMessages;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Chat")
    class UEditableTextBox* EditableTextBox_MessageInput;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Chat")
    class UButton* Button_SendMessage;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void OnSendButtonClicked();

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void AddMessage(const FString& Sender, const FString& Message);

    // 새로 추가하는 함수
    UFUNCTION(BlueprintCallable, Category = "Chat")
    void BroadcastMessage(const FString& Message)
    {
        if (OnMessageSent.IsBound())
        {
            UE_LOG(LogTemp, Warning, TEXT("Broadcasting message: %s"), *Message);
            OnMessageSent.Broadcast(Message);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("OnMessageSent is not bound!"));
        }
    }

    // 메시지 전송을 위한 델리게이트 선언
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageSent, const FString&, Message);

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Chat")
    FOnMessageSent OnMessageSent;

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> MessageWidgetClass;
};
