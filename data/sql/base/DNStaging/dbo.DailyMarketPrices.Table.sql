USE [dnstaging]
GO
/****** Object:  Table [dbo].[DailyMarketPrices]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DailyMarketPrices](
	[MarketPriceID] [int] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[ItemOption] [tinyint] NOT NULL,
	[TradeDate] [date] NOT NULL,
	[PayMethodCode] [tinyint] NOT NULL,
	[TotalPrice] [bigint] NOT NULL,
	[TradeCount] [int] NOT NULL,
	[MinPrice] [int] NOT NULL,
	[MaxPrice] [int] NOT NULL,
	[RegisterCount] [int] NOT NULL,
	[WorldID] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_DailyMarketPrices_TradeDate_ItemID]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_DailyMarketPrices_TradeDate_ItemID] ON [dbo].[DailyMarketPrices]
(
	[TradeDate] ASC,
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [PK_DailyMarketPrices]    Script Date: 11/14/2026 9:01:05 AM ******/
ALTER TABLE [dbo].[DailyMarketPrices] ADD  CONSTRAINT [PK_DailyMarketPrices] PRIMARY KEY NONCLUSTERED 
(
	[MarketPriceID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
