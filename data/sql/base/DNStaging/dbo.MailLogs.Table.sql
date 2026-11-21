USE [dnstaging]
GO
/****** Object:  Table [dbo].[MailLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MailLogs](
	[SendDate] [datetime2](0) NOT NULL,
	[MailID] [bigint] NOT NULL,
	[MailTypeCode] [tinyint] NOT NULL,
	[DeliveryTypeCode] [tinyint] NOT NULL,
	[SenderCharacterID] [bigint] NULL,
	[SenderName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NULL,
	[ReceiverCharacterID] [bigint] NOT NULL,
	[Subject] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[AttachCoin] [bigint] NOT NULL,
	[AttachItemFlag] [bit] NOT NULL,
	[WorldID] [int] NULL
) ON [PRIMARY]
GO
