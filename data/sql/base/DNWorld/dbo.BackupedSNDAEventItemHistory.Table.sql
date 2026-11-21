USE [dnworld]
GO
/****** Object:  Table [dbo].[BackupedSNDAEventItemHistory]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BackupedSNDAEventItemHistory](
	[SeasonID] [int] NOT NULL,
	[SenderCharacterID] [bigint] NOT NULL,
	[ReceiveCharacterID] [bigint] NOT NULL,
	[ReceiveFlag] [bit] NOT NULL,
	[SendDate] [datetime] NOT NULL,
	[ReceiveDate] [datetime] NULL,
	[MailID] [bigint] NULL,
 CONSTRAINT [PK_BackupedSNDAEventItemHistory] PRIMARY KEY CLUSTERED 
(
	[SeasonID] ASC,
	[SenderCharacterID] ASC,
	[ReceiveCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[BackupedSNDAEventItemHistory] ADD  CONSTRAINT [DF_BackupedSNDAEventItemHistory_ReceiveFlag]  DEFAULT ((0)) FOR [ReceiveFlag]
GO
ALTER TABLE [dbo].[BackupedSNDAEventItemHistory] ADD  CONSTRAINT [DF_BackupedSNDAEventItemHistory_SendDate]  DEFAULT (getdate()) FOR [SendDate]
GO
