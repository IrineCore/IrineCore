USE [dnstaging]
GO
/****** Object:  Table [dbo].[TradeSuccessLogs_20130920]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TradeSuccessLogs_20130920](
	[TradeDate] [datetime2](0) NOT NULL,
	[TradeID] [int] NOT NULL,
	[BuyerCharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[SealCount] [tinyint] NULL,
	[ItemDurability] [smallint] NULL,
	[ItemCount] [smallint] NOT NULL,
	[Price] [int] NOT NULL,
	[WorldID] [int] NULL,
	[CharacterID] [bigint] NULL,
	[PayMethodCode] [tinyint] NULL,
	[TradeStatusCode] [tinyint] NULL,
	[RandomSeed] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_TradeSuccessLogs_20130920_TradeDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_TradeSuccessLogs_20130920_TradeDate] ON [dbo].[TradeSuccessLogs_20130920]
(
	[TradeDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_TradeSuccessLogs_20130920_BuyerCharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_TradeSuccessLogs_20130920_BuyerCharacterID] ON [dbo].[TradeSuccessLogs_20130920]
(
	[BuyerCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_TradeSuccessLogs_20130920_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_TradeSuccessLogs_20130920_CharacterID] ON [dbo].[TradeSuccessLogs_20130920]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_TradeSuccessLogs_20130920_TradeID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_TradeSuccessLogs_20130920_TradeID] ON [dbo].[TradeSuccessLogs_20130920]
(
	[TradeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[TradeSuccessLogs_20130920]  WITH CHECK ADD  CONSTRAINT [CK_TradeSuccessLogs_20130920_TradeDate] CHECK  (([TradeDate]>='2013-09-20' AND [TradeDate]<'2013-09-21'))
GO
ALTER TABLE [dbo].[TradeSuccessLogs_20130920] CHECK CONSTRAINT [CK_TradeSuccessLogs_20130920_TradeDate]
GO
