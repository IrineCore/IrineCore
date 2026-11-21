USE [dnworld]
GO
/****** Object:  Table [dbo].[Mails]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Mails](
	[MailID] [bigint] IDENTITY(1,1) NOT NULL,
	[SenderCharacterID] [bigint] NULL,
	[ReceiverCharacterID] [bigint] NOT NULL,
	[MailTypeCode] [tinyint] NOT NULL,
	[DeliveryTypeCode] [tinyint] NOT NULL,
	[Subject] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[Content] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[SendDate] [datetime2](0) NOT NULL,
	[ReadFlag] [bit] NOT NULL,
	[DeleteFlag] [bit] NOT NULL,
	[AttachCoin] [bigint] NOT NULL,
	[AttachCoinReceiveFlag] [bit] NOT NULL,
	[SenderName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
	[SystemMailFKey] [bigint] NULL,
	[DeleteDate] [datetime2](0) NULL,
	[NewFlag] [bit] NULL,
	[MailExpireDate] [datetime2](0) NULL,
	[MailTabCode] [tinyint] NULL,
 CONSTRAINT [PK_Mails] PRIMARY KEY CLUSTERED 
(
	[MailID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_Mails_MailTabCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Mails_MailTabCode] ON [dbo].[Mails]
(
	[MailTabCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_Mails_SendDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Mails_SendDate] ON [dbo].[Mails]
(
	[SendDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNF_Mails_AttachCoin]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNF_Mails_AttachCoin] ON [dbo].[Mails]
(
	[AttachCoin] ASC
)
WHERE ([DeleteFlag]=(0) AND [AttachCoinReceiveFlag]=(0) AND [AttachCoin]>(0))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Mails_ReceiverCharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_Mails_ReceiverCharacterID] ON [dbo].[Mails]
(
	[ReceiverCharacterID] ASC
)
INCLUDE([DeleteFlag],[SendDate],[ReadFlag]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Mails_SenderCharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_Mails_SenderCharacterID] ON [dbo].[Mails]
(
	[SenderCharacterID] ASC
)
INCLUDE([SendDate],[DeliveryTypeCode]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
