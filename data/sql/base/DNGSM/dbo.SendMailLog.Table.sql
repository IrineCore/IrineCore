USE [dngsm]
GO
/****** Object:  Table [dbo].[SendMailLog]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SendMailLog](
	[AuditLogID] [int] NOT NULL,
	[WorldID] [int] NOT NULL,
	[SenderName] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[ReceiverCode] [tinyint] NOT NULL,
	[ReceiverCharacterName] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[ReceiverCharacterID] [bigint] NULL,
	[ReceiverCharacterLevelMin] [tinyint] NULL,
	[ReceiverCharacterLevelMax] [tinyint] NULL,
	[ReceiverCharacterJobCode] [tinyint] NULL,
	[Subject] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[Content] [nvarchar](300) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[AttachCoin] [bigint] NOT NULL,
	[SendDate] [datetime] NOT NULL,
 CONSTRAINT [PK_SendMailLog] PRIMARY KEY CLUSTERED 
(
	[AuditLogID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[SendMailLog]  WITH CHECK ADD  CONSTRAINT [FK_AuditLogs_SendMailLog_AuditLogID] FOREIGN KEY([AuditLogID])
REFERENCES [dbo].[AuditLogs] ([AuditLogID])
GO
ALTER TABLE [dbo].[SendMailLog] CHECK CONSTRAINT [FK_AuditLogs_SendMailLog_AuditLogID]
GO
