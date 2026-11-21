USE [dnstaging]
GO
/****** Object:  Table [dbo].[TradeSuccessLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TradeSuccessLogs](
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
