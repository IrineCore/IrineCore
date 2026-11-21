USE [dnworld]
GO
/****** Object:  Table [dbo].[MarketPrices]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MarketPrices](
	[MarketPriceID] [int] IDENTITY(1,1) NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[ItemOption] [tinyint] NOT NULL,
	[TradeDate] [date] NOT NULL,
	[PayMethodCode] [tinyint] NOT NULL,
	[TotalPrice] [bigint] NOT NULL,
	[TradeCount] [int] NOT NULL,
	[MinPrice] [int] NOT NULL,
	[MaxPrice] [int] NOT NULL,
	[RepresentativeItemSerial] [bigint] NULL,
	[RegisterCount] [int] NOT NULL,
	[RepresentativeItemPotential] [tinyint] NULL,
	[RepresentativeItemDurability] [int] NULL,
	[RepresentativeItemSoulBoundFlag] [bit] NULL,
	[RepresentativeItemSealCount] [tinyint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_MarketPrices_ItemID_TradeDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_MarketPrices_ItemID_TradeDate] ON [dbo].[MarketPrices]
(
	[ItemID] ASC,
	[TradeDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_MarketPrices]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[MarketPrices] ADD  CONSTRAINT [PK_MarketPrices] PRIMARY KEY NONCLUSTERED 
(
	[MarketPriceID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_MarketPrices_RepresentativeItemSerial]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_MarketPrices_RepresentativeItemSerial] ON [dbo].[MarketPrices]
(
	[RepresentativeItemSerial] ASC
)
INCLUDE([ItemID],[RepresentativeItemDurability],[ItemLevel],[ItemOption],[RepresentativeItemSealCount],[RepresentativeItemSoulBoundFlag],[RepresentativeItemPotential]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_MarketPrices_TradeDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_MarketPrices_TradeDate] ON [dbo].[MarketPrices]
(
	[TradeDate] ASC
)
INCLUDE([ItemID],[TradeCount],[TotalPrice],[RegisterCount],[MaxPrice],[ItemLevel],[RepresentativeItemSerial]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[MarketPrices] ADD  CONSTRAINT [DF_MarketPrices_RegisterCount]  DEFAULT ((0)) FOR [RegisterCount]
GO
