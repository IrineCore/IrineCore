USE [dnworld]
GO
/****** Object:  Table [dbo].[MailSchedules]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MailSchedules](
	[MailScheduleID] [int] IDENTITY(1,1) NOT NULL,
	[SendDate] [smalldatetime] NOT NULL,
	[SenderName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[Subject] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[Content] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[AttachCoin] [bigint] NOT NULL,
	[MailScheduleExpireDate] [smalldatetime] NULL,
	[MailTypeCode] [tinyint] NOT NULL,
	[NewFlag] [bit] NULL,
	[DeleteFlag] [bit] NOT NULL,
	[MailLifespan] [int] NULL,
	[MailTabCode] [tinyint] NULL,
 CONSTRAINT [PK_MailSchedules] PRIMARY KEY CLUSTERED 
(
	[MailScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NNF_MailSchedules_SendDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNF_MailSchedules_SendDate] ON [dbo].[MailSchedules]
(
	[SendDate] ASC
)
WHERE ([MailTypeCode]=(0) AND [DeleteFlag]=(0))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[MailSchedules] ADD  CONSTRAINT [DF_MailSchedules_DeleteFlag]  DEFAULT ((0)) FOR [DeleteFlag]
GO
