USE [dnstaging]
GO
/****** Object:  Table [dbo].[MailLogs_20130921]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MailLogs_20130921](
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
/****** Object:  Index [IX_NC_MailLogs_20130921_SendDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_MailLogs_20130921_SendDate] ON [dbo].[MailLogs_20130921]
(
	[SendDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_MailLogs_20130921_MailID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_MailLogs_20130921_MailID] ON [dbo].[MailLogs_20130921]
(
	[MailID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_MailLogs_20130921_ReceiverCharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_MailLogs_20130921_ReceiverCharacterID] ON [dbo].[MailLogs_20130921]
(
	[ReceiverCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_MailLogs_20130921_SenderCharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_MailLogs_20130921_SenderCharacterID] ON [dbo].[MailLogs_20130921]
(
	[SenderCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[MailLogs_20130921]  WITH CHECK ADD  CONSTRAINT [CK_MailLogs_20130921_SendDate] CHECK  (([SendDate]>='2013-09-21' AND [SendDate]<'2013-09-22'))
GO
ALTER TABLE [dbo].[MailLogs_20130921] CHECK CONSTRAINT [CK_MailLogs_20130921_SendDate]
GO
