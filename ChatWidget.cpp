#include "ChatWidget.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UChatWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Button_SendMessage)
    {
        Button_SendMessage->OnClicked.AddDynamic(this, &UChatWidget::OnSendButtonClicked);
    }
}

void UChatWidget::OnSendButtonClicked()
{
    if (EditableTextBox_MessageInput)
    {
        FString Message = EditableTextBox_MessageInput->GetText().ToString();
        if (!Message.IsEmpty())
        {
            // 델리게이트 바인딩 체크 로그 추가
            if (OnMessageSent.IsBound())
            {
                UE_LOG(LogTemp, Warning, TEXT("OnMessageSent is bound, broadcasting: %s"), *Message);
                OnMessageSent.Broadcast(Message);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("OnMessageSent is not bound for message: %s"), *Message);
            }

            AddMessage("User", Message);
            EditableTextBox_MessageInput->SetText(FText::GetEmpty());
        }
    }
}

void UChatWidget::AddMessage(const FString& Sender, const FString& Message)
{
    if (ScrollBox_ChatMessages && MessageWidgetClass)
    {
        UUserWidget* MessageWidget = CreateWidget<UUserWidget>(this, MessageWidgetClass);
        if (MessageWidget)
        {
            UTextBlock* SenderTextBlock = Cast<UTextBlock>(MessageWidget->GetWidgetFromName(TEXT("TextBlock_Sender")));
            UTextBlock* MessageTextBlock = Cast<UTextBlock>(MessageWidget->GetWidgetFromName(TEXT("TextBlock_Message")));
            if (SenderTextBlock)
            {
                SenderTextBlock->SetText(FText::FromString(Sender + ":"));
            }
            if (MessageTextBlock)
            {
                MessageTextBlock->SetText(FText::FromString(Message));
            }
            ScrollBox_ChatMessages->AddChild(MessageWidget);
            ScrollBox_ChatMessages->ScrollToEnd();
        }
    }
}
