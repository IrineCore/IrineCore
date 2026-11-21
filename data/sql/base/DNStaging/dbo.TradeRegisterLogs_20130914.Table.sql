USE [dnstaging]
GO
/****** Object:  Table [dbo].[TradeRegisterLogs_20130914]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TradeRegisterLogs_20130914](
	[RegisterDate] [datetime2](0) NOT NULL,
	[TradeID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[SealCount] [tinyint] NULL,
	[ItemDurability] [smallint] NULL,
	[ItemCount] [smallint] NOT NULL,
	[Price] [int] NOT NULL,
	[WorldID] [int] NULL,
	[PayMethodCode] [tinyint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_TradeRegisterLogs_20130914_RegisterDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_TradeRegisterLogs_20130914_RegisterDate] ON [dbo].[TradeRegisterLogs_20130914]
(
	[RegisterDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_TradeRegisterLogs_20130914_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_TradeRegisterLogs_20130914_CharacterID] ON [dbo].[TradeRegisterLogs_20130914]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_TradeRegisterLogs_20130914_TradeID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_TradeRegisterLogs_20130914_TradeID] ON [dbo].[TradeRegisterLogs_20130914]
(
	[TradeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[TradeRegisterLogs_20130914]  WITH CHECK ADD  CONSTRAINT [CK_TradeRegisterLogs_20130914_RegisterDate] CHECK  (([RegisterDate]>='2013-09-14' AND [RegisterDate]<'2013-09-15'))
GO
ALTER TABLE [dbo].[TradeRegisterLogs_20130914] CHECK CONSTRAINT [CK_TradeRegisterLogs_20130914_RegisterDate]
GO
