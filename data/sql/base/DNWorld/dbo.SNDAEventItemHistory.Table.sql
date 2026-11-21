USE [dnworld]
GO
/****** Object:  Table [dbo].[SNDAEventItemHistory]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SNDAEventItemHistory](
	[SenderCharacterID] [bigint] NOT NULL,
	[ReceiveCharacterID] [bigint] NOT NULL,
	[ReceiveFlag] [bit] NOT NULL,
	[SendDate] [datetime] NOT NULL,
	[ReceiveDate] [datetime] NULL,
	[MailID] [bigint] NULL,
 CONSTRAINT [PK_SNDAEventItemHistory] PRIMARY KEY CLUSTERED 
(
	[SenderCharacterID] ASC,
	[ReceiveCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[SNDAEventItemHistory] ADD  CONSTRAINT [DF_SNDAEventItemHistory_ReceiveFlag]  DEFAULT ((0)) FOR [ReceiveFlag]
GO
ALTER TABLE [dbo].[SNDAEventItemHistory] ADD  CONSTRAINT [DF_SNDAEventItmeHistory_SendDate]  DEFAULT (getdate()) FOR [SendDate]
GO
